/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2015 WebIssues Team
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

#include "membersdialog.h"

#include "data/datamanager.h"
#include "data/entities.h"
#include "dialogs/projectdialogs.h"
#include "dialogs/userdialogs.h"
#include "models/membersmodel.h"
#include "utils/treeviewhelper.h"
#include "utils/iconloader.h"
#include "xmlui/builder.h"
#include "xmlui/toolstrip.h"

#include <QLayout>
#include <QAction>
#include <QTreeView>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QGroupBox>
#include <QLineEdit>
#include <QMenu>

MembersDialog::MembersDialog( int projectId ) : InformationDialog( NULL, Qt::Window ),
    m_projectId( projectId )
{
    ProjectEntity project = ProjectEntity::find( projectId );

    QAction* action;

    action = new QAction( IconLoader::icon( "user-new" ), tr( "&Add Members..." ), this );
    action->setShortcut( QKeySequence::New );
    connect( action, SIGNAL( triggered() ), this, SLOT( addMember() ) );
    setAction( "addMember", action );

    action = new QAction( IconLoader::icon( "edit-access" ), tr( "&Change Access..." ), this );
    connect( action, SIGNAL( triggered() ), this, SLOT( changeAccess() ) );
    setAction( "changeAccess", action );

    action = new QAction( IconLoader::icon( "edit-delete" ), tr( "&Remove Members" ), this );
    action->setShortcut( QKeySequence::Delete );
    connect( action, SIGNAL( triggered() ), this, SLOT( removeMember() ) );
    setAction( "removeMember", action );

    setDefaultMenuAction( "menuMember", "changeAccess" );

    loadXmlUiFile( ":/resources/membersdialog.xml" );

    XmlUi::Builder* builder = new XmlUi::Builder( this );
    builder->addClient( this );

    setWindowTitle( tr( "Manage Permissions" ) );
    setPromptPixmap( IconLoader::pixmap( "edit-access", 22 ) );
    setPrompt( tr( "Edit permissions of project <b>%1</b>:" ).arg( project.name() ) );

    QVBoxLayout* layout = new QVBoxLayout();

    QGroupBox* globalGroup = new QGroupBox( tr( "Global Access" ), this );
    layout->addWidget( globalGroup );

    QHBoxLayout* globalLayout = new QHBoxLayout( globalGroup );

    m_globalEdit = new QLineEdit( globalGroup );
    m_globalEdit->setReadOnly( true );
    globalLayout->addWidget( m_globalEdit );

    QPushButton* globalButton = new QPushButton( tr( "Change..." ), globalGroup );
    globalButton->setIcon( IconLoader::icon( "edit-modify" ) );
    globalButton->setIconSize( QSize( 16, 16 ) );
    globalLayout->addWidget( globalButton );

    connect( globalButton, SIGNAL( clicked() ), this, SLOT( changeGlobalAccess() ) );

    layout->addSpacing( 5 );

    QVBoxLayout* membersLayout = new QVBoxLayout();
    membersLayout->setSpacing( 4 );
    layout->addLayout( membersLayout );

    XmlUi::ToolStrip* strip = new XmlUi::ToolStrip( this );
    builder->registerToolStrip( "stripMembers", strip );
    membersLayout->addWidget( strip );

    m_list = new QTreeView( this );
    membersLayout->addWidget( m_list );

    TreeViewHelper helper( m_list );
    helper.initializeView( TreeViewHelper::MultiSelect );

    m_list->sortByColumn( 0, Qt::AscendingOrder );

    m_model = new MembersModel( projectId, this );
    m_list->setModel( m_model );

    helper.loadColumnWidths( "MembersDialogWidths", QList<int>() << 150 << 150 );

    connect( m_list->selectionModel(), SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
        this, SLOT( updateActions() ) );
    connect( m_list, SIGNAL( doubleClicked( const QModelIndex& ) ),
        this, SLOT( doubleClicked( const QModelIndex& ) ) );
    connect( m_list, SIGNAL( customContextMenuRequested( const QPoint& ) ),
        this, SLOT( listContextMenu( const QPoint& ) ) );

    connect( m_model, SIGNAL( layoutChanged() ), this, SLOT( updateActions() ) );
    connect( m_model, SIGNAL( modelReset() ), this, SLOT( updateActions() ) );

    setContentLayout( layout, false );

    resize( 350, 450 );

    dataManager->addObserver( this );

    updateGlobalAccess();
    updateActions();
}

MembersDialog::~MembersDialog()
{
    TreeViewHelper helper( m_list );
    helper.saveColumnWidths( "MembersDialogWidths" );

    dataManager->removeObserver( this );
}

void MembersDialog::changeGlobalAccess()
{
    ChangeProjectAccessDialog dialog( m_projectId, this );
    dialog.exec();
}

void MembersDialog::addMember()
{
    AddMemberDialog dialog( m_projectId, this );
    dialog.exec();
}

void MembersDialog::changeAccess()
{
    ChangeMemberAccessDialog dialog( m_selectedUsers, m_projectId, this );
    dialog.exec();
}

void MembersDialog::removeMember()
{
    RemoveMemberDialog dialog( m_selectedUsers, m_projectId, this );
    dialog.exec();
}

void MembersDialog::customEvent( QEvent* e )
{
    if ( e->type() == UpdateEvent::Type ) {
        UpdateEvent* updateEvent = (UpdateEvent*)e;
        if ( updateEvent->unit() == UpdateEvent::Projects )
            updateGlobalAccess();
    }
}

void MembersDialog::updateGlobalAccess()
{
    ProjectEntity project = ProjectEntity::find( m_projectId );

    if ( project.isPublic() )
        m_globalEdit->setText( tr( "Public project" ) );
    else
        m_globalEdit->setText( tr( "Regular project" ) );
}

void MembersDialog::updateActions()
{
    bool isAdmin = ProjectEntity::isAdmin( m_projectId );

    action( "addMember" )->setEnabled( isAdmin );

    m_selectedUsers.clear();

    QModelIndexList indexes;
    if ( m_list->selectionModel() )
        indexes = m_list->selectionModel()->selectedRows();

    for ( int i = 0; i < indexes.count(); i++ )
        m_selectedUsers.append( m_model->rowId( indexes.at( i ) ) );

    bool canChange = !m_selectedUsers.isEmpty();

    if ( dataManager->currentUserAccess() != AdminAccess && m_selectedUsers.contains( dataManager->currentUserId() ) )
        canChange = false;

    action( "changeAccess" )->setEnabled( canChange );
    action( "removeMember" )->setEnabled( canChange );
}

void MembersDialog::doubleClicked( const QModelIndex& index )
{
    if ( index.isValid() ) {
        int userId = m_model->rowId( index );

        if ( dataManager->currentUserAccess() == AdminAccess || userId != dataManager->currentUserId() ) {
            QList<int> users;
            users.append( userId );

            ChangeMemberAccessDialog dialog( users, m_projectId, this );
            dialog.exec();
        }
    }
}

void MembersDialog::listContextMenu( const QPoint& pos )
{
    QModelIndex index = m_list->indexAt( pos );

    if ( index.isValid() ) {
        m_list->selectionModel()->setCurrentIndex( index, QItemSelectionModel::Current );
        m_list->selectionModel()->select( index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
    }

    QString menuName;
    if ( index.isValid() )
        menuName = "menuMember";
    else
        menuName = "menuNull";

    QMenu* menu = builder()->contextMenu( menuName );
    if ( menu )
        menu->popup( m_list->viewport()->mapToGlobal( pos ) );
}
