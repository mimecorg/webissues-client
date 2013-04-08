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

#include "managealertsdialog.h"

#include "data/datamanager.h"
#include "data/entities.h"
#include "dialogs/alertdialogs.h"
#include "models/alertsmodel.h"
#include "utils/treeviewhelper.h"
#include "utils/iconloader.h"
#include "xmlui/builder.h"
#include "xmlui/toolstrip.h"

#include <QLayout>
#include <QAction>
#include <QTreeView>
#include <QDialogButtonBox>
#include <QMenu>

ManageAlertsDialog::ManageAlertsDialog( int folderId ) : InformationDialog( NULL, Qt::Window ),
    m_folderId( folderId )
{
    FolderEntity folder = FolderEntity::find( m_folderId );

    m_emailEnabled = dataManager->setting( "email_enabled" ).toInt();

    QAction* action;

    action = new QAction( IconLoader::icon( "alert-new" ), tr( "&Add Alert..." ), this );
    action->setShortcut( QKeySequence::New );
    connect( action, SIGNAL( triggered() ), this, SLOT( addAlert() ) );
    setAction( "addAlert", action );

    action = new QAction( IconLoader::icon( "edit-delete" ), tr( "&Delete Alert" ), this );
    action->setShortcut( QKeySequence::Delete );
    connect( action, SIGNAL( triggered() ), this, SLOT( editDelete() ) );
    setAction( "editDelete", action );

    if ( m_emailEnabled != 0 ) {
        action = new QAction( IconLoader::icon( "edit-modify" ), tr( "&Modify Alert..." ), this );
        connect( action, SIGNAL( triggered() ), this, SLOT( editModify() ) );
        setAction( "editModify", action );

        setDefaultMenuAction( "menuAlert", "editModify" );
    }

    loadXmlUiFile( ":/resources/managealertsdialog.xml" );

    XmlUi::Builder* builder = new XmlUi::Builder( this );
    builder->addClient( this );

    setWindowTitle( tr( "Manage Alerts" ) );
    setPromptPixmap( IconLoader::pixmap( "configure-alerts", 22 ) );
    setPrompt( tr( "Edit alert settings for folder <b>%1</b>:" ).arg( folder.name() ) );

    QVBoxLayout* layout = new QVBoxLayout();
    layout->setSpacing( 4 );

    XmlUi::ToolStrip* strip = new XmlUi::ToolStrip( this );
    builder->registerToolStrip( "stripAlerts", strip );
    layout->addWidget( strip );

    m_list = new QTreeView( this );
    layout->addWidget( m_list );

    TreeViewHelper helper( m_list );
    helper.initializeView( TreeViewHelper::NotSortable );

    m_model = new AlertsModel( folderId, this );
    m_list->setModel( m_model );

    QList<int> widths;
    widths << 150 << 80 << 80 << 80;
    if ( m_emailEnabled != 0 )
        widths.append( 150 );

    helper.loadColumnWidths( "ManageAlertsDialogWidths", widths );

    connect( m_list->selectionModel(), SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
        this, SLOT( updateActions() ) );
    connect( m_list, SIGNAL( doubleClicked( const QModelIndex& ) ),
        this, SLOT( doubleClicked( const QModelIndex& ) ) );
    connect( m_list, SIGNAL( customContextMenuRequested( const QPoint& ) ),
        this, SLOT( listContextMenu( const QPoint& ) ) );

    connect( m_model, SIGNAL( layoutChanged() ), this, SLOT( updateActions() ) );
    connect( m_model, SIGNAL( modelReset() ), this, SLOT( updateActions() ) );

    setContentLayout( layout, false );

    resize( 600, 400 );

    updateActions();
}

ManageAlertsDialog::~ManageAlertsDialog()
{
    TreeViewHelper helper( m_list );
    helper.saveColumnWidths( "ManageAlertsDialogWidths" );
}

void ManageAlertsDialog::addAlert()
{
    AddAlertDialog dialog( m_folderId, this );
    dialog.exec();
}

void ManageAlertsDialog::editDelete()
{
    if ( m_selectedAlertId != 0 ) {
        DeleteAlertDialog dialog( m_selectedAlertId, this );
        dialog.exec();
    }
}

void ManageAlertsDialog::editModify()
{
    if ( m_selectedAlertId != 0 && m_emailEnabled != 0 ) {
        ModifyAlertDialog dialog( m_selectedAlertId, this );
        dialog.exec();
    }
}

void ManageAlertsDialog::updateActions()
{
    FolderEntity folder = FolderEntity::find( m_folderId );

    action( "addAlert" )->setEnabled( folder.isValid() );

    m_selectedAlertId = 0;

    TreeViewHelper helper( m_list );
    QModelIndex index = helper.selectedIndex();

    if ( index.isValid() )
        m_selectedAlertId = m_model->rowId( index );

    action( "editDelete" )->setEnabled( m_selectedAlertId != 0 );
    if ( m_emailEnabled != 0 )
        action( "editModify" )->setEnabled( m_selectedAlertId != 0 );
}

void ManageAlertsDialog::doubleClicked( const QModelIndex& index )
{
    if ( index.isValid() && m_emailEnabled != 0 ) {
        int alertId = m_model->rowId( index );

        ModifyAlertDialog dialog( alertId, this );
        dialog.exec();
    }
}

void ManageAlertsDialog::listContextMenu( const QPoint& pos )
{
    QModelIndex index = m_list->indexAt( pos );

    if ( index.isValid() ) {
        m_list->selectionModel()->setCurrentIndex( index, QItemSelectionModel::Current );
        m_list->selectionModel()->select( index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
    }

    QString menuName;
    if ( index.isValid() )
        menuName = "menuAlert";
    else
        menuName = "menuNull";

    QMenu* menu = builder()->contextMenu( menuName );
    if ( menu )
        menu->popup( m_list->viewport()->mapToGlobal( pos ) );
}
