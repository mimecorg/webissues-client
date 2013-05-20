/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2013 WebIssues Team
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#include "sqltreemodel.h"

#include <QSqlQueryModel>
#include <QVector>
#include <QHash>

class SqlTreeModelNode
{
public:
    SqlTreeModelNode( int level = 0, int index = 0 ) :
        m_level( level ),
        m_index( index )
    {
    }

public:
    int m_level;
    int m_index;
    QList<int> m_rows;
};

class SqlTreeModelLevel
{
public:
    SqlTreeModelLevel( QSqlQueryModel* model ) :
        m_model( model )
    {
    }

    ~SqlTreeModelLevel()
    {
        clear();
    }

    void clear()
    {
        qDeleteAll( m_nodes );
        m_ids.clear();
        m_nodes.clear();
    }

public:
    QSqlQueryModel* m_model;

    QList<int> m_columnMapping;

    QVector<int> m_ids;
    QVector<int> m_parentIds;
    QHash<int, SqlTreeModelNode*> m_nodes;
};

class SqlTreeModelPrivate
{
public:
    SqlTreeModelPrivate( SqlTreeModel* model ) :
        q( model ),
        m_columns( -1 ),
        m_sortColumn( -1 ),
        m_sortOrder( Qt::AscendingOrder )
    {
    }

    ~SqlTreeModelPrivate()
    {
        qDeleteAll( m_levelData );
    }

public:
    const SqlTreeModelNode* findNode( const QModelIndex& parent ) const;

public:
    SqlTreeModel* q;

    QList<SqlTreeModelLevel*> m_levelData;

    int m_columns;
    QVector<QHash<int, QVariant> > m_headers;

    int m_sortColumn;
    Qt::SortOrder m_sortOrder;

    SqlTreeModelNode m_root;
};

SqlTreeModel::SqlTreeModel( QObject* parent ) : QAbstractItemModel( parent ),
    d( new SqlTreeModelPrivate( this ) )
{
}

SqlTreeModel::~SqlTreeModel()
{
    delete d;
}

void SqlTreeModel::appendModel( QSqlQueryModel* model )
{
    d->m_levelData.append( new SqlTreeModelLevel( model ) );
    d->m_columns = -1;
}

QSqlQueryModel* SqlTreeModel::modelAt( int level ) const
{
    return d->m_levelData.at( level )->m_model;
}

void SqlTreeModel::setColumnMapping( int level, const QList<int>& columnMapping )
{
    SqlTreeModelLevel* levelData = d->m_levelData.at( level );
    if ( levelData->m_columnMapping != columnMapping ) {
        levelData->m_columnMapping = columnMapping;
        d->m_columns = -1;
    }
}

void SqlTreeModel::updateData()
{
    bool columnsChanged = false;

    if ( d->m_columns == -1 ) {
        for ( int level = 0; level < d->m_levelData.count(); level++ ) {
            SqlTreeModelLevel* levelData = d->m_levelData.at( level );

            if ( levelData->m_columnMapping.isEmpty() ) {
                int count = levelData->m_model->columnCount();
                for ( int i = ( level == 0 ) ? 1 : 2; i < count; i++ )
                    levelData->m_columnMapping.append( i );
            }

            d->m_columns = qMax( d->m_columns, levelData->m_columnMapping.count() );
        }

        columnsChanged = true;
    }

    QModelIndexList oldIndexes;
    QList<int> oldLevels;
    QList<int> oldIds;

    if ( !columnsChanged ) {
        emit layoutAboutToBeChanged();

        oldIndexes = persistentIndexList();

        for ( int i = 0; i < oldIndexes.count(); i++ ) {
            oldLevels.append( levelOf( oldIndexes.at( i ) ) );
            oldIds.append( rowId( oldIndexes.at( i ) ) );
        }
    }

    foreach ( SqlTreeModelLevel* levelData, d->m_levelData )
        levelData->clear();
    d->m_root.m_rows.clear();

    for ( int level = 0; level < d->m_levelData.count(); level++ ) {
        SqlTreeModelLevel* levelData = d->m_levelData.at( level );
        QSqlQueryModel* model = levelData->m_model;

        while ( model->canFetchMore() )
            model->fetchMore();

        int count = model->rowCount();

        levelData->m_ids.resize( count );
        if ( level > 0 )
            levelData->m_parentIds.resize( count );

        for ( int row = 0; row < count; row++ ) {
            int id = model->data( model->index( row, 0 ) ).toInt();
            levelData->m_ids[ row ] = id;

            SqlTreeModelNode* node;

            if ( level == 0 ) {
                node = &d->m_root;
            } else {
                int parentId = model->data( model->index( row, 1 ) ).toInt();
                levelData->m_parentIds[ row ] = parentId;

                SqlTreeModelLevel* parentLevelData = d->m_levelData.at( level - 1 );

                int parentRow = parentLevelData->m_ids.indexOf( parentId );
                if ( parentRow < 0 )
                    continue;

                node = parentLevelData->m_nodes.value( parentRow );

                if ( !node ) {
                    node = new SqlTreeModelNode( level, parentRow );
                    parentLevelData->m_nodes.insert( parentRow, node ); 
                }
            }

            node->m_rows.append( row );
        }
    }

    if ( !columnsChanged ) {
        QModelIndexList newIndexes;
        for ( int i = 0; i < oldIndexes.count(); i++ )
            newIndexes.append( findIndex( oldLevels.at( i ), oldIds.at( i ), oldIndexes.at( i ).column() ) );

        changePersistentIndexList( oldIndexes, newIndexes );

        emit layoutChanged();
    } else {
        reset();
    }
}

int SqlTreeModel::levelOf( const QModelIndex& index ) const
{
    if ( !index.isValid() )
        return -1;

    SqlTreeModelNode* node = (SqlTreeModelNode*)index.internalPointer();
    if ( !node )
        return -1;

    return node->m_level;
}

int SqlTreeModel::mappedRow( const QModelIndex& index ) const
{
    if ( !index.isValid() )
        return -1;

    SqlTreeModelNode* node = (SqlTreeModelNode*)index.internalPointer();
    if ( !node )
        return -1;

    return node->m_rows.value( index.row(), -1 );
}

int SqlTreeModel::mappedColumn( const QModelIndex& index ) const
{
    int level = levelOf( index );
    if ( level < 0 )
        return -1;

    SqlTreeModelLevel* levelData = d->m_levelData.at( level );

    return levelData->m_columnMapping.value( index.column(), -1 );
}

int SqlTreeModel::rowId( const QModelIndex& index ) const
{
    int level = levelOf( index );
    if ( level < 0 )
        return -1;

    SqlTreeModelLevel* levelData = d->m_levelData.at( level );

    int row = mappedRow( index );
    if ( row < 0 )
        return -1;

    return levelData->m_ids.at( row );
}

int SqlTreeModel::rowParentId( const QModelIndex& index ) const
{
    int level = levelOf( index );
    if ( level < 1 )
        return -1;

    SqlTreeModelLevel* levelData = d->m_levelData.at( level );

    int row = mappedRow( index );
    if ( row < 0 )
        return -1;

    return levelData->m_parentIds.at( row );
}

QVariant SqlTreeModel::rawData( int level, int row, int column, int role ) const
{
    if ( level < 0 || row < 0 || column < 0 )
        return QVariant();

    SqlTreeModelLevel* levelData = d->m_levelData.at( level );
    QSqlQueryModel* model = levelData->m_model;

    return model->data( model->index( row, column ), role );
}

QModelIndex SqlTreeModel::findIndex( int level, int id, int column ) const
{
    if ( level < 0 || id < 0 || column < 0 )
        return QModelIndex();

    SqlTreeModelLevel* levelData = d->m_levelData.at( level );

    int row = levelData->m_ids.indexOf( id );
    if ( row < 0 )
        return QModelIndex();

    const SqlTreeModelNode* node;

    if ( level == 0 ) {
        node = &d->m_root;
    } else {
        int parentId = levelData->m_parentIds.at( row );

        SqlTreeModelLevel* parentLevelData = d->m_levelData.at( level - 1 );

        int parentRow = parentLevelData->m_ids.indexOf( parentId );
        if ( parentRow < 0 )
            return QModelIndex();

        node = parentLevelData->m_nodes.value( parentRow );
        if ( !node )
            return QModelIndex();
    }

    int nodeRow = node->m_rows.indexOf( row );
    if ( nodeRow < 0 )
        return QModelIndex();

    return createIndex( nodeRow, column, (void*)node );
}

const SqlTreeModelNode* SqlTreeModelPrivate::findNode( const QModelIndex& parent ) const
{
    if ( !parent.isValid() )
        return &m_root;

    int level = q->levelOf( parent );
    if ( level < 0 )
        return NULL;

    SqlTreeModelLevel* levelData = m_levelData.at( level );

    int row = q->mappedRow( parent );
    if ( row < 0 )
        return NULL;

    return levelData->m_nodes.value( row );
}

int SqlTreeModel::columnCount( const QModelIndex& /*parent*/ ) const
{
    return qMax( d->m_columns, 0 );
}

int SqlTreeModel::rowCount( const QModelIndex& parent ) const
{
    const SqlTreeModelNode* node = d->findNode( parent );
    return node ? node->m_rows.count() : 0;
}

QModelIndex SqlTreeModel::index( int row, int column, const QModelIndex& parent ) const
{
    const SqlTreeModelNode* node = d->findNode( parent );
    if ( !node || row < 0 || row >= node->m_rows.count() )
        return QModelIndex();
    return createIndex( row, column, (void*)node );
}

QModelIndex SqlTreeModel::parent( const QModelIndex& index ) const
{
    if ( !index.isValid() )
        return QModelIndex();

    SqlTreeModelNode* node = (SqlTreeModelNode*)index.internalPointer();
    if ( !node )
        return QModelIndex();

    int level = node->m_level;
    if ( level < 1 )
        return QModelIndex();

    int row = node->m_index;

    const SqlTreeModelNode* parentNode;

    if ( level == 1 ) {
        parentNode = &d->m_root;
    } else {
        SqlTreeModelLevel* levelData = d->m_levelData.at( level - 1 );
        int parentId = levelData->m_parentIds.at( row );

        SqlTreeModelLevel* parentLevelData = d->m_levelData.at( level - 2 );

        int parentRow = parentLevelData->m_ids.indexOf( parentId );
        if ( parentRow < 0 )
            return QModelIndex();

        parentNode = parentLevelData->m_nodes.value( parentRow );
        if ( !parentNode )
            return QModelIndex();
    }

    int nodeRow = parentNode->m_rows.indexOf( row );
    if ( nodeRow < 0 )
        return QModelIndex();

    return createIndex( nodeRow, 0, (void*)parentNode );
}

QVariant SqlTreeModel::data( const QModelIndex& index, int role ) const
{
    return rawData( levelOf( index ), mappedRow( index ), mappedColumn( index ), role );
}

bool SqlTreeModel::setHeaderData( int section, Qt::Orientation orientation, const QVariant &value, int role )
{
    if ( orientation != Qt::Horizontal || section < 0 )
        return false;

    if ( d->m_headers.size() <= section )
        d->m_headers.resize( qMax( section + 1, 16 ) );

    d->m_headers[ section ][ role ] = value;

    emit headerDataChanged( orientation, section, section );

    return true;
}

QVariant SqlTreeModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    if ( orientation == Qt::Horizontal ) {
        QVariant value = d->m_headers.value( section ).value( role );

        if ( role == Qt::DisplayRole && !value.isValid() )
            value = d->m_headers.value( section ).value( Qt::EditRole );

        if ( value.isValid() )
            return value;

        SqlTreeModelLevel* levelData = d->m_levelData.first();
        int column = levelData->m_columnMapping.value( section, -1 );
        if ( column >= 0 )
            return levelData->m_model->headerData( column, Qt::Horizontal, role );
    }

    return QAbstractItemModel::headerData( section, orientation, role );
}

void SqlTreeModel::setSort( int column, Qt::SortOrder order )
{
    d->m_sortColumn = column;
    d->m_sortOrder = order;
}

int SqlTreeModel::sortColumn() const
{
    return d->m_sortColumn;
}

Qt::SortOrder SqlTreeModel::sortOrder() const
{
    return d->m_sortOrder;
}

void SqlTreeModel::sort( int column, Qt::SortOrder order )
{
    if ( d->m_sortColumn != column || d->m_sortOrder != order ) {
        setSort( column, order );
        updateQueries();
    }
}

void SqlTreeModel::updateQueries()
{
}
