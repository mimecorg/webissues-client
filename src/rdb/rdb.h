/****************************************************************************
* Simple template-based relational database
* Copyright (C) 2006-2011 Michał Męciński
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*   1. Redistributions of source code must retain the above copyright notice,
*      this list of conditions and the following disclaimer.
*   2. Redistributions in binary form must reproduce the above copyright
*      notice, this list of conditions and the following disclaimer in the
*      documentation and/or other materials provided with the distribution.
*   3. Neither the name of the copyright holder nor the names of the
*      contributors may be used to endorse or promote products derived from
*      this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
****************************************************************************/

#ifndef RDB_RDB_H
#define RDB_RDB_H

/**
* Simple template-based relational database.
*/
namespace RDB
{

struct Bucket;

struct Link
{
    int m_key;
    Bucket* m_prev;
    Bucket* m_next;
};

struct Bucket
{
    void* m_row;
    Link m_links[ 1 ];
};

class IndexData
{
public:
    IndexData() : m_dim( 0 ), m_size( 0 ), m_buckets( 0 ) { }

    ~IndexData() { delete[] m_buckets; }

public:
    void clear()
    {
        for ( int i = 0; i < m_size; i++ )
            m_buckets[ i ] = 0;
    }

    void insert( Bucket* bucket, int key )
    {
        unsigned int hash = key % m_size;
        bucket->m_links[ m_dim ].m_key = key;
        bucket->m_links[ m_dim ].m_next = m_buckets[ hash ];
        if ( bucket->m_links[ m_dim ].m_next )
            bucket->m_links[ m_dim ].m_next->m_links[ m_dim ].m_prev = bucket;
        bucket->m_links[ m_dim ].m_prev = 0;
        m_buckets[ hash ] = bucket;
    }

    void remove( Bucket* bucket )
    {
        if ( bucket->m_links[ m_dim ].m_prev ) {
            bucket->m_links[ m_dim ].m_prev->m_links[ m_dim ].m_next = bucket->m_links[ m_dim ].m_next;
        } else {
            unsigned int hash = bucket->m_links[ m_dim ].m_key % m_size;
            m_buckets[ hash ] = bucket->m_links[ m_dim ].m_next;
        }
        if ( bucket->m_links[ m_dim ].m_next )
            bucket->m_links[ m_dim ].m_next->m_links[ m_dim ].m_prev = bucket->m_links[ m_dim ].m_prev;
    }

    Bucket* find( int key ) const
    {
        unsigned int hash = key % m_size;
        Bucket* bucket = m_buckets[ hash ];
        while ( bucket ) {
            if ( bucket->m_links[ m_dim ].m_key == key )
                return bucket;
            bucket = bucket->m_links[ m_dim ].m_next;
        }
        return 0; 
    }

    Bucket* find( int key, const IndexData* data2, int key2 ) const
    {
        unsigned int hash = key % m_size;
        Bucket* bucket = m_buckets[ hash ];
        while ( bucket ) {
            if ( bucket->m_links[ m_dim ].m_key == key && bucket->m_links[ data2->m_dim ].m_key == key2 )
                return bucket;
            bucket = bucket->m_links[ m_dim ].m_next;
        }
        return 0; 
    }

public:
    int m_dim;
    int m_size;
    Bucket** m_buckets;

private:
   IndexData( const IndexData& );
   IndexData& operator =( const IndexData& );
};

/**
* Base class for indexes and iterators.
*/
class IndexWrapper
{
protected:
    IndexWrapper() : m_data( 0 ) { }

    explicit IndexWrapper( const IndexData* data ) : m_data( data ) { }

public:
    /**
    * Return @c true if the value was initialized.
    */
    bool isValid() const { return m_data; }

    /**
    * Return the dimension used by the index.
    */
    int dim() const { return m_data ? m_data->m_dim : -1; }

protected:
    const IndexData* m_data;
};

/**
* Base class for pair indexes.
*/
class IndexPairWrapper
{
protected:
    IndexPairWrapper() : m_first( 0 ), m_second( 0 ) { }

    IndexPairWrapper( const IndexData* first, const IndexData* second ) :
        m_first( first ), m_second( second ) { }

public:
    /**
    * Return @c true if the value was initialized.
    */
    bool isValid() const { return m_first && m_second; }

    /**
    * Return the dimension used by the fist index.
    */
    int firstDim() const { return m_first ? m_first->m_dim : -1; }
    /**
    * Return the dimension used by the second index.
    */
    int secondDim() const { return m_second ? m_second->m_dim : -1; }

protected:
    const IndexData* m_first;
    const IndexData* m_second;
};

/**
* Unique key index for non-const rows.
*/
template<class ROW>
class UniqueIndex : public IndexWrapper
{
public:
    /**
    * Default constructor.
    */
    UniqueIndex() { }

    explicit UniqueIndex( const IndexData* data ) : IndexWrapper( data ) { }

public:
    /**
    * Find the row with the given key.
    */
    ROW* find( int key ) const
    {
        if ( m_data ) {
            Bucket* bucket = m_data->find( key );
            if ( bucket )
                return static_cast<ROW*>( bucket->m_row );
        }
        return 0;
    }
};

/**
* Typeless specialization of the unique key index.
*/
template<>
class UniqueIndex<void> : public IndexWrapper
{
public:
    /**
    * Default constructor.
    */
    UniqueIndex() { }

    /**
    * Conversion from a typed index.
    */
    template<class ROW>
    UniqueIndex( const UniqueIndex<ROW>& other ) : IndexWrapper( other ) { }
};

/**
* Unique key index for constant rows.
*/
template<class ROW>
class UniqueConstIndex : public IndexWrapper
{
public:
    /**
    * Default constructor.
    */
    UniqueConstIndex() { }

    /**
    * Conversion from a non-const index.
    */
    UniqueConstIndex( const UniqueIndex<ROW>& other ) : IndexWrapper( other ) { }

    explicit UniqueConstIndex( const IndexData* data ) : IndexWrapper( data ) { }

public:
    /**
    * Find the row with the given key.
    */
    const ROW* find( int key ) const
    {
        if ( m_data ) {
            Bucket* bucket = m_data->find( key );
            if ( bucket )
                return static_cast<const ROW*>( bucket->m_row );
        }
        return 0;
    }
};

/**
* Typeless specialization of the unique key constant index.
*/
template<>
class UniqueConstIndex<void> : public IndexWrapper
{
public:
    /**
    * Default constructor.
    */
    UniqueConstIndex() { }

    /**
    * Conversion from a typed index.
    */
    template<class ROW>
    UniqueConstIndex( const UniqueConstIndex<ROW>& other ) : IndexWrapper( other ) { }

    /**
    * Conversion from a typed index.
    */
    template<class ROW>
    UniqueConstIndex( const UniqueIndex<ROW>& other ) : IndexWrapper( other ) { }
};

/**
* Foreign key index for non-const rows.
*/
template<class ROW>
class ForeignIndex : public IndexWrapper
{
public:
    /**
    * Default constructor.
    */
    ForeignIndex() { }

    explicit ForeignIndex( const IndexData* data ) : IndexWrapper( data ) { }
};

/**
* Typeless specialization of the foreign key index.
*/
template<>
class ForeignIndex<void> : public IndexWrapper
{
public:
    /**
    * Default constructor.
    */
    ForeignIndex() { }

    /**
    * Conversion from a typed index.
    */
    template<class ROW>
    ForeignIndex( const ForeignIndex<ROW>& other ) : IndexWrapper( other ) { }
};

/**
* Foreign key index for constant rows.
*/
template<class ROW>
class ForeignConstIndex : public IndexWrapper
{
public:
    /**
    * Default constructor.
    */
    ForeignConstIndex() { }

    /**
    * Conversion from a non-const index.
    */
    ForeignConstIndex( const ForeignIndex<ROW>& other ) : IndexWrapper( other ) { }

    explicit ForeignConstIndex( const IndexData* data ) : IndexWrapper( data ) { }
};

/**
* Typeless specialization of the foreign key constant index.
*/
template<>
class ForeignConstIndex<void> : public IndexWrapper
{
public:
    /**
    * Default constructor.
    */
    ForeignConstIndex() { }

    /**
    * Conversion from a typed index.
    */
    template<class ROW>
    ForeignConstIndex( const ForeignConstIndex<ROW>& other ) : IndexWrapper( other ) { }

    /**
    * Conversion from a typed index.
    */
    template<class ROW>
    ForeignConstIndex( const ForeignIndex<ROW>& other ) : IndexWrapper( other ) { }
};

/**
* Unique pair of foreign key indexes for non-const rows.
*/
template<class ROW>
class UniquePairIndex : public IndexPairWrapper
{
public:
    /**
    * Default constructor.
    */
    UniquePairIndex() { }

    UniquePairIndex( const IndexData* first, const IndexData* second ) :
        IndexPairWrapper( first, second ) { }

public:
    /**
    * Return the first foreign index from the pair.
    */
    ForeignIndex<ROW> first() const { return ForeignIndex<ROW>( m_first ); }
    /**
    * Return the second foreign index from the pair.
    */
    ForeignIndex<ROW> second() const { return ForeignIndex<ROW>( m_second ); }

    /**
    * Find the row with the given pair of keys.
    */
    ROW* find( int firstKey, int secondKey ) const
    {
        if ( m_first ) {
            Bucket* bucket = m_first->find( firstKey, m_second, secondKey );
            if ( bucket )
                return static_cast<ROW*>( bucket->m_row );
        }
        return 0;
    }
};

/**
* Unique pair of foreign key indexes for constant rows.
*/
template<class ROW>
class UniquePairConstIndex : public IndexPairWrapper
{
public:
    /**
    * Default constructor.
    */
    UniquePairConstIndex() { }

    /**
    * Conversion from a typed index.
    */
    UniquePairConstIndex( const UniquePairIndex<ROW>& other ) :
        IndexPairWrapper( other ) { }

    UniquePairConstIndex( const IndexData* first, const IndexData* second ) :
        IndexPairWrapper( first, second ) { }

public:
    /**
    * Return the first foreign index from the pair.
    */
    ForeignConstIndex<ROW> first() const { return ForeignConstIndex<ROW>( m_first ); }
    /**
    * Return the second foreign index from the pair.
    */
    ForeignConstIndex<ROW> second() const { return ForeignIndex<ROW>( m_second ); }

    /**
    * Find the row with the given pair of keys.
    */
    const ROW* find( int firstKey, int secondKey ) const
    {
        if ( m_first ) {
            Bucket* bucket = m_first->find( firstKey, m_second, secondKey );
            if ( bucket )
                return static_cast<const ROW*>( bucket->m_row );
        }
        return 0;
    }
};

/**
* Base class for index iterators.
*/
class IndexIteratorBase : public IndexWrapper
{
protected:
    IndexIteratorBase() :
        m_hash( 0 ), m_bucket( 0 ) { }

    IndexIteratorBase( const IndexWrapper& index ) :
        IndexWrapper( index ),
        m_hash( 0 ), m_bucket( 0 ) { }

public:
    /**
    * Go to the next row.
    * @return @c true if there is next row.
    */
    bool next()
    {
        if ( m_data ) {
            while ( m_hash < m_data->m_size ) {
                if ( !m_bucket )
                    m_bucket = m_data->m_buckets[ m_hash ];
                else
                    m_bucket = m_bucket->m_links[ m_data->m_dim ].m_next;
                if ( m_bucket )
                    return true;
                m_hash++;
            }
        }
        return false;
    }

    /**
    * Return the key of the current row associated with the iterator's index.
    */
    int key() const
    {
        return m_bucket ? m_bucket->m_links[ m_data->m_dim ].m_key : 0;
    }

    /**
    * Return the given key of the current row.
    */
    int key( int dim ) const
    {
        return m_bucket ? m_bucket->m_links[ dim ].m_key : 0;
    }

protected:
    int m_hash;
    Bucket* m_bucket;
};

/**
* Non-const iterator for all rows in an index.
*/
template<class ROW>
class IndexIterator : public IndexIteratorBase
{
public:
    /**
    * Default constructor.
    */
    IndexIterator() { }

    /**
    * Create iterator for a unique index.
    */
    explicit IndexIterator( const UniqueIndex<ROW>& index ) :
        IndexIteratorBase( index ) { }

    /**
    * Create iterator for a foreign index.
    */
    explicit IndexIterator( const ForeignIndex<ROW>& index ) :
        IndexIteratorBase( index ) { }

public:
    /**
    * Return the current row.
    */
    ROW* get() const
    {
        return static_cast<ROW*>( m_bucket ? m_bucket->m_row : 0 );
    }
};

/**
* Typeless specialization of the non-const index iterator.
*/
template<>
class IndexIterator<void> : public IndexIteratorBase
{
public:
    /**
    * Default constructor.
    */
    IndexIterator() { }

    /**
    * Create iterator for a unique index.
    */
    explicit IndexIterator( const UniqueIndex<void>& index ) :
        IndexIteratorBase( index ) { }

    /**
    * Create iterator for a foreign index.
    */
    explicit IndexIterator( const ForeignIndex<void>& index ) :
        IndexIteratorBase( index ) { }
};

/**
* Constant iterator for all rows in an index.
*/
template<class ROW>
class IndexConstIterator : public IndexIteratorBase
{
public:
    /**
    * Default constructor.
    */
    IndexConstIterator() { }

    /**
    * Create iterator for a unique index.
    */
    explicit IndexConstIterator( const UniqueConstIndex<ROW>& index ) :
        IndexIteratorBase( index ) { }

    /**
    * Create iterator for a foreign index.
    */
    explicit IndexConstIterator( const ForeignConstIndex<ROW>& index ) :
        IndexIteratorBase( index ) { }

public:
    /**
    * Return the current row.
    */
    const ROW* get() const
    {
        return static_cast<ROW*>( m_bucket ? m_bucket->m_row : 0 );
    }
};

/**
* Typeless specialization of the constant index iterator.
*/
template<>
class IndexConstIterator<void> : public IndexIteratorBase
{
public:
    /**
    * Default constructor.
    */
    IndexConstIterator() { }

    /**
    * Create iterator for a unique index.
    */
    explicit IndexConstIterator( const UniqueConstIndex<void>& index ) :
        IndexIteratorBase( index ) { }

    /**
    * Create iterator for a foreign index.
    */
    explicit IndexConstIterator( const ForeignConstIndex<void>& index ) :
        IndexIteratorBase( index ) { }
};

/**
* Base class for foreign iterators.
*/
class ForeignIteratorBase : public IndexWrapper
{
protected:
    ForeignIteratorBase() :
        m_key( 0 ), m_end( false ), m_bucket( 0 ) { }

    ForeignIteratorBase( const IndexWrapper& index, int key ) :
        IndexWrapper( index ),
        m_key( key ), m_end( false ), m_bucket( 0 ) { }

public:
    /**
    * Go to the next row.
    * @return @c true if there is next row.
    */
    bool next()
    {
        if ( m_data ) {
            while ( !m_end ) {
                if ( !m_bucket ) {
                    unsigned int hash = m_key % m_data->m_size;
                    m_bucket = m_data->m_buckets[ hash ];
                } else {
                    m_bucket = m_bucket->m_links[ m_data->m_dim ].m_next;
                }
                if ( m_bucket ) {
                    if ( m_bucket->m_links[ m_data->m_dim ].m_key != m_key )
                        continue;
                    return true;
                }
                m_end = true;
            }
        }
        return false;
    }

    /**
    * Return the given key of the current row.
    */
    int key( int dim ) const
    {
        return m_bucket ? m_bucket->m_links[ dim ].m_key : 0;
    }

protected:
    int m_key;
    bool m_end;
    Bucket* m_bucket;
};

/**
* Non-const iterator for rows with a given foreign key value.
*/
template<class ROW>
class ForeignIterator : public ForeignIteratorBase
{
public:
    /**
    * Default constructor.
    */
    ForeignIterator() { }

    /**
    * Create iterator for a foreign index.
    */
    ForeignIterator( const ForeignIndex<ROW>& index, int key ) :
        ForeignIteratorBase( index, key ) { }

public:
    /**
    * Return the current row.
    */
    ROW* get() const
    {
        return static_cast<ROW*>( m_bucket ? m_bucket->m_row : 0 );
    }
};

/**
* Typeless specialization of the non-const foreign iterator.
*/
template<>
class ForeignIterator<void> : public ForeignIteratorBase
{
public:
    /**
    * Default constructor.
    */
    ForeignIterator() { }

    /**
    * Create iterator for a foreign index.
    */
    ForeignIterator( const ForeignIndex<void>& index, int key ) :
        ForeignIteratorBase( index, key ) { }
};

/**
* Constant iterator for rows with a given foreign key value.
*/
template<class ROW>
class ForeignConstIterator : public ForeignIteratorBase
{
public:
    /**
    * Default constructor.
    */
    ForeignConstIterator() { }

    /**
    * Create iterator for a foreign index.
    */
    ForeignConstIterator( const ForeignConstIndex<ROW>& index, int key ) :
        ForeignIteratorBase( index, key ) { }

public:
    /**
    * Return the current row.
    */
    const ROW* get() const
    {
        return static_cast<const ROW*>( m_bucket ? m_bucket->m_row : 0 );
    }
};

/**
* Typeless specialization of the constant foreign iterator.
*/
template<>
class ForeignConstIterator<void> : public ForeignIteratorBase
{
public:
    /**
    * Default constructor.
    */
    ForeignConstIterator() { }

    /**
    * Create iterator for a foreign index.
    */
    ForeignConstIterator( const ForeignConstIndex<void>& index, int key ) :
        ForeignIteratorBase( index, key ) { }
};

/**
* Base of all table templates.
*/
template<class ROW, int DIMS>
class TableBase
{
protected:
    TableBase()
    {
        for ( int i = 0; i < DIMS; i++ ) {
            m_data[ i ].m_dim = i;
            m_keyMethods[ i ] = 0;
        }
    }

    ~TableBase() { clear(); }

public:
    /**
    * Insert a row into the table.
    */
    void insert( ROW* row )
    {
        Bucket* bucket = reinterpret_cast<Bucket*>( new BucketStorage() );
        bucket->m_row = row;

        for ( int i = 0; i < DIMS; i++ ) {
            KeyMethod keyMethod = m_keyMethods[ i ];
            int key = ( row->*keyMethod )();
            m_data[ i ].insert( bucket, key );
        }
    }

    /**
    * Remove all rows.
    */
    void clear()
    {
        for ( int i = 0; i < m_data[ 0 ].m_size; i++ ) {
            Bucket* bucket = m_data[ 0 ].m_buckets[ i ];
            while ( bucket ) {
                Bucket* next = bucket->m_links[ 0 ].m_next;
                delete static_cast<ROW*>( bucket->m_row );
                delete reinterpret_cast<BucketStorage*>( bucket );
                bucket = next;
            }
        }

        for ( int i = 0; i < DIMS; i++ )
            m_data[ i ].clear();
    }

protected:
    typedef int ( ROW::*KeyMethod )() const;

    IndexData* data( int dim ) { return &m_data[ dim ]; }
    const IndexData* data( int dim ) const { return &m_data[ dim ]; }

    void init( IndexData* data, int size, KeyMethod keyMethod )
    {
        data->m_size = size;
        data->m_buckets = new Bucket*[ size ];
        data->clear();

        m_keyMethods[ data->m_dim ] = keyMethod;
    }

    void removeBucket( Bucket* bucket )
    {
        for ( int i = 0; i < DIMS; i++ )
            m_data[ i ].remove( bucket );

        delete static_cast<ROW*>( bucket->m_row );
        delete reinterpret_cast<BucketStorage*>( bucket );
    }

    void removeBuckets( IndexData* data, int key )
    {
        unsigned int hash = key % data->m_size;
        Bucket* bucket = data->m_buckets[ hash ];
        while ( bucket ) {
            Bucket* next = bucket->m_links[ data->m_dim ].m_next;
            if ( bucket->m_links[ data->m_dim ].m_key == key )
                removeBucket( bucket );
            bucket = next;
        }
    }

private:
    struct BucketStorage
    {
        void* m_row;
        Link m_links[ DIMS ];
    };

    IndexData m_data[ DIMS ];

    KeyMethod m_keyMethods[ DIMS ];

private:
    TableBase( const TableBase& );
    TableBase& operator =( const TableBase& );
};

/**
* Table with one index for rows with a single primary key.
*/
template<class ROW, int DIMS = 1>
class SimpleTable : public TableBase<ROW, DIMS>
{
    typedef TableBase<ROW, DIMS> Base;
public:
    /**
    * Default constructor.
    */
    SimpleTable() { }

public:
    /**
    * Initialize the primary index.
    * @param size Size of the hash table to use.
    * @param keyMethod Pointer to method returning the primary key.
    */
    void initIndex( int size, typename Base::KeyMethod keyMethod )
    {
        init( Base::data( 0 ), size, keyMethod );
    }

    /**
    * Return the primary index.
    */
    UniqueIndex<ROW> index()
    {
        return UniqueIndex<ROW>( Base::data( 0 ) );
    }

    /**
    * @overload
    */
    UniqueConstIndex<ROW> index() const
    {
        return UniqueIndex<ROW>( Base::data( 0 ) );
    }

    /**
    * Find the row with the given primary key.
    */
    ROW* find( int key )
    {
        return index().find( key );
    }

    /**
    * @overload
    */
    const ROW* find( int key ) const
    {
        return index().find( key );
    }

    /**
    * Remove the row with the given primary key.
    */
    void remove( int key )
    {
        Bucket* bucket = Base::data( 0 )->find( key );
        if ( bucket )
            Base::removeBucket( bucket );
    }

private:
    SimpleTable( const SimpleTable& );
    SimpleTable& operator =( const SimpleTable& );
};

/**
* Table with two indexes for rows with a primary key and a foreign key.
*/
template<class ROW, int DIMS = 2>
class ChildTable : public SimpleTable<ROW, DIMS>
{
    typedef SimpleTable<ROW, DIMS> Base;
public:
    /**
    * Default constructor.
    */
    ChildTable() { }

public:
    /**
    * Initialize the parent (foreign) index.
    * @param size Size of the hash table to use.
    * @param keyMethod Pointer to method returning the foreign key.
    */
    void initParentIndex( int size, typename Base::KeyMethod keyMethod )
    {
        init( Base::data( 1 ), size, keyMethod );
    }

    /**
    * Return the foreign index.
    */
    ForeignIndex<ROW> parentIndex()
    {
        return ForeignIndex<ROW>( Base::data( 1 ) );
    }

    /**
    * @overload
    */
    ForeignConstIndex<ROW> parentIndex() const
    {
        return ForeignConstIndex<ROW>( Base::data( 1 ) );
    }

    /**
    * Remove all rows with the given foreign key.
    */
    void removeChildren( int parentKey )
    {
        Base::removeBuckets( Base::data( 1 ), parentKey );
    }

private:
    ChildTable( const ChildTable& );
    ChildTable& operator =( const ChildTable& );
};

/**
* Table for rows with two foreign keys which form a unique pair.
*/
template<class ROW, int DIMS = 2>
class CrossTable : public TableBase<ROW, DIMS>
{
    typedef TableBase<ROW, DIMS> Base;
public:
    /**
    * Default constructor.
    */
    CrossTable() { }

public:
    /**
    * Initialize the first foreign index.
    * @param size Size of the hash table to use.
    * @param keyMethod Pointer to method returning the first key.
    */
    void initFirstIndex( int size, typename Base::KeyMethod keyMethod )
    {
        init( Base::data( 0 ), size, keyMethod );
    }

    /**
    * Initialize the second foreign index.
    * @param size Size of the hash table to use.
    * @param keyMethod Pointer to method returning the second key.
    */
    void initSecondIndex( int size, typename Base::KeyMethod keyMethod )
    {
        init( Base::data( 1 ), size, keyMethod );
    }

    /**
    * Return the unique pair index.
    */
    UniquePairIndex<ROW> index()
    {
        return UniquePairIndex<ROW>( Base::data( 0 ), Base::data( 1 ) );
    }

    /**
    * @overload
    */
    UniquePairConstIndex<ROW> index() const
    {
        return UniquePairConstIndex<ROW>( Base::data( 0 ), Base::data( 1 ) );
    }

    /**
    * Find the row with the given keys.
    */
    ROW* find( int firstKey, int secondKey )
    {
        return index().find( firstKey, secondKey );
    }

    /**
    * @overload
    */
    const ROW* find( int firstKey, int secondKey ) const
    {
        return index().find( firstKey, secondKey );
    }

    /**
    * Remove the row with the given keys.
    */
    void remove( int firstKey, int secondKey )
    {
        Bucket* bucket = Base::data( 0 )->find( firstKey, Base::data( 1 ), secondKey );
        if ( bucket )
            Base::removeBucket( bucket );
    }

    /**
    * Remove the rows with the given value of the first key.
    */
    void removeFirst( int firstKey )
    {
        Base::removeBuckets( Base::data( 0 ), firstKey );
    }

    /**
    * Remove the rows with the given value of the second key.
    */
    void removeSecond( int secondKey )
    {
        Base::removeBuckets( Base::data( 1 ), secondKey );
    }

private:
    CrossTable( const CrossTable& );
    CrossTable& operator =( const CrossTable& );
};

}

#endif
