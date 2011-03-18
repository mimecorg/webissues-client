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

#include "tableitemmodel.h"
#include "abstracttablemodel.h"
#include "abstractrowfilter.h"

#include <QPixmap>

using namespace RDB;

TableItemModel::TableItemModel( QObject* parent ) : QAbstractItemModel( parent ),
    m_sortColumn( -1 ),
    m_sortOrder( Qt::AscendingOrder ),
    m_filter( NULL ),
    m_totalCount( 0 )
{
}

TableItemModel::~TableItemModel()
{
}

void TableItemModel::setColumns( const QList<int>& columns )
{
    if ( m_columns == columns )
        return;

    m_columns = columns;

    emit reset();
}

void TableItemModel::setRowFilter( AbstractRowFilter* filter )
{
    m_filter = filter;

    connect( filter, SIGNAL( conditionsChanged() ), this, SLOT( updateData() ) );

    if ( m_uniqueIndex.isValid() || m_foreignIndex.isValid() )
        updateData();
}

void TableItemModel::setRootTableModel( AbstractTableModel* model, const UniqueConstIndex<void>& index )
{
    attachTableModel( model, index.dim() );

    m_uniqueIndex = index;
    m_foreignIndex = ForeignConstIndex<void>();

    buildAllIndexes();

    emit reset();
}

void TableItemModel::setRootTableModel( AbstractTableModel* model, const RDB::UniqueConstIndex<void>& index,
        const RDB::ForeignConstIndex<void>& parentIndex, int parentId )
{
    attachTableModel( model, index.dim() );

    m_uniqueIndex = UniqueConstIndex<void>();
    m_foreignIndex = parentIndex;
    m_parentId = parentId;

    buildAllIndexes();

    emit reset();
}

void TableItemModel::setRootTableModel( AbstractTableModel* model, const RDB::ForeignConstIndex<void>& index,
        const RDB::ForeignConstIndex<void>& parentIndex, int parentId )
{
    attachTableModel( model, index.dim() );

    m_uniqueIndex = UniqueConstIndex<void>();
    m_foreignIndex = parentIndex;
    m_parentId = parentId;

    buildAllIndexes();

    emit reset();
}

AbstractTableModel* TableItemModel::rootTableModel() const
{
    return m_models.first();
}

void TableItemModel::addChildTableModel( AbstractTableModel* model, const RDB::UniqueConstIndex<void>& index,
        const RDB::ForeignConstIndex<void>& parentIndex )
{
    attachTableModel( model, index.dim() );

    m_childIndexes.append( parentIndex );

    for ( int i = 0; i < m_itemGroups.count(); i++ ) {
        if ( m_itemGroups.at( i ).m_level == m_childIndexes.count() - 1 )
            buildChildIndexes( i );
    }

    emit reset();
}

void TableItemModel::addChildTableModel( AbstractTableModel* model, const RDB::ForeignConstIndex<void>& index,
        const RDB::ForeignConstIndex<void>& parentIndex )
{
    attachTableModel( model, index.dim() );

    m_childIndexes.append( parentIndex );

    for ( int i = 0; i < m_itemGroups.count(); i++ ) {
        if ( m_itemGroups.at( i ).m_level == m_childIndexes.count() - 1 )
            buildChildIndexes( i );
    }

    emit reset();
}

int TableItemModel::columnCount( const QModelIndex& /*parent*/ /*= QModelIndex()*/ ) const
{
    return m_columns.count();
}

QVariant TableItemModel::headerData( int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/ ) const
{
    if ( orientation == Qt::Horizontal && role == Qt::DisplayRole ) {
        if ( section < 0 || section >= m_columns.count() )
            return QVariant();

        return m_models.first()->columnName( m_columns.at( section ) );
    }

    if ( orientation == Qt::Horizontal && role == ColumnRole ) {
        if ( section < 0 || section >= m_columns.count() )
            return QVariant();

        return (int)m_columns.at( section );
    }

    return QVariant();
}

int TableItemModel::rowCount( const QModelIndex& parent /*= QModelIndex()*/ ) const
{
    if ( !parent.isValid() )
        return m_itemGroups.first().m_items.count();

    int group = parent.internalId();
    if ( group < 0 || group >= m_itemGroups.count() )
        return 0;

    if ( parent.row() < 0 || parent.row() >= m_itemGroups.at( group ).m_items.count() )
        return 0;

    int childGroup = m_itemGroups.at( group ).m_items.at( parent.row() ).m_childGroup;
    if ( childGroup < 0 || childGroup >= m_itemGroups.count() )
        return 0;

    return m_itemGroups.at( childGroup ).m_items.count();
}

QModelIndex TableItemModel::index( int row, int column, const QModelIndex& parent /*= QModelIndex()*/ ) const
{
    if ( !parent.isValid() )
        return createIndex( row, column, 0 );

    int group = parent.internalId();
    if ( group < 0 || group >= m_itemGroups.count() )
        return QModelIndex();

    if ( parent.row() < 0 || parent.row() >= m_itemGroups.at( group ).m_items.count() )
        return QModelIndex();

    int childGroup = m_itemGroups.at( group ).m_items.at( parent.row() ).m_childGroup;
    if ( childGroup < 0 || childGroup >= m_itemGroups.count() )
        return QModelIndex();

    return createIndex( row, column, childGroup );
}

QModelIndex TableItemModel::parent( const QModelIndex& index ) const
{
    if ( !index.isValid() )
        return QModelIndex();

    int group = index.internalId();
    if ( group < 0 || group >= m_itemGroups.count() )
        return QModelIndex();

    int level = m_itemGroups.at( group ).m_level;
    if ( level == 0 )
        return QModelIndex();

    for ( int i = 0; i < m_itemGroups.count(); i++ ) {
        if ( m_itemGroups.at( i ).m_level == level - 1 ) {
            for ( int j = 0; j < m_itemGroups.at( i ).m_items.count(); j++ ) {
                if ( m_itemGroups.at( i ).m_items.at( j ).m_childGroup == group )
                    return createIndex( j, 0, i );
            }
        }
    }

    return QModelIndex();
}

QVariant TableItemModel::data( const QModelIndex& index, int role /*= Qt::DisplayRole*/ ) const
{
    if ( !index.isValid() )
        return QVariant();

    int group = index.internalId();
    if ( group < 0 || group >= m_itemGroups.count() )
        return QVariant();

    if ( index.row() < 0 || index.row() >= m_itemGroups.at( group ).m_items.count() )
        return QVariant();

    int level = m_itemGroups.at( group ).m_level;
    int id = m_itemGroups.at( group ).m_items.at( index.row() ).m_id;

    if ( index.column() < 0 || index.column() >= m_columns.count() )
        return QVariant();

    int column = m_columns.at( index.column() );

    switch ( role ) {
        case Qt::DisplayRole:
            return m_models.at( level )->text( id, column );

        case Qt::DecorationRole:
            return m_models.at( level )->icon( id, column );

        case Qt::ToolTipRole:
            return m_models.at( level )->toolTip( id, column );

        case RowIdRole:
            return id;

        case LevelRole:
            return level;

        default:
            return QVariant();
    }
}

void TableItemModel::sort( int column, Qt::SortOrder order /*= Qt::AscendingOrder*/ )
{
    if ( column < 0 || column >= m_columns.count() )
        return;

    if ( m_sortColumn == m_columns.at( column ) && m_sortOrder == order )
        return;

    m_sortColumn = m_columns.at( column );
    m_sortOrder = order;

    sortAllGroups();
}

void TableItemModel::attachTableModel( AbstractTableModel* model, int dim )
{
    connect( model, SIGNAL( dataChanged() ), this, SLOT( updateData() ) );

    m_models.append( model );
    m_keyDims.append( dim );
}

void TableItemModel::buildAllIndexes()
{
    m_itemGroups.clear();

    buildRootIndex();

    for ( int i = 0; i < m_itemGroups.count(); i++ )
        buildChildIndexes( i );
}

void TableItemModel::buildRootIndex()
{
    m_totalCount = 0;

    addGroup( 0 );

    int dim = m_keyDims.at( 0 );

    if ( m_uniqueIndex.isValid() ) {
        IndexConstIterator<void> it( m_uniqueIndex );
        while ( it.next() ) {
            m_totalCount++;
            int id = it.key( dim );
            if ( m_filter && !m_filter->filterRow( id ) )
                continue;
            addItemToGroup( 0, id );
        }
    } else if ( m_foreignIndex.isValid() ) {
        ForeignConstIterator<void> it( m_foreignIndex, m_parentId );
        while ( it.next() ) {
            m_totalCount++;
            int id = it.key( dim );
            if ( m_filter && !m_filter->filterRow( id ) )
                continue;
            addItemToGroup( 0, id );
        }
    }

    sortGroup( 0 );
}

void TableItemModel::buildChildIndexes( int group )
{
    int level = m_itemGroups.at( group ).m_level;

    if ( level >= m_childIndexes.count() )
        return;

    int dim = m_keyDims.at( level + 1 );

    for ( int i = 0; i < m_itemGroups.at( group ).m_items.count(); i++ ) {
        int childGroup = -1;

        ForeignConstIterator<void> it( m_childIndexes.at( level ), m_itemGroups.at( group ).m_items.at( i ).m_id );
        while ( it.next() ) {
            if ( childGroup < 0 ) {
                childGroup = addGroup( level + 1 );
                m_itemGroups[ group ].m_items[ i ].m_childGroup = childGroup;
            }
            addItemToGroup( childGroup, it.key( dim ) );
        }

        if ( childGroup >= 0 )
            sortGroup( childGroup );
    }
}

int TableItemModel::addGroup( int level )
{
    ItemGroup group;
    group.m_level = level;
    m_itemGroups.append( group );

    return m_itemGroups.count() - 1;
}

void TableItemModel::addItemToGroup( int group, int id )
{
    Item item;
    item.m_id = id;
    item.m_childGroup = -1;
    m_itemGroups[ group ].m_items.append( item );
}

void TableItemModel::sortAllGroups()
{
    if ( m_sortColumn < 0 )
        return;

    emit layoutAboutToBeChanged();

    QModelIndexList oldIndexes = persistentIndexList();

    QList<CellIndex> cellIndexes;
    for ( int i = 0; i < oldIndexes.count(); i++ )
        cellIndexes.append( cellAt( oldIndexes.at( i ) ) );

    for ( int i = 0; i < m_itemGroups.count(); i++ )
        sortGroup( i );

    QModelIndexList newIndexes;
    for ( int i = 0; i < cellIndexes.count(); i++ )
        newIndexes.append( findCell( cellIndexes.at( i ) ) );

    changePersistentIndexList( oldIndexes, newIndexes );

    emit layoutChanged();
}

void TableItemModel::sortGroup( int group )
{
    if ( m_sortColumn < 0 )
        return;

    AbstractTableModel* model = m_models.at( m_itemGroups.at( group ).m_level );

    qSort( m_itemGroups[ group ].m_items.begin(), m_itemGroups[ group ].m_items.end(),
        CompareItems( model, m_sortColumn, m_sortOrder ) );
}

void TableItemModel::updateData()
{
    emit layoutAboutToBeChanged();

    QModelIndexList oldIndexes = persistentIndexList();

    QList<CellIndex> cellIndexes;
    for ( int i = 0; i < oldIndexes.count(); i++ )
        cellIndexes.append( cellAt( oldIndexes.at( i ) ) );

    buildAllIndexes();

    QModelIndexList newIndexes;
    for ( int i = 0; i < cellIndexes.count(); i++ )
        newIndexes.append( findCell( cellIndexes.at( i ) ) );

    changePersistentIndexList( oldIndexes, newIndexes );

    emit layoutChanged();
}

TableItemModel::CellIndex TableItemModel::cellAt( const QModelIndex& index )
{
    CellIndex cell;

    if ( !index.isValid() ) {
        cell.m_id = 0;
        cell.m_level = -1;
        cell.m_column = 0;
        return cell;
    }

    int group = index.internalId();

    cell.m_id = m_itemGroups.at( group ).m_items.at( index.row() ).m_id;
    cell.m_level = m_itemGroups.at( group ).m_level;
    cell.m_column = index.column();

    return cell;
}

QModelIndex TableItemModel::findCell( const CellIndex& cell )
{
    if ( cell.m_level < 0 )
        return QModelIndex();

    for ( int i = 0; i < m_itemGroups.count(); i++ ) {
        if ( m_itemGroups.at( i ).m_level == cell.m_level ) {
            for ( int j = 0; j < m_itemGroups.at( i ).m_items.count(); j++ ) {
                if ( m_itemGroups.at( i ).m_items.at( j ).m_id == cell.m_id )
                    return createIndex( j, cell.m_column, i );
            }
        }
    }

    return QModelIndex();
}

QModelIndex TableItemModel::findCell( int level, int id, int column )
{
    CellIndex index;
    index.m_level = level;
    index.m_id = id;
    index.m_column = m_columns.indexOf( column );
    return findCell( index );
}

TableItemModel::CompareItems::CompareItems( AbstractTableModel* model, int column, Qt::SortOrder order ) :
    m_model( model ),
    m_column( column ),
    m_order( order )
{
}

bool TableItemModel::CompareItems::operator()( const TableItemModel::Item& item1, const TableItemModel::Item& item2 )
{
    int result = m_model->compare( item1.m_id, item2.m_id, m_column );
    if ( result == 0 )
        return item1.m_id < item2.m_id;
    if ( m_order == Qt::AscendingOrder )
        return result < 0;
    else
        return result > 0;
}
