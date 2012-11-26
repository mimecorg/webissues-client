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

#include "viewsettingsdialog.h"

#include "application.h"
#include "data/datamanager.h"
#include "data/entities.h"
#include "data/issuetypecache.h"
#include "data/localsettings.h"
#include "dialogs/viewdialogs.h"
#include "models/viewsmodel.h"
#include "utils/treeviewhelper.h"
#include "utils/viewsettingshelper.h"
#include "utils/iconloader.h"
#include "xmlui/builder.h"
#include "xmlui/toolstrip.h"

#include <QLayout>
#include <QLabel>
#include <QGroupBox>
#include <QAction>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QLineEdit>
#include <QTreeView>
#include <QMenu>

ViewSettingsDialog::ViewSettingsDialog( int typeId, bool isPublic, QWidget* parent ) : InformationDialog( parent ),
    m_typeId( typeId ),
    m_isPublic( isPublic ),
    m_selectedViewId( 0 ),
    m_orderEdit( NULL ),
    m_columnsEdit( NULL ),
    m_sortByEdit( NULL ),
    m_initialEdit( NULL )
{
    QAction* action;

    if ( isPublic )
        action = new QAction( IconLoader::icon( "view-new" ), tr( "&Add Public View..." ), this );
    else
        action = new QAction( IconLoader::icon( "view-new" ), tr( "&Add Personal View..." ), this );
    action->setShortcut( QKeySequence::New );
    connect( action, SIGNAL( triggered() ), this, SLOT( addView() ) );
    setAction( "addView", action );

    action = new QAction( IconLoader::icon( "view-clone" ), tr( "&Clone View..." ), this );
    connect( action, SIGNAL( triggered() ), this, SLOT( cloneView() ) );
    setAction( "cloneView", action );

    action = new QAction( IconLoader::icon( "edit-rename" ), tr( "&Rename View..." ), this );
    action->setShortcut( tr( "F2" ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( editRename() ) );
    setAction( "editRename", action );

    action = new QAction( IconLoader::icon( "edit-delete" ), tr( "&Delete View" ), this );
    action->setShortcut( QKeySequence::Delete );
    connect( action, SIGNAL( triggered() ), this, SLOT( editDelete() ) );
    setAction( "editDelete", action );

    action = new QAction( IconLoader::icon( "edit-modify" ), tr( "&Modify View..." ), this );
    connect( action, SIGNAL( triggered() ), this, SLOT( editModify() ) );
    setAction( "editModify", action );

    if ( dataManager->currentUserAccess() == AdminAccess ) {
        if ( isPublic )
            action = new QAction( IconLoader::icon( "edit-unpublish" ), tr( "&Unpublish View" ), this );
        else
            action = new QAction( IconLoader::icon( "edit-publish" ), tr( "&Publish View" ), this );
        connect( action, SIGNAL( triggered() ), this, SLOT( editPublish() ) );
        setAction( "editPublish", action );
    }

    setDefaultMenuAction( "menuView", "editModify" );

    loadXmlUiFile( ":/resources/viewsettingsdialog.xml" );

    XmlUi::Builder* builder = new XmlUi::Builder( this );
    builder->addClient( this );

    TypeEntity type = TypeEntity::find( typeId );

    setPromptPixmap( IconLoader::pixmap( "configure-views", 22 ) );

    if ( isPublic ) {
        setWindowTitle( tr( "View Settings" ) );
        setPrompt( tr( "Edit public view settings for type <b>%1</b>:" ).arg( type.name() ) );
    } else {
        setWindowTitle( tr( "Manage Views" ) );
        setPrompt( tr( "Edit personal view settings for type <b>%1</b>:" ).arg( type.name() ) );
    }

    QVBoxLayout* layout = new QVBoxLayout();

    if ( isPublic ) {
        if ( !type.attributes().isEmpty() ) {
            QGroupBox* orderGroup = new QGroupBox( tr( "Order of Attributes" ), this );
            layout->addWidget( orderGroup );

            QHBoxLayout* orderLayout = new QHBoxLayout( orderGroup );

            m_orderEdit = new QLineEdit( orderGroup );
            m_orderEdit->setReadOnly( true );
            orderLayout->addWidget( m_orderEdit );

            QPushButton* orderButton = new QPushButton( tr( "Modify..." ), orderGroup );
            orderButton->setIcon( IconLoader::icon( "edit-modify" ) );
            orderButton->setIconSize( QSize( 16, 16 ) );
            orderLayout->addWidget( orderButton );

            connect( orderButton, SIGNAL( clicked() ), this, SLOT( modifyOrder() ) );
        }

        QGroupBox* defaultGroup = new QGroupBox( tr( "Default View" ), this );
        layout->addWidget( defaultGroup );

        QGridLayout* defaultLayout = new QGridLayout( defaultGroup );

        QLabel* columnsLabel = new QLabel( tr( "Columns:" ), defaultGroup );
        defaultLayout->addWidget( columnsLabel, 0, 0 );

        m_columnsEdit = new QLineEdit( defaultGroup );
        m_columnsEdit->setReadOnly( true );
        defaultLayout->addWidget( m_columnsEdit, 0, 1 );

        QLabel* sortByLabel = new QLabel( tr( "Sort By:" ), defaultGroup );
        defaultLayout->addWidget( sortByLabel, 1, 0 );

        m_sortByEdit = new QLineEdit( defaultGroup );
        m_sortByEdit->setReadOnly( true );
        defaultLayout->addWidget( m_sortByEdit, 1, 1 );

        QPushButton* defaultButton = new QPushButton( tr( "Modify..." ), defaultGroup );
        defaultButton->setIcon( IconLoader::icon( "edit-modify" ) );
        defaultButton->setIconSize( QSize( 16, 16 ) );
        defaultLayout->addWidget( defaultButton, 0, 2 );

        connect( defaultButton, SIGNAL( clicked() ), this, SLOT( modifyDefaultView() ) );

        if ( dataManager->checkServerVersion( "1.0.4" ) ) {
            QGroupBox* initialGroup = new QGroupBox( tr( "Initial View" ), this );
            layout->addWidget( initialGroup );

            QHBoxLayout* initialLayout = new QHBoxLayout( initialGroup );

            m_initialEdit = new QLineEdit( initialGroup );
            m_initialEdit->setReadOnly( true );
            initialLayout->addWidget( m_initialEdit );

            QPushButton* initialButton = new QPushButton( tr( "Modify..." ), initialGroup );
            initialButton->setIcon( IconLoader::icon( "edit-modify" ) );
            initialButton->setIconSize( QSize( 16, 16 ) );
            initialLayout->addWidget( initialButton );

            connect( initialButton, SIGNAL( clicked() ), this, SLOT( modifyInitialView() ) );
        }

        layout->addSpacing( 5 );
    }

    QVBoxLayout* viewsLayout = new QVBoxLayout();
    viewsLayout->setSpacing( 4 );
    layout->addLayout( viewsLayout );

    XmlUi::ToolStrip* strip = new XmlUi::ToolStrip( this );
    builder->registerToolStrip( "stripViews", strip );
    viewsLayout->addWidget( strip );

    m_list = new QTreeView( this );
    viewsLayout->addWidget( m_list );

    TreeViewHelper helper( m_list );
    helper.initializeView( TreeViewHelper::NotSortable );

    m_model = new ViewsModel( typeId, isPublic, this );
    m_list->setModel( m_model );

    helper.loadColumnWidths( "ViewSettingsDialogWidths", QList<int>() << 150 << 300 << 150 << 300 );

    connect( m_list->selectionModel(), SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
        this, SLOT( updateActions() ) );
    connect( m_list, SIGNAL( doubleClicked( const QModelIndex& ) ),
        this, SLOT( doubleClicked( const QModelIndex& ) ) );
    connect( m_list, SIGNAL( customContextMenuRequested( const QPoint& ) ),
        this, SLOT( listContextMenu( const QPoint& ) ) );

    connect( m_model, SIGNAL( layoutChanged() ), this, SLOT( updateActions() ) );
    connect( m_model, SIGNAL( modelReset() ), this, SLOT( updateActions() ) );

    if ( isPublic || dataManager->currentUserAccess() == AdminAccess ) {
        QPushButton* switchButton = new QPushButton( isPublic ? tr( "&Manage Personal Views..." ) : tr( "&Public View Settings..." ), this );
        switchButton->setIcon( IconLoader::icon( "configure-views" ) );
        switchButton->setIconSize( QSize( 16, 16 ) );
        buttonBox()->addButton( switchButton, QDialogButtonBox::ResetRole );

        connect( switchButton, SIGNAL( clicked() ), this, SLOT( switchMode() ) );
    }

    setContentLayout( layout, false );

    if ( isPublic )
        resize( application->applicationSettings()->value( "ViewSettingsDialogSizePublic", QSize( 800, 550 ) ).toSize() );
    else
        resize( application->applicationSettings()->value( "ViewSettingsDialogSizePersonal", QSize( 800, 350 ) ).toSize() );

    dataManager->addObserver( this );

    updateViewSettings();
    updateActions();
}

ViewSettingsDialog::~ViewSettingsDialog()
{
    if ( m_isPublic )
        application->applicationSettings()->setValue( "ViewSettingsDialogSizePublic", size() );
    else
        application->applicationSettings()->setValue( "ViewSettingsDialogSizePersonal", size() );

    TreeViewHelper helper( m_list );
    helper.saveColumnWidths( "ViewSettingsDialogWidths" );

    dataManager->removeObserver( this );
}

void ViewSettingsDialog::modifyOrder()
{
    AttributeOrderDialog dialog( m_typeId, this );
    dialog.exec();
}

void ViewSettingsDialog::modifyDefaultView()
{
    DefaultViewDialog dialog( m_typeId, this );
    dialog.exec();
}

void ViewSettingsDialog::modifyInitialView()
{
    InitialViewDialog dialog( m_typeId, this );
    dialog.exec();
}

void ViewSettingsDialog::addView()
{
    AddViewDialog dialog( m_typeId, m_isPublic, this );
    dialog.exec();
}

void ViewSettingsDialog::cloneView()
{
    CloneViewDialog dialog( m_selectedViewId, m_isPublic, this );
    dialog.exec();
}

void ViewSettingsDialog::editRename()
{
    if ( m_selectedViewId != 0 ) {
        RenameViewDialog dialog( m_selectedViewId, this );
        dialog.exec();
    }
}

void ViewSettingsDialog::editDelete()
{
    if ( m_selectedViewId != 0 ) {
        DeleteViewDialog dialog( m_selectedViewId, this );
        dialog.exec();
    }
}

void ViewSettingsDialog::editModify()
{
    if ( m_selectedViewId != 0 ) {
        ModifyViewDialog dialog( m_selectedViewId, this );
        dialog.exec();
    }
}

void ViewSettingsDialog::editPublish()
{
    if ( m_selectedViewId != 0 ) {
        PublishViewDialog dialog( m_selectedViewId, !m_isPublic, this );
        dialog.exec();
    }
}

void ViewSettingsDialog::switchMode()
{
    done( SwitchMode );
}

void ViewSettingsDialog::customEvent( QEvent* e )
{
    if ( e->type() == UpdateEvent::Type ) {
        UpdateEvent* updateEvent = (UpdateEvent*)e;
        if ( updateEvent->unit() == UpdateEvent::Types )
            updateViewSettings();
    }
}

void ViewSettingsDialog::updateViewSettings()
{
    IssueTypeCache* cache = dataManager->issueTypeCache( m_typeId );
    ViewSettingsHelper helper( m_typeId );

    if ( m_orderEdit )
        m_orderEdit->setText( helper.attributeNames( cache->attributes() ) );

    if ( m_isPublic ) {
        m_columnsEdit->setText( helper.columnNames( cache->viewColumns( cache->defaultView() ) ) );
        m_sortByEdit->setText( helper.sortOrderInfo( cache->viewSortOrder( cache->defaultView() ) ) );
        if ( m_initialEdit )
            m_initialEdit->setText( helper.viewName( cache->initialViewId() ) );
    }
}

void ViewSettingsDialog::updateActions()
{
    TypeEntity type = TypeEntity::find( m_typeId );

    action( "addView" )->setEnabled( type.isValid() );

    m_selectedViewId = 0;

    TreeViewHelper helper( m_list );
    QModelIndex index = helper.selectedIndex();

    if ( index.isValid() )
        m_selectedViewId = m_model->rowId( index );

    action( "cloneView" )->setEnabled( m_selectedViewId != 0 );
    action( "editRename" )->setEnabled( m_selectedViewId != 0 );
    action( "editDelete" )->setEnabled( m_selectedViewId != 0 );
    action( "editModify" )->setEnabled( m_selectedViewId != 0 );
    if ( dataManager->currentUserAccess() == AdminAccess )
        action( "editPublish" )->setEnabled( m_selectedViewId != 0 );
}

void ViewSettingsDialog::doubleClicked( const QModelIndex& index )
{
    if ( index.isValid() ) {
        int viewId = m_model->rowId( index );
        ModifyViewDialog dialog( viewId, this );
        dialog.exec();
    }
}

void ViewSettingsDialog::listContextMenu( const QPoint& pos )
{
    QModelIndex index = m_list->indexAt( pos );

    if ( index.isValid() ) {
        m_list->selectionModel()->setCurrentIndex( index, QItemSelectionModel::Current );
        m_list->selectionModel()->select( index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
    }

    QString menuName;
    if ( index.isValid() )
        menuName = "menuView";
    else
        menuName = "menuNull";

    QMenu* menu = builder()->contextMenu( menuName );
    if ( menu )
        menu->popup( m_list->viewport()->mapToGlobal( pos ) );
}
