/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2014 WebIssues Team
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

#ifndef USERSVIEW_H
#define USERSVIEW_H

#include "views/view.h"

class UsersModel;
class FilterLabel;

class QTreeView;
class QModelIndex;

/**
* View for displaying and managing users.
*/
class UsersView : public View
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent The parent object.
    * @param parentWidget The parent widget of the view's main widget.
    */
    UsersView( QObject* parent, QWidget* parentWidget );

    /**
    * Destructor.
    */
    ~UsersView();

public: // overrides
    void initialUpdate();

private slots:
    void updateActions();

    void updateUsers();
    void addUser();
    void editRename();
    void changePassword();
    void managePermissions();
    void userPreferences();

    void filterChanged( int index );

    void contextMenu( const QPoint& pos );
    void doubleClicked( const QModelIndex& index );

private:
    QTreeView* m_list;
    UsersModel* m_model;

    FilterLabel* m_filterLabel;

    int m_selectedUserId;

    bool m_systemAdmin;
};

#endif
