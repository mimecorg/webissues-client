/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2013 WebIssues Team
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
#include "commands/commandmanager.h"
#include "data/datamanager.h"
#include "data/entities.h"
#include "data/localsettings.h"
#include "data/issuetypecache.h"
#include "dialogs/dialogmanager.h"
#include "dialogs/projectdialogs.h"
#include "dialogs/membersdialog.h"
#include "dialogs/managealertsdialog.h"
#include "models/projectsmodel.h"
#include "utils/treeviewhelper.h"
#include "utils/errorhelper.h"
#include "utils/iconloader.h"
#include "views/viewmanager.h"
#include "xmlui/builder.h"

#include <QTreeView>
#include <QAction>
#include <QMenu>
#include <QTimer>
#include <QMessageBox>

ProjectsView::ProjectsView( QObject* parent, QWidget* parentWidget ) : View( parent ),
    m_folderUpdateCounter( 0 ),
    m_updateCounter( 0 ),
    m_sessionExpired( false )
{
    m_systemAdmin = dataManager->currentUserAccess() == AdminAccess;

    QAction* action;

    action = new QAction( IconLoader::icon( "file-reload" ), tr( "&Update Projects" ), this );
    action->setShortcut( QKeySequence::Refresh );
    connect( action, SIGNAL( triggered() ), this, SLOT( updateProjects() ), Qt::QueuedConnection );
    setAction( "updateProjects", action );

    action = new QAction( IconLoader::icon( "edit-access" ), tr( "&Manage Permissions..." ), this );
    action->setIconText( tr( "Permissions" ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( managePermissions() ), Qt::QueuedConnection );
    setAction( "managePermissions", action );

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

    action = new QAction( IconLoader::icon( "project" ), tr( "&Open Project" ), this );
    action->setShortcut( QKeySequence::Open );
    connect( action, SIGNAL( triggered() ), this, SLOT( openProject() ), Qt::QueuedConnection );
    setAction( "openProject", action );

    action = new QAction( IconLoader::icon( "folder-open" ), tr( "&Open Folder" ), this );
    action->setShortcut( QKeySequence::Open );
    connect( action, SIGNAL( triggered() ), this, SLOT( openFolder() ), Qt::QueuedConnection );
    setAction( "openFolder", action );

    action = new QAction( IconLoader::icon( "folder-open" ), tr( "&Open List" ), this );
    action->setShortcut( QKeySequence::Open );
    connect( action, SIGNAL( triggered() ), this, SLOT( openGlobalList() ), Qt::QueuedConnection );
    setAction( "openGlobalList", action );

    action = new QAction( IconLoader::icon( "configure-alerts" ), tr( "&Manage Alerts..." ), this );
    connect( action, SIGNAL( triggered() ), this, SLOT( manageAlerts() ), Qt::QueuedConnection );
    setAction( "manageAlerts", action );

    setTitle( "sectionAdd", tr( "Add" ) );
    setTitle( "sectionProjects", tr( "Projects" ) );

    setDefaultMenuAction( "menuProject", "openProject" );
    setDefaultMenuAction( "menuProjectAdmin", "openProject" );
    setDefaultMenuAction( "menuFolder", "openFolder" );
    setDefaultMenuAction( "menuGlobalList", "openGlobalList" );

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
    action( "managePermissions" )->setVisible( m_anyProjectAdmin );
    action( "addFolder" )->setVisible( m_anyProjectAdmin );
    action( "editRename" )->setVisible( m_systemAdmin || m_anyProjectAdmin );
    action( "editDelete" )->setVisible( m_systemAdmin || m_anyProjectAdmin );
    action( "moveFolder" )->setVisible( m_systemAdmin || m_anyProjectAdmin );
}

void ProjectsView::updateTimeout()
{
    LocalSettings* settings = application->applicationSettings();
    int folderUpdateInterval = settings->value( "FolderUpdateInterval" ).toInt();
    int updateInterval = settings->value( "UpdateInterval" ).toInt();

    m_folderUpdateCounter++;
    m_updateCounter++;

    if ( m_updateCounter >= updateInterval ) {
        m_folderUpdateCounter = 0;
        m_updateCounter = 0;
        periodicUpdateData( true );
    } else if ( m_folderUpdateCounter >= folderUpdateInterval ) {
        m_folderUpdateCounter = 0;
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
    if ( m_sessionExpired )
        return;

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
        if ( dataManager->folderUpdateNeeded( folder.id() ) ) {
            if ( !batch ) {
                batch = new UpdateBatch( -20 );
                batch->setIfNeeded( true );
            }
            batch->updateFolder( folder.id() );
        }
    }

    if ( batch )
        executeUpdate( batch );
}

void ProjectsView::updateFailed()
{
    if ( commandManager->error() == CommandManager::WebIssuesError && commandManager->errorCode() == ErrorHelper::LoginRequired )
        m_sessionExpired = true;
}

void ProjectsView::updateActions()
{
    m_selectedProjectId = 0;
    m_selectedFolderId = 0;
    m_selectedTypeId = 0;
    m_selectedViewId = 0;

    m_currentProjectId = 0;
    m_currentFolderId = 0;
    m_currentTypeId = 0;

    TreeViewHelper helper( m_list );
    QModelIndex index = helper.selectedIndex();

    if ( index.isValid() ) {
        int level = m_model->levelOf( index );
        int rowId = m_model->rowId( index );
        if ( level == ProjectsModel::Types ) {
            m_selectedTypeId = rowId;
            m_currentTypeId = rowId;
            IssueTypeCache* cache = dataManager->issueTypeCache( rowId );
            m_selectedViewId = cache->initialViewId();
        } if ( level == ProjectsModel::Projects ) {
            m_selectedProjectId = rowId;
            m_currentProjectId = rowId;
        } else if ( level == ProjectsModel::Folders ) {
            m_selectedFolderId = rowId;
            m_currentFolderId = rowId;
            FolderEntity folder = FolderEntity::find( rowId );
            m_currentProjectId = folder.projectId();
            IssueTypeCache* cache = dataManager->issueTypeCache( folder.typeId() );
            m_selectedViewId = cache->initialViewId();
        } else if ( level == ProjectsModel::Alerts ) {
            AlertEntity alert = AlertEntity::find( rowId );
            FolderEntity folder = alert.folder();
            m_selectedViewId = alert.viewId();
            m_currentFolderId = folder.id();
            m_currentProjectId = folder.projectId();
        } else if ( level == ProjectsModel::GlobalAlerts ) {
            AlertEntity alert = AlertEntity::find( rowId );
            TypeEntity type = alert.type();
            m_selectedViewId = alert.viewId();
            m_currentTypeId = type.id();
        }
    }

    m_currentProjectAdmin = ProjectEntity::isAdmin( m_currentProjectId );

    action( "managePermissions" )->setEnabled( m_selectedProjectId && m_currentProjectAdmin );
    action( "openProject" )->setEnabled( m_selectedProjectId != 0 );
    action( "openFolder" )->setEnabled( m_selectedFolderId != 0 );
    action( "openGlobalList" )->setEnabled( m_selectedTypeId != 0 );
    action( "addFolder" )->setEnabled( m_currentProjectId != 0 && m_currentProjectAdmin );
    action( "editRename" )->setEnabled( ( m_selectedProjectId != 0 && m_systemAdmin ) || ( m_selectedFolderId != 0 && m_currentProjectAdmin ) );
    action( "editDelete" )->setEnabled( ( m_selectedProjectId != 0 && m_systemAdmin ) || ( m_selectedFolderId != 0 && m_currentProjectAdmin ) );
    action( "moveFolder" )->setEnabled( m_selectedFolderId != 0 && m_currentProjectAdmin );
    action( "manageAlerts" )->setEnabled( m_currentFolderId != 0 || m_currentTypeId != 0 );

    action( "editRename" )->setText( m_selectedFolderId != 0 ? tr( "&Rename Folder..." ) : tr( "&Rename Project..." ) );
    action( "editDelete" )->setText( m_selectedFolderId != 0 ? tr( "&Delete Folder" ) : tr( "&Delete Project" ) );
}

void ProjectsView::updateSelection()
{
    updateActions();

    if ( m_selectedProjectId != 0 )
        emit projectSelected( m_selectedProjectId );
    else
        emit selectionChanged( m_currentFolderId, m_currentTypeId, m_selectedViewId );
}

void ProjectsView::setSelection( int folderId, int typeId, int viewId )
{
    if ( m_currentFolderId == folderId && m_currentTypeId == typeId && m_selectedViewId == viewId )
        return;

    if ( typeId != 0 ) {
        TypeEntity type = TypeEntity::find( typeId );
        IssueTypeCache* cache = dataManager->issueTypeCache( typeId );
        int initialViewId = cache->initialViewId();

        if ( viewId != initialViewId ) {
            foreach ( const AlertEntity& alert, type.alerts() ) {
                if ( alert.viewId() == viewId ) {
                    QModelIndex index = m_model->findIndex( ProjectsModel::GlobalAlerts, alert.id(), 0 );
                    if ( index.isValid() ) {
                        m_list->selectionModel()->setCurrentIndex( index, QItemSelectionModel::Current );
                        m_list->selectionModel()->select( index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
                        return;
                    }
                    break;
                }
            }
        }

        QModelIndex index = m_model->findIndex( ProjectsModel::Types, typeId, 0 );
        if ( index.isValid() ) {
            m_list->selectionModel()->setCurrentIndex( index, QItemSelectionModel::Current );
            m_list->selectionModel()->select( index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
        } else {
            m_list->clearSelection();
        }
    } else if ( folderId != 0 ) {
        FolderEntity folder = FolderEntity::find( folderId );
        IssueTypeCache* cache = dataManager->issueTypeCache( folder.typeId() );
        int initialViewId = cache->initialViewId();

        if ( viewId != initialViewId ) {
            foreach ( const AlertEntity& alert, folder.alerts() ) {
                if ( alert.viewId() == viewId ) {
                    QModelIndex index = m_model->findIndex( ProjectsModel::Alerts, alert.id(), 0 );
                    if ( index.isValid() ) {
                        m_list->selectionModel()->setCurrentIndex( index, QItemSelectionModel::Current );
                        m_list->selectionModel()->select( index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
                        return;
                    }
                    break;
                }
            }
        }

        QModelIndex index = m_model->findIndex( ProjectsModel::Folders, folderId, 0 );
        if ( index.isValid() ) {
            m_list->selectionModel()->setCurrentIndex( index, QItemSelectionModel::Current );
            m_list->selectionModel()->select( index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
        } else {
            m_list->clearSelection();
        }
    }
}

void ProjectsView::updateProjects()
{
    if ( !isUpdating() ) {
        m_folderUpdateCounter = 0;
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

void ProjectsView::managePermissions()
{
    if ( m_selectedProjectId != 0 && m_currentProjectAdmin ) {
        if ( dialogManager->activateDialog( "MembersDialog", m_selectedProjectId ) )
            return;
        MembersDialog* dialog = new MembersDialog( m_selectedProjectId );
        dialogManager->addDialog( dialog, m_selectedProjectId );
        dialog->show();
    }
}

void ProjectsView::addProject()
{
    if ( m_systemAdmin ) {
        if ( dialogManager->activateDialog( "AddProjectDialog" ) )
            return;
        AddProjectDialog* dialog = new AddProjectDialog();
        dialogManager->addDialog( dialog );
        dialog->show();
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

void ProjectsView::openProject()
{
    if ( m_selectedProjectId != 0  )
        viewManager->openSummaryView( m_selectedProjectId );
}

void ProjectsView::openFolder()
{
    if ( m_selectedFolderId != 0  )
        viewManager->openFolderView( m_selectedFolderId );
}

void ProjectsView::openGlobalList()
{
    if ( m_selectedTypeId != 0  )
        viewManager->openGlobalListView( m_selectedTypeId );
}

void ProjectsView::manageAlerts()
{
    if ( m_currentFolderId != 0  ) {
        if ( dialogManager->activateDialog( "ManageFolderAlertsDialog", m_currentFolderId ) )
            return;
        ManageFolderAlertsDialog* dialog = new ManageFolderAlertsDialog( m_currentFolderId );
        dialogManager->addDialog( dialog, m_currentFolderId );
        dialog->show();
    } else if ( m_currentTypeId != 0 ) {
        if ( dialogManager->activateDialog( "ManageGlobalAlertsDialog", m_currentTypeId ) )
            return;
        ManageGlobalAlertsDialog* dialog = new ManageGlobalAlertsDialog( m_currentTypeId );
        dialogManager->addDialog( dialog, m_currentTypeId );
        dialog->show();
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
        if ( level == ProjectsModel::Projects )
            menuName = m_systemAdmin ? "menuProjectAdmin" : "menuProject";
        else if ( level == ProjectsModel::Types )
            menuName = "menuGlobalList";
        else if ( level == ProjectsModel::Folders )
            menuName = "menuFolder";
        else if ( level == ProjectsModel::Alerts || level == ProjectsModel::GlobalAlerts )
            menuName = "menuAlert";
        else
            menuName = "menuNull";
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

        if ( level == ProjectsModel::Projects )
            viewManager->openSummaryView( rowId );
        else if ( level == ProjectsModel::Types )
            viewManager->openGlobalListView( rowId );
        else if ( level == ProjectsModel::Folders )
            viewManager->openFolderView( rowId );
    }
}
