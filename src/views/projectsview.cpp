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

#include "projectsview.h"

#include "application.h"
#include "commands/updatebatch.h"
#include "data/datamanager.h"
#include "data/entities.h"
#include "data/localsettings.h"
#include "dialogs/projectdialogs.h"
#include "dialogs/membersdialog.h"
#include "dialogs/managealertsdialog.h"
#include "models/projectsmodel.h"
#include "utils/treeviewhelper.h"
#include "utils/iconloader.h"
#include "views/viewmanager.h"
#include "xmlui/builder.h"

#include <QTreeView>
#include <QAction>
#include <QMenu>
#include <QTimer>
#include <QMessageBox>

ProjectsView::ProjectsView( QObject* parent, QWidget* parentWidget ) : View( parent ),
    m_updateCounter( 0 )
{
    m_systemAdmin = dataManager->currentUserAccess() == AdminAccess;

    QAction* action;

    action = new QAction( IconLoader::icon( "file-reload" ), tr( "&Update Projects" ), this );
    action->setShortcut( QKeySequence::Refresh );
    connect( action, SIGNAL( triggered() ), this, SLOT( updateProjects() ), Qt::QueuedConnection );
    setAction( "updateProjects", action );

    action = new QAction( IconLoader::icon( "view-members" ), tr( "Project &Members..." ), this );
    action->setIconText( tr( "Members" ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( showMembers() ), Qt::QueuedConnection );
    setAction( "showMembers", action );

    if ( m_systemAdmin ) {
        action = new QAction( IconLoader::icon( "project-new" ), tr( "Add &Project..." ), this );
        connect( action, SIGNAL( triggered() ), this, SLOT( addProject() ), Qt::QueuedConnection );
        setAction( "addProject", action );
    }

    action = new QAction( IconLoader::icon( "folder-new" ), tr( "Add &Folder..." ), this );
    action->setShortcut( QKeySequence::New );
    connect( action, SIGNAL( triggered() ), this, SLOT( addFolder() ), Qt::QueuedConnection );
    setAction( "addFolder", action );

    action = new QAction( IconLoader::icon( "edit-rename" ), tr( "&Rename Folder..." ), this );
    action->setIconText( tr( "Rename" ) );
    action->setShortcut( tr( "F2" ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( editRename() ), Qt::QueuedConnection );
    setAction( "editRename", action );

    action = new QAction( IconLoader::icon( "edit-delete" ), tr( "&Delete Folder" ), this );
    action->setIconText( tr( "Delete" ) );
    action->setShortcut( QKeySequence::Delete );
    connect( action, SIGNAL( triggered() ), this, SLOT( editDelete() ), Qt::QueuedConnection );
    setAction( "editDelete", action );

    action = new QAction( IconLoader::icon( "folder-move" ), tr( "&Move Folder..." ), this );
    action->setIconText( tr( "Move" ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( moveFolder() ), Qt::QueuedConnection );
    setAction( "moveFolder", action );

    action = new QAction( IconLoader::icon( "folder-open" ), tr( "&Open Folder" ), this );
    action->setShortcut( QKeySequence::Open );
    connect( action, SIGNAL( triggered() ), this, SLOT( openFolder() ), Qt::QueuedConnection );
    setAction( "openFolder", action );

    action = new QAction( IconLoader::icon( "configure-alerts" ), tr( "&Manage Alerts..." ), this );
    connect( action, SIGNAL( triggered() ), this, SLOT( manageAlerts() ), Qt::QueuedConnection );
    setAction( "manageAlerts", action );

    setTitle( "sectionAdd", tr( "Add" ) );
    setTitle( "sectionProjects", tr( "Projects" ) );

    setDefaultMenuAction( "menuFolder", "openFolder" );

    loadXmlUiFile( ":/resources/projectsview.xml" );

    m_list = new QTreeView( parentWidget );

    TreeViewHelper helper( m_list );
    helper.initializeView( TreeViewHelper::TreeStyle );

    connect( m_list, SIGNAL( customContextMenuRequested( const QPoint& ) ),
        this, SLOT( contextMenu( const QPoint& ) ) );
    connect( m_list, SIGNAL( doubleClicked( const QModelIndex& ) ),
        this, SLOT( doubleClicked( const QModelIndex& ) ) );

    setMainWidget( m_list );
}

ProjectsView::~ProjectsView()
{
    TreeViewHelper helper( m_list );
    helper.saveColumnWidths( "ProjectsViewWidths" );
    helper.saveExpandedNodes( "ExpandedProjects" );
}

void ProjectsView::initialUpdate()
{
    m_list->sortByColumn( 0, Qt::AscendingOrder );

    m_model = new ProjectsModel( this );
    m_list->setModel( m_model );

    TreeViewHelper helper( m_list );
    helper.loadColumnWidths( "ProjectsViewWidths", QList<int>() << 150 << 150 );
    helper.loadExpandedNodes( "ExpandedProjects" );

    setCaption( dataManager->serverName() );

    connect( m_list->selectionModel(), SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
        this, SLOT( updateSelection() ) );
    connect( m_model, SIGNAL( layoutChanged() ), this, SLOT( updateActions() ) );

    updateActions();

    initialUpdateData();

    m_timer = new QTimer( this );
    connect( m_timer, SIGNAL( timeout() ), this, SLOT( updateTimeout() ) );
    m_timer->start( 60 * 1000 );

    setAccess( checkDataAccess(), true );
}

Access ProjectsView::checkDataAccess()
{
    m_anyProjectAdmin = m_systemAdmin;

    int userId = dataManager->currentUserId();
    foreach ( const MemberEntity& member, MemberEntity::list( userId ) ) {
        if ( member.access() == AdminAccess )
            m_anyProjectAdmin = true;
    }

    return m_anyProjectAdmin ? AdminAccess : NormalAccess;
}

void ProjectsView::updateAccess( Access /*access*/ )
{
    action( "showMembers" )->setVisible( m_anyProjectAdmin );
    action( "addFolder" )->setVisible( m_anyProjectAdmin );
    action( "editRename" )->setVisible( m_systemAdmin || m_anyProjectAdmin );
    action( "editDelete" )->setVisible( m_systemAdmin || m_anyProjectAdmin );
    action( "moveFolder" )->setVisible( m_systemAdmin || m_anyProjectAdmin );
}

void ProjectsView::updateTimeout()
{
    LocalSettings* settings = application->applicationSettings();
    int updateInterval = settings->value( "UpdateInterval" ).toInt();

    m_updateCounter++;

    if ( m_updateCounter >= updateInterval ) {
        m_updateCounter = 0;
        periodicUpdateData( true );
    } else {
        periodicUpdateData( false );
    }
}

void ProjectsView::initialUpdateData()
{
    UpdateBatch* batch = new UpdateBatch( -1 );
    batch->updateSettings();
    batch->updateUsers();
    batch->updateTypes();
    batch->updateProjects();
    batch->updateStates();

    executeUpdate( batch );
}

void ProjectsView::periodicUpdateData( bool full )
{
    UpdateBatch* batch = new UpdateBatch( -15 );
    if ( full ) {
        batch->updateUsers();
        batch->updateTypes();
    }
    batch->updateProjects();
    batch->updateStates();

    executeUpdate( batch );
}

void ProjectsView::cascadeUpdateFolders()
{
    UpdateBatch* batch = NULL;

    foreach ( const FolderEntity& folder, FolderEntity::list() ) {
        if ( !folder.alerts().isEmpty() ) {
            if ( dataManager->folderUpdateNeeded( folder.id() ) ) {
                if ( !batch ) {
                    batch = new UpdateBatch( -20 );
                    batch->setIfNeeded( true );
                }
                batch->updateFolder( folder.id() );
            }
        }
    }

    if ( batch )
        executeUpdate( batch );
}

void ProjectsView::updateActions()
{
    m_selectedProjectId = 0;
    m_selectedFolderId = 0;
    m_selectedViewId = 0;

    m_currentProjectId = 0;
    m_currentFolderId = 0;

    TreeViewHelper helper( m_list );
    QModelIndex index = helper.selectedIndex();

    if ( index.isValid() ) {
        int level = m_model->levelOf( index );
        int rowId = m_model->rowId( index );
        if ( level == 0 ) {
            m_selectedProjectId = rowId;
            m_currentProjectId = rowId;
        } else if ( level == 1 ) {
            m_selectedFolderId = rowId;
            m_currentFolderId = rowId;
            FolderEntity folder = FolderEntity::find( rowId );
            m_currentProjectId = folder.projectId();
        } else {
            AlertEntity alert = AlertEntity::find( rowId );
            FolderEntity folder = alert.folder();
            m_selectedViewId = alert.viewId();
            m_currentFolderId = folder.id();
            m_currentProjectId = folder.projectId();
        }
    }

    m_currentProjectAdmin = ProjectEntity::isAdmin( m_currentProjectId );

    action( "showMembers" )->setEnabled( m_selectedProjectId && m_currentProjectAdmin );
    action( "openFolder" )->setEnabled( m_selectedFolderId != 0 );
    action( "addFolder" )->setEnabled( m_currentProjectId != 0 && m_currentProjectAdmin );
    action( "editRename" )->setEnabled( ( m_selectedProjectId != 0 && m_systemAdmin ) || ( m_selectedFolderId != 0 && m_currentProjectAdmin ) );
    action( "editDelete" )->setEnabled( ( m_selectedProjectId != 0 && m_systemAdmin ) || ( m_selectedFolderId != 0 && m_currentProjectAdmin ) );
    action( "moveFolder" )->setEnabled( m_selectedFolderId != 0 && m_currentProjectAdmin );
    action( "manageAlerts" )->setEnabled( m_currentFolderId != 0 );

    action( "editRename" )->setText( m_selectedFolderId != 0 ? tr( "&Rename Folder..." ) : tr( "&Rename Project..." ) );
    action( "editDelete" )->setText( m_selectedFolderId != 0 ? tr( "&Delete Folder" ) : tr( "&Delete Project" ) );
}

void ProjectsView::updateSelection()
{
    updateActions();

    emit selectionChanged( m_currentFolderId, m_selectedViewId );
}

void ProjectsView::setSelection( int folderId, int viewId )
{
    if ( m_currentFolderId == folderId && m_selectedViewId == viewId )
        return;

    if ( viewId != 0 ) {
        FolderEntity folder = FolderEntity::find( folderId );
        foreach ( const AlertEntity& alert, folder.alerts() ) {
            if ( alert.viewId() == viewId ) {
                QModelIndex index = m_model->findIndex( 2, alert.id(), 0 );
                if ( index.isValid() ) {
                    m_list->selectionModel()->setCurrentIndex( index, QItemSelectionModel::Current );
                    m_list->selectionModel()->select( index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
                    return;
                }
                break;
            }
        }
    }

    QModelIndex index = m_model->findIndex( 1, folderId, 0 );
    if ( index.isValid() ) {
        m_list->selectionModel()->setCurrentIndex( index, QItemSelectionModel::Current );
        m_list->selectionModel()->select( index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
    } else {
        m_list->clearSelection();
    }
}

void ProjectsView::updateProjects()
{
    if ( !isUpdating() ) {
        m_updateCounter = 0;
        m_timer->start();

        UpdateBatch* batch = new UpdateBatch();
        batch->updateUsers();
        batch->updateTypes();
        batch->updateProjects();
        batch->updateStates();

        executeUpdate( batch );
    }
}

void ProjectsView::showMembers()
{
    if ( m_selectedProjectId != 0 && m_currentProjectAdmin ) {
        MembersDialog dialog( m_selectedProjectId, mainWidget() );
        dialog.exec();
    }
}

void ProjectsView::addProject()
{
    if ( m_systemAdmin ) {
        AddProjectDialog dialog( mainWidget() );
        dialog.exec();
    }
}

void ProjectsView::addFolder()
{
    if ( m_currentProjectId != 0 && m_currentProjectAdmin ) {
        AddFolderDialog dialog( m_currentProjectId, mainWidget() );
        if ( dialog.exec() == QDialog::Accepted ) {
            TreeViewHelper helper( m_list );
            m_list->expand( helper.selectedIndex() );
        }
    }
}

void ProjectsView::editRename()
{
    if ( m_selectedProjectId != 0 && m_systemAdmin ) {
        RenameProjectDialog dialog( m_selectedProjectId, mainWidget() );
        dialog.exec();
    } else if ( m_selectedFolderId != 0 && m_currentProjectAdmin ) {
        RenameFolderDialog dialog( m_selectedFolderId, mainWidget() );
        dialog.exec();
    }
}

void ProjectsView::editDelete()
{
    if ( m_selectedProjectId != 0 && m_systemAdmin ) {
        DeleteProjectDialog dialog( m_selectedProjectId, mainWidget() );
        dialog.exec();
    } else if ( m_selectedFolderId != 0 && m_currentProjectAdmin ) {
        DeleteFolderDialog dialog( m_selectedFolderId, mainWidget() );
        dialog.exec();
    }
}

void ProjectsView::moveFolder()
{
    if ( m_selectedFolderId != 0 && m_currentProjectAdmin ) {
        MoveFolderDialog dialog( m_selectedFolderId, mainWidget() );
        dialog.exec();
    }
}

void ProjectsView::openFolder()
{
    if ( m_selectedFolderId != 0  )
        viewManager->openFolderView( m_selectedFolderId );
}

void ProjectsView::manageAlerts()
{
    if ( m_currentFolderId != 0  ) {
        ManageAlertsDialog dialog( m_currentFolderId, mainWidget() );
        dialog.exec();
    }
}

void ProjectsView::updateEvent( UpdateEvent* e )
{
    setAccess( checkDataAccess() );

    if ( e->unit() == UpdateEvent::Projects )
        cascadeUpdateFolders();
}

void ProjectsView::contextMenu( const QPoint& pos )
{
    QModelIndex index = m_list->indexAt( pos );

    if ( index.isValid() ) {
        m_list->selectionModel()->setCurrentIndex( index, QItemSelectionModel::Current );
        m_list->selectionModel()->select( index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
    }

    QString menuName;
    if ( index.isValid() ) {
        int level = m_model->levelOf( index );
        if ( level == 0 )
            menuName = m_systemAdmin ? "menuProjectAdmin" : "menuProject";
        else if ( level == 1 )
            menuName = "menuFolder";
        else
            menuName = "menuAlert";
    } else {
        menuName = "menuNull";
    }

    QMenu* menu = builder()->contextMenu( menuName );
    if ( menu )
        menu->popup( m_list->viewport()->mapToGlobal( pos ) );
}

void ProjectsView::doubleClicked( const QModelIndex& index )
{
    if ( index.isValid() ) {
        int level = m_model->levelOf( index );
        int rowId = m_model->rowId( index );

        if ( level == 1 )
            viewManager->openFolderView( rowId );
    }
}
