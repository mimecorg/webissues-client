/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2012 WebIssues Team
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

#include "treeviewhelper.h"

#include "application.h"
#include "data/datamanager.h"
#include "data/localsettings.h"
#include "models/sqltreemodel.h"

#include <QTreeView>
#include <QHeaderView>

class ExpandedNode
{
public:
    ExpandedNode( int level = -1, int id = -1 ) :
        m_level( level ),
        m_id( id )
    {
    }

public:
    friend QDataStream& operator <<( QDataStream& stream, const ExpandedNode& node )
    {
        return stream << node.m_level << node.m_id;
    }

    friend QDataStream& operator >>( QDataStream& stream, ExpandedNode& node )
    {
        return stream >> node.m_level >> node.m_id;
    }

public:
    int m_level;
    int m_id;
};

typedef QList<ExpandedNode> ExpandedNodeList;

Q_DECLARE_METATYPE( ExpandedNodeList )

TreeViewHelper::TreeViewHelper( QTreeView* view ) :
    m_view( view )
{
    static bool registered = false;

    if ( !registered ) {
        qRegisterMetaTypeStreamOperators<ExpandedNodeList>( "ExpandedNodeList" );
        registered = true;
    }
}

TreeViewHelper::~TreeViewHelper()
{
}

void TreeViewHelper::initializeView( ViewFlags flags /*= 0*/ )
{
    m_view->setSortingEnabled( ( flags & NotSortable ) == 0 );
    m_view->setRootIsDecorated( ( flags & TreeStyle ) != 0 );

    if ( flags & TreeStyle )
        m_view->setExpandsOnDoubleClick( false );

    if ( flags & MultiSelect )
        m_view->setSelectionMode( QAbstractItemView::ExtendedSelection );

    m_view->setUniformRowHeights( true );

    m_view->header()->setStretchLastSection( false );
    m_view->header()->setMovable( false );

    m_view->setContextMenuPolicy( Qt::CustomContextMenu );
    m_view->header()->setContextMenuPolicy( Qt::CustomContextMenu );
}

QModelIndex TreeViewHelper::selectedIndex()
{
    if ( !m_view->selectionModel() )
        return QModelIndex();

    QModelIndexList selection = m_view->selectionModel()->selectedRows();
    if ( selection.isEmpty() )
        return QModelIndex();

    return selection.at( 0 );
}

void TreeViewHelper::saveColumnWidths( const QString& key )
{
    int count = m_view->model()->columnCount();

    QList<int> widths;
    for ( int i = 0; i < count; i++ )
        widths.append( m_view->columnWidth( i ) );

    application->applicationSettings()->setValue( key, QVariant::fromValue( widths ) );
}

void TreeViewHelper::loadColumnWidths( const QString& key, const QList<int>& defaultWidths )
{
    int count = m_view->model()->columnCount();

    QList<int> widths = application->applicationSettings()->value( key ).value<LocalSettings::IntList>();

    if ( widths.count() != count )
        widths = defaultWidths;

    for ( int i = 0; i < count; i++ )
        m_view->setColumnWidth( i, widths.value( i, 150 ) );
}

void TreeViewHelper::saveColumnWidths( const QString& key, const QList<int>& columns )
{
    int count = m_view->model()->columnCount();

    QMap<int, int> widths = dataManager->connectionSettings()->value( key ).value<LocalSettings::IntMap>();

    for ( int i = 0; i < count; i++ ) {
        if ( !m_view->isColumnHidden( i ) )
            widths.insert( columns.at( i ), m_view->columnWidth( i ) );
    }

    dataManager->connectionSettings()->setValue( key, QVariant::fromValue( widths ) );
}

void TreeViewHelper::loadColumnWidths( const QString& key, const QList<int>& columns, const QMap<int, int>& defaultWidths )
{
    int count = m_view->model()->columnCount();

    QMap<int, int> widths = dataManager->connectionSettings()->value( key ).value<LocalSettings::IntMap>();

    for ( int i = 0; i < count; i++ )
        m_view->setColumnWidth( i, widths.value( columns.at( i ), defaultWidths.value( columns.at( i ), 150 ) ) );
}

void TreeViewHelper::saveExpandedNodes( const QString& key )
{
    SqlTreeModel* model = qobject_cast<SqlTreeModel*>( m_view->model() );
    if ( !model )
        return;

    QList<ExpandedNode> nodes;

    QList<QModelIndex> indexes;
    indexes.append( QModelIndex() );

    while ( !indexes.isEmpty() ) {
        QModelIndex parent = indexes.takeFirst();

        int count = model->rowCount( parent );

        for ( int i = 0; i < count; i++ ) {
            QModelIndex index = model->index( i, 0, parent );
            if ( m_view->isExpanded( index ) ) {
                nodes.append( ExpandedNode( model->levelOf( index ), model->rowId( index ) ) );
                indexes.append( index );
            }
        }
    }

    dataManager->connectionSettings()->setValue( key, QVariant::fromValue( nodes ) );
}

void TreeViewHelper::loadExpandedNodes( const QString& key )
{
    SqlTreeModel* model = qobject_cast<SqlTreeModel*>( m_view->model() );
    if ( !model )
        return;

    QList<ExpandedNode> nodes = dataManager->connectionSettings()->value( key ).value<ExpandedNodeList>();

    foreach ( const ExpandedNode& node, nodes ) {
        QModelIndex index = model->findIndex( node.m_level, node.m_id, 0 );
        if ( index.isValid() )
            m_view->setExpanded( index, true );
    }
}
