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

#ifndef MEMBERSDIALOG_H
#define MEMBERSDIALOG_H

#include "dialogs/informationdialog.h"
#include "xmlui/client.h"

class MembersModel;

class QTreeView;

/**
* Dialog for managing project members.
*/
class MembersDialog : public InformationDialog, public XmlUi::Client
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param projectId Identifier of the project.
    * @param parent The parent widget.
    */
    MembersDialog( int projectId, QWidget* parent );

    /**
    * Destructor.
    */
    ~MembersDialog();

private slots:
    void addMember();
    void changeAccess();
    void removeMember();

    void updateActions();

    void doubleClicked( const QModelIndex& index );
    void listContextMenu( const QPoint& pos );

private:
    int m_projectId;

    QTreeView* m_list;
    MembersModel* m_model;

    QList<int> m_selectedUsers;
};

#endif