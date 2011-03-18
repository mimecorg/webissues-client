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

#ifndef RDB_UTILITIES_H
#define RDB_UTILITIES_H

#include <QList>
#include <QString>

namespace RDB
{

template<class T, typename R>
class LessThan
{
public:
    LessThan( R (T::*order)() const ) : m_order( order ) {}

    bool operator ()( const T* row1, const T* row2 )
    {
        return ( row1->*m_order )() < ( row2->*m_order )();
    }

private:
    R (T::*m_order)() const;
};

/**
* Sort rows of a table using given property.
* The values of the property are compared using operator >.
* @param it Row iterator of any kind.
* @param order Getter method of the property to use for sorting.
* @return The list of rows ordered by the property value.
* Example:
* @code
* RDB::IndexConstIterator<MyRow> it( myTable.index() );
* QList<const MyRow*> rows = sortRows( it, &MyRow::size );
* @endcode
*/
template<class T, typename IT, typename R>
inline QList<const T*> sortRows( IT it, R (T::*order)() const )
{
    QList<const T*> list;
    while ( it.next() )
        list.append( it.get() );
    qSort( list.begin(), list.end(), LessThan< T, R >( order ) );
    return list;
}

template<class T, typename R>
class LocaleAwareLessThan
{
public:
    LocaleAwareLessThan( R (T::*order)() const ) : m_order( order ) {}

    bool operator ()( const T* row1, const T* row2 )
    {
        return QString::localeAwareCompare( ( row1->*m_order )(), ( row2->*m_order )() ) < 0;
    }

private:
    R (T::*m_order)() const;
};

/**
* Sort rows of a table using given property.
* The values of the property are compared using QString::localeAwareCompare().
* @param it Row iterator of any kind.
* @param order Getter method of the property to use for sorting.
* @return The list of rows ordered by the property value.
* @see sortRows()
*/
template<class T, typename IT, typename R>
inline QList<const T*> localeAwareSortRows( IT it, R (T::*order)() const )
{
    QList<const T*> list;
    while ( it.next() )
        list.append( it.get() );
    qSort( list.begin(), list.end(), LocaleAwareLessThan< T, R >( order ) );
    return list;
}

/**
* Find a row with given value of a non-indexed property in a table.
* @param it Row iterator of any kind.
* @param method Getter method of the property to check.
* @param value The value to find.
* @return The first row with given value or @c NULL if none was found.
*/
template<class T, typename IT, typename R, typename V>
inline const T* findRow( IT it, R (T::*method)() const, const V& value )
{
    while ( it.next() ) {
        if ( ( it.get()->*method )() == value )
            return it.get();
    }
    return NULL;
}

}

#endif
