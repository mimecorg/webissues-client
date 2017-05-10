/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2017 WebIssues Team
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

#ifndef TYPESVIEW_H
#define TYPESVIEW_H

#include "views/view.h"

class TypesModel;

class QModelIndex;
class QTreeView;

/**
* View for displaying the tree of types and attributes.
*/
class TypesView : public View
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent The parent object.
    * @param parentWidget The parent widget of the view's main widget.
    */
    TypesView( QObject* parent, QWidget* parentWidget );

    /**
    * Destructor.
    */
    ~TypesView();

public: // overrides
    void initialUpdate();

private slots:
    void updateActions();

    void updateTypes();
    void addType();
    void addAttribute();
    void editRename();
    void editModify();
    void editDelete();
    void viewSettings();

    void contextMenu( const QPoint& pos );
    void doubleClicked( const QModelIndex& index );

private:
    QTreeView* m_list;
    TypesModel* m_model;

    int m_selectedTypeId;
    int m_selectedAttributeId;

    int m_currentTypeId;
};

#endif
