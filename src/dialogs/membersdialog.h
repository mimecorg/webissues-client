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

#ifndef MEMBERSDIALOG_H
#define MEMBERSDIALOG_H

#include "dialogs/informationdialog.h"
#include "xmlui/client.h"

class MembersModel;

class QTreeView;
class QModelIndex;
class QLineEdit;

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
    */
    MembersDialog( int projectId );

    /**
    * Destructor.
    */
    ~MembersDialog();

protected: // overrides
    void customEvent( QEvent* e );

private slots:
    void changeGlobalAccess();

    void addMember();
    void changeAccess();
    void removeMember();

    void updateActions();

    void doubleClicked( const QModelIndex& index );
    void listContextMenu( const QPoint& pos );

private:
    void updateGlobalAccess();

private:
    int m_projectId;

    QLineEdit* m_globalEdit;

    QTreeView* m_list;
    MembersModel* m_model;

    QList<int> m_selectedUsers;
};

#endif
