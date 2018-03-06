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

#include "userprojectsdialog.h"

#include "application.h"
#include "data/datamanager.h"
#include "data/entities.h"
#include "data/localsettings.h"
#include "dialogs/userdialogs.h"
#include "models/userprojectsmodel.h"
#include "utils/treeviewhelper.h"
#include "utils/iconloader.h"
#include "xmlui/builder.h"
#include "xmlui/toolstrip.h"

#include <QLayout>
#include <QAction>
#include <QTreeView>
#include <QDialogButtonBox>
#include <QMenu>

UserProjectsDialog::UserProjectsDialog( int userId, QWidget* parent ) : InformationDialog( parent ),
    m_userId( userId )
{
    UserEntity user = UserEntity::find( userId );

    QAction* action;

    action = new QAction( IconLoader::icon( "project-new" ), tr( "&Add Projects..." ), this );
    action->setShortcut( QKeySequence::New );
    connect( action, SIGNAL( triggered() ), this, SLOT( addProjects() ) );
    setAction( "addProjects", action );

    action = new QAction( IconLoader::icon( "edit-access" ), tr( "&Change Access..." ), this );
    connect( action, SIGNAL( triggered() ), this, SLOT( changeAccess() ) );
    setAction( "changeAccess", action );

    action = new QAction( IconLoader::icon( "edit-delete" ), tr( "&Remove Projects" ), this );
    action->setShortcut( QKeySequence::Delete );
    connect( action, SIGNAL( triggered() ), this, SLOT( removeProjects() ) );
    setAction( "removeProjects", action );

    setDefaultMenuAction( "menuMember", "changeAccess" );

    loadXmlUiFile( ":/resources/userprojectsdialog.xml" );

    XmlUi::Builder* builder = new XmlUi::Builder( this );
    builder->addClient( this );

    setWindowTitle( tr( "User Projects" ) );
    setPromptPixmap( IconLoader::pixmap( "view-members", 22 ) );
    setPrompt( tr( "Projects of user <b>%1</b>:" ).arg( user.name() ) );

    QVBoxLayout* layout = new QVBoxLayout();
    layout->setSpacing( 4 );

    XmlUi::ToolStrip* strip = new XmlUi::ToolStrip( this );
    builder->registerToolStrip( "stripMemberProjects", strip );
    layout->addWidget( strip );

    m_list = new QTreeView( this );
    layout->addWidget( m_list );

    TreeViewHelper helper( m_list );
    helper.initializeView( TreeViewHelper::MultiSelect );

    m_list->sortByColumn( 0, Qt::AscendingOrder );

    m_model = new UserProjectsModel( userId, this );
    m_list->setModel( m_model );

    helper.loadColumnWidths( "MemberProjectsDialogWidths", QList<int>() << 150 << 150 );

    connect( m_list->selectionModel(), SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
        this, SLOT( updateActions() ) );
    connect( m_list, SIGNAL( doubleClicked( const QModelIndex& ) ),
        this, SLOT( doubleClicked( const QModelIndex& ) ) );
    connect( m_list, SIGNAL( customContextMenuRequested( const QPoint& ) ),
        this, SLOT( listContextMenu( const QPoint& ) ) );

    connect( m_model, SIGNAL( layoutChanged() ), this, SLOT( updateActions() ) );
    connect( m_model, SIGNAL( modelReset() ), this, SLOT( updateActions() ) );

    setContentLayout( layout, false );

    resize( application->applicationSettings()->value( "MemberProjectsDialogSize", QSize( 350, 450 ) ).toSize() );

    updateActions();
}

UserProjectsDialog::~UserProjectsDialog()
{
    application->applicationSettings()->setValue( "MemberProjectsDialogSize", size() );

    TreeViewHelper helper( m_list );
    helper.saveColumnWidths( "MemberProjectsDialogWidths" );
}

void UserProjectsDialog::addProjects()
{
    AddUserProjectsDialog dialog( m_userId, this );
    dialog.exec();
}

void UserProjectsDialog::changeAccess()
{
    ChangeUserProjectsAccessDialog dialog( m_userId, m_selectedProjects, this );
    dialog.exec();
}

void UserProjectsDialog::removeProjects()
{
    RemoveUserProjectsDialog dialog( m_userId, m_selectedProjects, this );
    dialog.exec();
}

void UserProjectsDialog::updateActions()
{
    m_selectedProjects.clear();

    QModelIndexList indexes;
    if ( m_list->selectionModel() )
        indexes = m_list->selectionModel()->selectedRows();

    for ( int i = 0; i < indexes.count(); i++ )
        m_selectedProjects.append( m_model->rowId( indexes.at( i ) ) );

    bool canChange = !m_selectedProjects.isEmpty();

    action( "changeAccess" )->setEnabled( canChange );
    action( "removeProjects" )->setEnabled( canChange );
}

void UserProjectsDialog::doubleClicked( const QModelIndex& index )
{
    if ( index.isValid() ) {
        int projectId = m_model->rowId( index );

        QList<int> projects;
        projects.append( projectId );

        ChangeUserProjectsAccessDialog dialog( m_userId, projects, this );
        dialog.exec();
    }
}

void UserProjectsDialog::listContextMenu( const QPoint& pos )
{
    QModelIndex index = m_list->indexAt( pos );

    if ( index.isValid() ) {
        m_list->selectionModel()->setCurrentIndex( index, QItemSelectionModel::Current );
        m_list->selectionModel()->select( index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
    }

    QString menuName;
    if ( index.isValid() )
        menuName = "menuMemberProject";
    else
        menuName = "menuNull";

    QMenu* menu = builder()->contextMenu( menuName );
    if ( menu )
        menu->popup( m_list->viewport()->mapToGlobal( pos ) );
}
