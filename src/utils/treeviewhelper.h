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

#ifndef TREEVIEWHELPER_H
#define TREEVIEWHELPER_H

#include <QMap>
#include <QPair>

class QTreeView;
class QModelIndex;

/**
* Helper class for reading and applying settings of a tree view.
*/
class TreeViewHelper
{
public:
    enum ViewFlag
    {
        TreeStyle = 1,
        MultiSelect = 2,
        NotSortable = 4
    };

    Q_DECLARE_FLAGS( ViewFlags, ViewFlag )

public:
    /**
    * Constructor.
    * @param view The associated tree view.
    */
    TreeViewHelper( QTreeView* view );

    /**
    * Destructor.
    */
    ~TreeViewHelper();

public:
    /**
    * Initialize common properties of the tree view.
    */
    void initializeView( ViewFlags flags = 0 );

    /**
    * Return the index of selected item in the view.
    */
    QModelIndex selectedIndex();

    /**
    * Save static column widths from the tree view.
    */
    void saveColumnWidths( const QString& key );

    /**
    * Load static column widths to the tree view.
    */
    void loadColumnWidths( const QString& key, const QList<int>& defaultWidths );

    /**
    * Save dynamic column widths from the tree view.
    */
    void saveColumnWidths( const QString& key, const QList<int>& columns );

    /**
    * Load dynamic column widths to the tree view.
    */
    void loadColumnWidths( const QString& key, const QList<int>& columns, const QMap<int, int>& defaultWidths );

    /**
    * Save expanded nodes from the tree view.
    */
    void saveExpandedNodes( const QString& key );

    /**
    * Load expanded nodes to the tree view.
    */
    void loadExpandedNodes( const QString& key );

private:
    QTreeView* m_view;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( TreeViewHelper::ViewFlags )

#endif
