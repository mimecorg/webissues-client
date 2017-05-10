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
    m_model( NULL ),
    m_selectedAlertId( 0 ),
    m_selectedEditable( false )
{
    m_emailEnabled = dataManager->setting( "email_enabled" ).toInt();

    if ( folderId != 0 )
        m_canEditPublic = FolderEntity::isAdmin( folderId );
    else
        m_canEditPublic = dataManager->currentUserAccess() == AdminAccess;

    QAction* action;

    action = new QAction( IconLoader::icon( "alert-new" ), tr( "&Add Alert..." ), this );
    action->setShortcut( QKeySequence::New );
    connect( action, SIGNAL( triggered() ), this, SLOT( addAlert() ) );
    setAction( "addAlert", action );

    if ( m_canEditPublic ) {
        action = new QAction( IconLoader::overlayedIcon( "alert-new", "overlay-public" ), tr( "Add &Public Alert..." ), this );
        connect( action, SIGNAL( triggered() ), this, SLOT( addPublicAlert() ) );
        setAction( "addPublicAlert", action );
    }

    action = new QAction( IconLoader::icon( "edit-delete" ), tr( "&Delete Alert" ), this );
    action->setShortcut( QKeySequence::Delete );
    connect( action, SIGNAL( triggered() ), this, SLOT( editDelete() ) );
    setAction( "editDelete", action );

    if ( m_emailEnabled ) {
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

    QVBoxLayout* layout = new QVBoxLayout();
    layout->setSpacing( 4 );

    XmlUi::ToolStrip* strip = new XmlUi::ToolStrip( this );
    builder->registerToolStrip( "stripAlerts", strip );
    layout->addWidget( strip );

    m_list = new QTreeView( this );
    layout->addWidget( m_list );

    TreeViewHelper helper( m_list );
    helper.initializeView( TreeViewHelper::NotSortable );

    connect( m_list, SIGNAL( doubleClicked( const QModelIndex& ) ),
        this, SLOT( doubleClicked( const QModelIndex& ) ) );
    connect( m_list, SIGNAL( customContextMenuRequested( const QPoint& ) ),
        this, SLOT( listContextMenu( const QPoint& ) ) );

    setContentLayout( layout, false );

    setDialogSizeKey( "ManageAlertsDialog" );

    resize( 680, 400 );

    updateActions();
}

ManageAlertsDialog::~ManageAlertsDialog()
{
    if ( m_model != NULL ) {
        TreeViewHelper helper( m_list );
        helper.saveColumnWidths( "ManageAlertsDialogWidths" );
    }
}

void ManageAlertsDialog::initializeList( AlertsModel* model )
{
    m_model = model;

    m_list->setModel( m_model );

    TreeViewHelper helper( m_list );

    QList<int> widths;
    widths << 150 << 80 << 80 << 80;
    if ( m_emailEnabled )
        widths.append( 150 );
    widths << 80;

    helper.loadColumnWidths( "ManageAlertsDialogWidths", widths );

    connect( m_model, SIGNAL( layoutChanged() ), this, SLOT( updateActions() ) );
    connect( m_model, SIGNAL( modelReset() ), this, SLOT( updateActions() ) );

    connect( m_list->selectionModel(), SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
        this, SLOT( updateActions() ) );
}

void ManageAlertsDialog::editDelete()
{
    if ( m_selectedEditable ) {
        DeleteAlertDialog dialog( m_selectedAlertId, this );
        dialog.exec();
    }
}

void ManageAlertsDialog::editModify()
{
    if ( m_selectedEditable && m_emailEnabled ) {
        ModifyAlertDialog dialog( m_selectedAlertId, this );
        dialog.exec();
    }
}

void ManageAlertsDialog::updateActions()
{
    m_selectedAlertId = 0;
    m_selectedEditable = false;

    TreeViewHelper helper( m_list );
    QModelIndex index = helper.selectedIndex();

    if ( index.isValid() ) {
        m_selectedAlertId = m_model->rowId( index );
        if ( m_canEditPublic ) {
            m_selectedEditable = true;
        } else {
            AlertEntity alert = AlertEntity::find( m_selectedAlertId );
            m_selectedEditable = !alert.isPublic();
        }
    }

    action( "editDelete" )->setEnabled( m_selectedEditable );
    if ( m_emailEnabled )
        action( "editModify" )->setEnabled( m_selectedEditable );
}

void ManageAlertsDialog::doubleClicked( const QModelIndex& index )
{
    if ( index.isValid() && m_selectedEditable && m_emailEnabled ) {
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

ManageFolderAlertsDialog::ManageFolderAlertsDialog( int folderId ) : ManageAlertsDialog( folderId ),
    m_folderId( folderId )
{
    FolderEntity folder = FolderEntity::find( folderId );
    setPrompt( tr( "Edit alert settings for folder <b>%1</b>:" ).arg( folder.name() ) );

    AlertsModel* model = new AlertsModel( this );
    model->initializeFolder( folderId );

    initializeList( model );
}

ManageFolderAlertsDialog::~ManageFolderAlertsDialog()
{
}

void ManageFolderAlertsDialog::addAlert()
{
    AddAlertDialog dialog( m_folderId, false, this );
    dialog.exec();
}

void ManageFolderAlertsDialog::addPublicAlert()
{
    AddAlertDialog dialog( m_folderId, true, this );
    dialog.exec();
}

void ManageFolderAlertsDialog::updateActions()
{
    FolderEntity folder = FolderEntity::find( m_folderId );
    action( "addAlert" )->setEnabled( folder.isValid() );

    ManageAlertsDialog::updateActions();
}

ManageGlobalAlertsDialog::ManageGlobalAlertsDialog( int typeId ) : ManageAlertsDialog( 0 ),
    m_typeId( typeId )
{
    TypeEntity type = TypeEntity::find( typeId );
    setPrompt( tr( "Edit alert settings for type <b>%1</b>:" ).arg( type.name() ) );

    AlertsModel* model = new AlertsModel( this );
    model->initializeGlobalList( typeId );

    initializeList( model );
}

ManageGlobalAlertsDialog::~ManageGlobalAlertsDialog()
{
}

void ManageGlobalAlertsDialog::addAlert()
{
    AddGlobalAlertDialog dialog( m_typeId, false, this );
    dialog.exec();
}

void ManageGlobalAlertsDialog::addPublicAlert()
{
    AddGlobalAlertDialog dialog( m_typeId, true, this );
    dialog.exec();
}

void ManageGlobalAlertsDialog::updateActions()
{
    TypeEntity type = TypeEntity::find( m_typeId );
    action( "addAlert" )->setEnabled( type.isValid() );

    ManageAlertsDialog::updateActions();
}
