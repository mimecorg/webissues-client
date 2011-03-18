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

#ifndef TREEVIEWHELPER_H
#define TREEVIEWHELPER_H

#include <QMap>
#include <QPair>

class QTreeView;
class QModelIndex;

/**
* Functions for reading and applying settings of a tree view.
*/
class TreeViewHelper
{
public:
    enum ColumnFlag
    {
        WideName = 1,
        UserColumns = 2
    };

    Q_DECLARE_FLAGS( ColumnFlags, ColumnFlag )

    enum ViewFlag
    {
        TreeStyle = 1,
        MultiSelect = 2,
        NotSortable = 4
    };

    Q_DECLARE_FLAGS( ViewFlags, ViewFlag )

public:
    /**
    * Initialize common properties of the tree view.
    */
    static void initializeView( QTreeView* view, ViewFlags flags = 0 );

    /**
    * Return the index of selected item in the view.
    */
    static QModelIndex selectedIndex( QTreeView* view );

    /**
    * Read sort order from the tree view.
    */
    static QPair<int, Qt::SortOrder> sortOrder( QTreeView* view );

    /**
    * Apply sort order to the tree view.
    */
    static void setSortOrder( QTreeView* view, const QPair<int, Qt::SortOrder>& order );

    /**
    * Save column widths from the tree view.
    */
    static void saveColumnWidths( QTreeView* view, const QString& type, ColumnFlags flags = 0 );

    /**
    * Load column widths to the tree view.
    */
    static void loadColumnWidths( QTreeView* view, const QString& type, ColumnFlags flags = 0 );

    /**
    * Save expanded nodes from the tree view.
    */
    static void saveExpandedNodes( QTreeView* view, const QString& type );

    /**
    * Load expanded nodes to the tree view.
    */
    static void loadExpandedNodes( QTreeView* view, const QString& type );

private:
    static int defaultColumnWidth( int column, ColumnFlags flags );
};

Q_DECLARE_OPERATORS_FOR_FLAGS( TreeViewHelper::ColumnFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( TreeViewHelper::ViewFlags )

#endif
