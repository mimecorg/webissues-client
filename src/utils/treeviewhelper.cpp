/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2011 WebIssues Team
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
#include "models/tablemodels.h"
#include "rdb/tableitemmodel.h"

#include <QTreeView>
#include <QHeaderView>

void TreeViewHelper::initializeView( QTreeView* view, ViewFlags flags /*= 0*/ )
{
    view->setSortingEnabled( ( flags & NotSortable ) == 0 );
    view->setRootIsDecorated( ( flags & TreeStyle ) != 0 );

    if ( flags & TreeStyle )
        view->setExpandsOnDoubleClick( false );

    if ( flags & MultiSelect )
        view->setSelectionMode( QAbstractItemView::ExtendedSelection );

    view->setUniformRowHeights( true );

    view->header()->setStretchLastSection( false );
    view->header()->setMovable( false );

    view->setContextMenuPolicy( Qt::CustomContextMenu );
    view->header()->setContextMenuPolicy( Qt::CustomContextMenu );
}

QModelIndex TreeViewHelper::selectedIndex( QTreeView* view )
{
    if ( !view->selectionModel() )
        return QModelIndex();

    QModelIndexList selection = view->selectionModel()->selectedRows();
    if ( selection.isEmpty() )
        return QModelIndex();

    return selection.at( 0 );
}

QPair<int, Qt::SortOrder> TreeViewHelper::sortOrder( QTreeView* view )
{
    RDB::TableItemModel* model = (RDB::TableItemModel*)view->model();

    QPair<int, Qt::SortOrder> order;
    order.first = model->sortColumn();
    order.second = model->sortOrder();

    return order;
}

void TreeViewHelper::setSortOrder( QTreeView* view, const QPair<int, Qt::SortOrder>& order )
{
    RDB::TableItemModel* model = (RDB::TableItemModel*)view->model();

    int index = model->columns().indexOf( order.first );
    if ( index >= 0 )
        view->sortByColumn( index, order.second );
    else
        view->sortByColumn( 0, Qt::AscendingOrder );
}

void TreeViewHelper::saveColumnWidths( QTreeView* view, const QString& type, ColumnFlags flags /*= 0*/ )
{
    QString key = QString( "%1Widths" ).arg( type );
    QMap<int, int> widths = application->applicationSettings()->value( key ).value<LocalSettings::IntMap>();

    QMap<int, int> userWidths;
    if ( flags & UserColumns )
        userWidths = dataManager->connectionSettings()->value( "ColumnWidths" ).value<LocalSettings::IntMap>();

    int count = view->model()->columnCount();
    if ( view->header()->stretchLastSection() )
        count--;

    for ( int i = 0; i < count; i++ ) {
        int column = view->model()->headerData( i, Qt::Horizontal, RDB::TableItemModel::ColumnRole ).toInt();
        int width = view->columnWidth( i );
        if ( ( flags & UserColumns ) && column > Column_UserDefined )
            userWidths.insert( column, width );
        else
            widths.insert( column, width );
    }

    application->applicationSettings()->setValue( key, QVariant::fromValue( widths ) );

    if ( flags & UserColumns )
        dataManager->connectionSettings()->setValue( "ColumnWidths", QVariant::fromValue( userWidths ) );
}

void TreeViewHelper::loadColumnWidths( QTreeView* view, const QString& type, ColumnFlags flags /*= 0*/ )
{
    QString key = QString( "%1Widths" ).arg( type );
    QMap<int, int> widths = application->applicationSettings()->value( key ).value<LocalSettings::IntMap>();

    QMap<int, int> userWidths;
    if ( flags & UserColumns )
        userWidths = dataManager->connectionSettings()->value( "ColumnWidths" ).value<LocalSettings::IntMap>();

    for ( int i = 0; i < view->model()->columnCount(); i++ ) {
        int column = view->model()->headerData( i, Qt::Horizontal, RDB::TableItemModel::ColumnRole ).toInt();
        int width = defaultColumnWidth( column, flags );
        if ( ( flags & UserColumns ) && column > Column_UserDefined )
            width = userWidths.value( column, width );
        else
            width = widths.value( column, width );
        view->setColumnWidth( i, width );
    }
}

void TreeViewHelper::saveExpandedNodes( QTreeView* view, const QString& type )
{
    QList<int> nodes;
    QList<int> nodes2;

    for ( int i = 0; i < view->model()->rowCount(); i++ ) {
        QModelIndex index = view->model()->index( i, 0 );
        if ( view->isExpanded( index ) )
            nodes.append( view->model()->data( index, RDB::TableItemModel::RowIdRole ).toInt() );
        for ( int j = 0; j < view->model()->rowCount( index ); j++ ) {
            QModelIndex index2 = view->model()->index( j, 0, index );
            if ( view->isExpanded( index2 ) )
                nodes2.append( view->model()->data( index2, RDB::TableItemModel::RowIdRole ).toInt() );
        }
    }

    QString key = QString( "%1Expanded" ).arg( type );
    dataManager->connectionSettings()->setValue( key, QVariant::fromValue( nodes ) );

    QString key2 = QString( "%1Expanded2" ).arg( type );
    dataManager->connectionSettings()->setValue( key2, QVariant::fromValue( nodes2 ) );
}

void TreeViewHelper::loadExpandedNodes( QTreeView* view, const QString& type )
{
    QString key = QString( "%1Expanded" ).arg( type );
    QList<int> nodes = dataManager->connectionSettings()->value( key ).value<LocalSettings::IntList>();

    QString key2 = QString( "%1Expanded2" ).arg( type );
    QList<int> nodes2 = dataManager->connectionSettings()->value( key2 ).value<LocalSettings::IntList>();

    for ( int i = 0; i < view->model()->rowCount(); i++ ) {
        QModelIndex index = view->model()->index( i, 0 );
        int id = view->model()->data( index, RDB::TableItemModel::RowIdRole ).toInt();
        if ( nodes.contains( id ) )
            view->setExpanded( index, true );
        for ( int j = 0; j < view->model()->rowCount( index ); j++ ) {
            QModelIndex index2 = view->model()->index( j, 0, index );
            int id2 = view->model()->data( index2, RDB::TableItemModel::RowIdRole ).toInt();
            if ( nodes2.contains( id2 ) )
                view->setExpanded( index2, true );
        }
    }
}

int TreeViewHelper::defaultColumnWidth( int column, ColumnFlags flags )
{
    switch ( column ) {
        case Column_ID:
            return 50;
        case Column_Name:
            return ( flags & WideName ) ? 300 : 150;
        case Column_Login:
        case Column_Required:
            return 100;
        case Column_Status:
            return 250;
        case Column_Details:
        case Column_Columns:
        case Column_Filter:
        case Column_Address:
            return 300;
        default:
            return 150;
    }
}
