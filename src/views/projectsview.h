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

#ifndef PROJECTSVIEW_H
#define PROJECTSVIEW_H

#include "views/view.h"

class ProjectsModel;

class QTreeView;
class QModelIndex;

/**
* View for displaying the tree of projects and folders.
*/
class ProjectsView : public View
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent The parent object.
    * @param parentWidget The parent widget of the view's main widget.
    */
    ProjectsView( QObject* parent, QWidget* parentWidget );

    /**
    * Destructor.
    */
    ~ProjectsView();

public:
    /**
    * Select specified folder or alert.
    */
    void setSelection( int folderId, int viewId );

public: // overrides
    void initialUpdate();

signals:
    /**
    * Emitted when the selected folder or alert is changed.
    */
    void selectionChanged( int folderId, int viewId );

    /**
    * Emitted when the selected project is changed.
    */
    void projectSelected( int projectId );

protected: // overrides
    void updateAccess( Access access );

    void updateEvent( UpdateEvent* e );

    void updateFailed();

private slots:
    void updateActions();
    void updateSelection();

    void updateTimeout();

    void updateProjects();
    void showMembers();
    void addProject();
    void addFolder();
    void editRename();
    void editDelete();
    void moveFolder();
    void openFolder();
    void manageAlerts();

    void contextMenu( const QPoint& pos );
    void doubleClicked( const QModelIndex& index );

private:
    Access checkDataAccess();

    void initialUpdateData();
    void periodicUpdateData( bool full );
    void cascadeUpdateFolders();

private:
    QTreeView* m_list;
    ProjectsModel* m_model;

    QTimer* m_timer;

    int m_folderUpdateCounter;
    int m_updateCounter;

    int m_selectedProjectId;
    int m_selectedFolderId;
    int m_selectedViewId;

    int m_currentProjectId;
    bool m_currentProjectAdmin;
    int m_currentFolderId;

    bool m_systemAdmin;
    bool m_anyProjectAdmin;

    bool m_sessionExpired;
};

#endif
