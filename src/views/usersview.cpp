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

#include "usersview.h"

#include "commands/updatebatch.h"
#include "data/datamanager.h"
#include "dialogs/userdialogs.h"
#include "dialogs/preferencesdialog.h"
#include "models/tablemodels.h"
#include "utils/treeviewhelper.h"
#include "utils/iconloader.h"
#include "xmlui/builder.h"

#include <QTreeView>
#include <QAction>
#include <QMenu>

UsersView::UsersView( QObject* parent, QWidget* parentWidget ) : View( parent )
{
    QAction* action;

    action = new QAction( IconLoader::icon( "file-reload" ), tr( "&Update Users" ), this );
    action->setShortcut( QKeySequence::Refresh );
    connect( action, SIGNAL( triggered() ), this, SLOT( updateUsers() ), Qt::QueuedConnection );
    setAction( "updateUsers", action );

    action = new QAction( IconLoader::icon( "user-new" ), tr( "&Add User..." ), this );
    action->setShortcut( QKeySequence::New );
    connect( action, SIGNAL( triggered() ), this, SLOT( addUser() ), Qt::QueuedConnection );
    setAction( "addUser", action );

    action = new QAction( IconLoader::icon( "edit-rename" ), tr( "&Rename User..." ), this );
    action->setIconText( tr( "Rename" ) );
    action->setShortcut( tr( "F2" ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( editRename() ), Qt::QueuedConnection );
    setAction( "editRename", action );

    action = new QAction( IconLoader::icon( "edit-access" ), tr( "&Change Access..." ), this );
    action->setIconText( tr( "Access" ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( changeAccess() ), Qt::QueuedConnection );
    setAction( "changeAccess", action );

    action = new QAction( IconLoader::icon( "edit-password" ), tr( "Change &Password..." ), this );
    action->setIconText( tr( "Password" ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( changePassword() ), Qt::QueuedConnection );
    setAction( "changePassword", action );

    action = new QAction( IconLoader::icon( "preferences" ), tr( "User P&references..." ), this );
    action->setIconText( tr( "Preferences" ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( userPreferences() ), Qt::QueuedConnection );
    setAction( "userPreferences", action );

    setTitle( "sectionAdd", tr( "Add" ) );
    setTitle( "sectionUsers", tr( "User Accounts" ) );

    setDefaultMenuAction( "menuUser", "changeAccess" );

    loadXmlUiFile( ":/resources/usersview.xml" );

    m_list = new QTreeView( parentWidget );
    TreeViewHelper::initializeView( m_list );

    connect( m_list, SIGNAL( customContextMenuRequested( const QPoint& ) ),
        this, SLOT( contextMenu( const QPoint& ) ) );
    connect( m_list, SIGNAL( doubleClicked( const QModelIndex& ) ),
        this, SLOT( doubleClicked( const QModelIndex& ) ) );

    setMainWidget( m_list );

    setViewerSizeHint( QSize( 400, 400 ) );

    setAccess( NormalAccess );
}

UsersView::~UsersView()
{
    TreeViewHelper::saveColumnWidths( m_list, "UsersView" );
}

void UsersView::initialUpdate()
{
    m_model = new RDB::TableItemModel( this );
    m_model->setRootTableModel( new UsersTableModel( m_model ), dataManager->users()->index() );

    QList<int> columns;
    columns.append( Column_Name );
    columns.append( Column_Login );
    columns.append( Column_Access );
    m_model->setColumns( columns );

    m_list->setModel( m_model );

    TreeViewHelper::setSortOrder( m_list, qMakePair( (int)Column_Name, Qt::AscendingOrder ) );
    TreeViewHelper::loadColumnWidths( m_list, "UsersView" );

    setCaption( tr( "User Accounts" ) );

    connect( m_list->selectionModel(), SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
        this, SLOT( updateActions() ) );
    connect( m_model, SIGNAL( layoutChanged() ), this, SLOT( updateActions() ) );

    updateActions();
}

void UsersView::updateActions()
{
    m_selectedUserId = 0;

    QModelIndex index = TreeViewHelper::selectedIndex( m_list );
    if ( index.isValid() )
        m_selectedUserId = m_model->data( index, RDB::TableItemModel::RowIdRole ).toInt();

    action( "editRename" )->setEnabled( m_selectedUserId != 0 );
    action( "changeAccess" )->setEnabled( m_selectedUserId != 0 && m_selectedUserId != dataManager->currentUserId() );
    action( "changePassword" )->setEnabled( m_selectedUserId != 0 );
    action( "userPreferences" )->setEnabled( m_selectedUserId != 0 );
}

void UsersView::updateUsers()
{
    if ( !isUpdating() ) {
        UpdateBatch* batch = new UpdateBatch();
        batch->updateUsers();

        executeUpdate( batch );
    }
}

void UsersView::addUser()
{
    AddUserDialog dialog( mainWidget() );
    dialog.exec();
}

void UsersView::editRename()
{
    if ( m_selectedUserId != 0 ) {
        RenameUserDialog dialog( m_selectedUserId, mainWidget() );
        dialog.exec();
    }
}

void UsersView::changeAccess()
{
    if ( m_selectedUserId != 0 && m_selectedUserId != dataManager->currentUserId() ) {
        ChangeUserAccessDialog dialog( m_selectedUserId, mainWidget() );
        dialog.exec();
    }
}

void UsersView::changePassword()
{
    if ( m_selectedUserId != 0 ) {
        SetPasswordDialog dialog( m_selectedUserId, mainWidget() );
        dialog.exec();
    }
}

void UsersView::userPreferences()
{
    if ( m_selectedUserId != 0 ) {
        PreferencesDialog dialog( m_selectedUserId, mainWidget() );
        dialog.exec();
    }
}

void UsersView::contextMenu( const QPoint& pos )
{
    QModelIndex index = m_list->indexAt( pos );

    if ( index.isValid() ) {
        m_list->selectionModel()->setCurrentIndex( index, QItemSelectionModel::Current );
        m_list->selectionModel()->select( index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
    }

    QString menuName;
    if ( index.isValid() )
        menuName = "menuUser";
    else
        menuName = "menuNull";

    QMenu* menu = builder()->contextMenu( menuName );
    if ( menu )
        menu->popup( m_list->viewport()->mapToGlobal( pos ) );
}

void UsersView::doubleClicked( const QModelIndex& index )
{
    if ( index.isValid() && m_systemAdmin ) {
        int userId = m_model->data( index, RDB::TableItemModel::RowIdRole ).toInt();

        if ( userId != dataManager->currentUserId() ) {
            ChangeUserAccessDialog dialog( userId, mainWidget() );
            dialog.exec();
        }
    }
}
