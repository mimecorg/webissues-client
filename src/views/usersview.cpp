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

#include "usersview.h"

#include "commands/updatebatch.h"
#include "data/datamanager.h"
#include "dialogs/dialogmanager.h"
#include "dialogs/userdialogs.h"
#include "dialogs/userprojectsdialog.h"
#include "dialogs/preferencesdialog.h"
#include "models/usersmodel.h"
#include "utils/treeviewhelper.h"
#include "utils/iconloader.h"
#include "widgets/filterlabel.h"
#include "xmlui/builder.h"

#include <QTreeView>
#include <QLayout>
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

    action = new QAction( IconLoader::icon( "edit-password" ), tr( "Change &Password..." ), this );
    action->setIconText( tr( "Password" ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( changePassword() ), Qt::QueuedConnection );
    setAction( "changePassword", action );

    action = new QAction( IconLoader::icon( "edit-access" ), tr( "&Manage Permissions..." ), this );
    connect( action, SIGNAL( triggered() ), this, SLOT( managePermissions() ), Qt::QueuedConnection );
    setAction( "managePermissions", action );

    action = new QAction( IconLoader::icon( "preferences" ), tr( "User P&references..." ), this );
    connect( action, SIGNAL( triggered() ), this, SLOT( userPreferences() ), Qt::QueuedConnection );
    setAction( "userPreferences", action );

    setTitle( "sectionAdd", tr( "Add" ) );
    setTitle( "sectionUsers", tr( "User Accounts" ) );

    setDefaultMenuAction( "menuUser", "managePermissions" );

    loadXmlUiFile( ":/resources/usersview.xml" );

    QWidget* main = new QWidget( parentWidget );

    QVBoxLayout* mainLayout = new QVBoxLayout( main );
    mainLayout->setMargin( 0 );
    mainLayout->setSpacing( 0 );

    QHBoxLayout* filterLayout = new QHBoxLayout();
    filterLayout->setMargin( 3 );

    mainLayout->addLayout( filterLayout );

    m_filterLabel = new FilterLabel( main );
    filterLayout->addWidget( m_filterLabel );

    m_filterLabel->addItem( tr( "All Users" ) );
    m_filterLabel->addItem( tr( "Active" ) );
    m_filterLabel->addItem( tr( "Disabled" ) );

    connect( m_filterLabel, SIGNAL( currentIndexChanged( int ) ), this, SLOT( filterChanged( int ) ) );

    m_list = new QTreeView( main );
    mainLayout->addWidget( m_list );

    TreeViewHelper helper( m_list );
    helper.initializeView();

    connect( m_list, SIGNAL( customContextMenuRequested( const QPoint& ) ),
        this, SLOT( contextMenu( const QPoint& ) ) );
    connect( m_list, SIGNAL( doubleClicked( const QModelIndex& ) ),
        this, SLOT( doubleClicked( const QModelIndex& ) ) );

    setMainWidget( main );

    setViewerSizeHint( QSize( 400, 400 ) );

    setAccess( NormalAccess );
}

UsersView::~UsersView()
{
    TreeViewHelper helper( m_list );
    helper.saveColumnWidths( "UsersViewWidths" );
}

void UsersView::initialUpdate()
{
    m_list->sortByColumn( 0, Qt::AscendingOrder );

    m_model = new UsersModel( this );
    m_list->setModel( m_model );

    m_model->setFilter( (UsersModel::Filter)m_filterLabel->currentIndex() );

    TreeViewHelper helper( m_list );
    helper.loadColumnWidths( "UsersViewWidths", QList<int>() << 150 << 100 << 150 );

    setCaption( tr( "User Accounts" ) );

    connect( m_list->selectionModel(), SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
        this, SLOT( updateActions() ) );
    connect( m_model, SIGNAL( layoutChanged() ), this, SLOT( updateActions() ) );

    updateActions();
}

void UsersView::updateActions()
{
    m_selectedUserId = 0;

    TreeViewHelper helper( m_list );
    QModelIndex index = helper.selectedIndex();

    if ( index.isValid() )
        m_selectedUserId = m_model->rowId( index );

    action( "editRename" )->setEnabled( m_selectedUserId != 0 );
    action( "changePassword" )->setEnabled( m_selectedUserId != 0 );
    action( "managePermissions" )->setEnabled( m_selectedUserId != 0 );
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

void UsersView::changePassword()
{
    if ( m_selectedUserId != 0 ) {
        SetPasswordDialog dialog( m_selectedUserId, mainWidget() );
        dialog.exec();
    }
}

void UsersView::managePermissions()
{
    if ( m_selectedUserId != 0 ) {
        if ( dialogManager->activateDialog( "UserProjectsDialog", m_selectedUserId ) )
            return;
        UserProjectsDialog* dialog = new UserProjectsDialog( m_selectedUserId );
        dialogManager->addDialog( dialog, m_selectedUserId );
        dialog->show();
    }
}

void UsersView::userPreferences()
{
    if ( m_selectedUserId != 0 ) {
        PreferencesDialog dialog( m_selectedUserId, mainWidget() );
        dialog.exec();
    }
}

void UsersView::filterChanged( int index )
{
    if ( isEnabled() )
        m_model->setFilter( (UsersModel::Filter)index );
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
        int userId = m_model->rowId( index );

        if ( dialogManager->activateDialog( "UserProjectsDialog", userId ) )
            return;
        UserProjectsDialog* dialog = new UserProjectsDialog( userId );
        dialogManager->addDialog( dialog, userId );
        dialog->show();
    }
}
