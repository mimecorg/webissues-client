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

#include "typesview.h"

#include "commands/updatebatch.h"
#include "data/datamanager.h"
#include "data/entities.h"
#include "dialogs/dialogmanager.h"
#include "dialogs/typedialogs.h"
#include "dialogs/viewsettingsdialog.h"
#include "models/typesmodel.h"
#include "utils/treeviewhelper.h"
#include "utils/iconloader.h"
#include "xmlui/builder.h"

#include <QTreeView>
#include <QAction>
#include <QMenu>
#include <QMessageBox>

TypesView::TypesView( QObject* parent, QWidget* parentWidget ) : View( parent )
{
    QAction* action;

    action = new QAction( IconLoader::icon( "file-reload" ), tr( "&Update Types" ), this );
    action->setShortcut( QKeySequence::Refresh );
    connect( action, SIGNAL( triggered() ), this, SLOT( updateTypes() ), Qt::QueuedConnection );
    setAction( "updateTypes", action );

    action = new QAction( IconLoader::icon( "type-new" ), tr( "Add &Type..." ), this );
    connect( action, SIGNAL( triggered() ), this, SLOT( addType() ), Qt::QueuedConnection );
    setAction( "addType", action );

    action = new QAction( IconLoader::icon( "attribute-new" ), tr( "Add &Attribute..." ), this );
    action->setShortcut( QKeySequence::New );
    connect( action, SIGNAL( triggered() ), this, SLOT( addAttribute() ), Qt::QueuedConnection );
    setAction( "addAttribute", action );

    action = new QAction( IconLoader::icon( "edit-rename" ), tr( "&Rename Attribute..." ), this );
    action->setIconText( tr( "Rename" ) );
    action->setShortcut( tr( "F2" ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( editRename() ), Qt::QueuedConnection );
    setAction( "editRename", action );

    action = new QAction( IconLoader::icon( "edit-modify" ), tr( "&Modify Attribute..." ), this );
    connect( action, SIGNAL( triggered() ), this, SLOT( editModify() ), Qt::QueuedConnection );
    setAction( "editModify", action );

    action = new QAction( IconLoader::icon( "edit-delete" ), tr( "&Delete Attribute" ), this );
    action->setIconText( tr( "Delete" ) );
    action->setShortcut( QKeySequence::Delete );
    connect( action, SIGNAL( triggered() ), this, SLOT( editDelete() ), Qt::QueuedConnection );
    setAction( "editDelete", action );

    action = new QAction( IconLoader::icon( "configure-views" ), tr( "&View Settings..." ), this );
    connect( action, SIGNAL( triggered() ), this, SLOT( viewSettings() ), Qt::QueuedConnection );
    setAction( "viewSettings", action );

    setTitle( "sectionAdd", tr( "Add" ) );
    setTitle( "sectionTypes", tr( "Issue Types" ) );

    setDefaultMenuAction( "menuAttribute", "editModify" );

    loadXmlUiFile( ":/resources/typesview.xml" );

    m_list = new QTreeView( parentWidget );

    TreeViewHelper helper( m_list );
    helper.initializeView( TreeViewHelper::TreeStyle | TreeViewHelper::NotSortable );

    connect( m_list, SIGNAL( customContextMenuRequested( const QPoint& ) ),
        this, SLOT( contextMenu( const QPoint& ) ) );
    connect( m_list, SIGNAL( doubleClicked( const QModelIndex& ) ),
        this, SLOT( doubleClicked( const QModelIndex& ) ) );

    setMainWidget( m_list );

    setViewerSizeHint( QSize( 700, 500 ) );

    setAccess( NormalAccess );
}

TypesView::~TypesView()
{
    TreeViewHelper helper( m_list );
    helper.saveColumnWidths( "TypesViewWidths" );
    helper.saveExpandedNodes( "ExpandedTypes" );
}

void TypesView::initialUpdate()
{
    m_model = new TypesModel( this );
    m_list->setModel( m_model );

    TreeViewHelper helper( m_list );
    helper.loadColumnWidths( "TypesViewWidths", QList<int>() << 150 << 150 << 150 << 100 << 300 );
    helper.loadExpandedNodes( "ExpandedTypes" );

    setCaption( tr( "Issue Types" ) );

    connect( m_list->selectionModel(), SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
        this, SLOT( updateActions() ) );
    connect( m_model, SIGNAL( layoutChanged() ), this, SLOT( updateActions() ) );

    updateActions();
}

void TypesView::updateActions()
{
    m_selectedTypeId = 0;
    m_selectedAttributeId = 0;
    m_currentTypeId = 0;

    TreeViewHelper helper( m_list );
    QModelIndex index = helper.selectedIndex();

    if ( index.isValid() ) {
        int level = m_model->levelOf( index );
        int rowId = m_model->rowId( index );
        if ( level == 0 ) {
            m_selectedTypeId = rowId;
            m_currentTypeId = rowId;
        } else {
            m_selectedAttributeId = rowId;
            AttributeEntity attribute = AttributeEntity::find( rowId );
            m_currentTypeId = attribute.typeId();
        }
    }

    action( "addAttribute" )->setEnabled( m_currentTypeId != 0 );
    action( "editRename" )->setEnabled( m_selectedTypeId != 0 || m_selectedAttributeId != 0 );
    action( "editModify" )->setEnabled( m_selectedAttributeId != 0 );
    action( "editDelete" )->setEnabled( m_selectedTypeId != 0 || m_selectedAttributeId != 0 );
    action( "viewSettings" )->setEnabled( m_selectedTypeId != 0 );

    action( "editRename" )->setText( m_selectedAttributeId != 0 ? tr( "&Rename Attribute..." ) : tr( "&Rename Type..." ) );
    action( "editDelete" )->setText( m_selectedAttributeId != 0 ? tr( "&Delete Attribute" ) : tr( "&Delete Type" ) );
}

void TypesView::updateTypes()
{
    if ( !isUpdating() ) {
        UpdateBatch* batch = new UpdateBatch();
        batch->updateTypes();

        executeUpdate( batch );
    }
}

void TypesView::addType()
{
    AddTypeDialog dialog( mainWidget() );
    dialog.exec();
}

void TypesView::addAttribute()
{
    if ( m_currentTypeId != 0 ) {
        AddAttributeDialog dialog( m_currentTypeId, mainWidget() );
        if ( dialog.exec() == QDialog::Accepted ) {
            TreeViewHelper helper( m_list );
            m_list->expand( helper.selectedIndex() );
        }
    }
}

void TypesView::editRename()
{
    if ( m_selectedTypeId != 0 ) {
        RenameTypeDialog dialog( m_selectedTypeId, mainWidget() );
        dialog.exec();
    } else if ( m_selectedAttributeId != 0 ) {
        RenameAttributeDialog dialog( m_selectedAttributeId, mainWidget() );
        dialog.exec();
    }
}

void TypesView::editModify()
{
    if ( m_selectedAttributeId != 0 ) {
        ModifyAttributeDialog dialog( m_selectedAttributeId, mainWidget() );
        dialog.exec();
    }
}

void TypesView::editDelete()
{
    if ( m_selectedTypeId != 0 ) {
        DeleteTypeDialog dialog( m_selectedTypeId, mainWidget() );
        dialog.exec();
    } else if ( m_selectedAttributeId != 0 ) {
        DeleteAttributeDialog dialog( m_selectedAttributeId, mainWidget() );
        dialog.exec();
    }
}

void TypesView::viewSettings()
{
    if ( m_selectedTypeId != 0 ) {
        if ( dialogManager->activateDialog( "PublicViewSettingsDialog", m_selectedTypeId ) )
            return;
        PublicViewSettingsDialog* dialog = new PublicViewSettingsDialog( m_selectedTypeId );
        dialogManager->addDialog( dialog, m_selectedTypeId );
        dialog->show();
    }
}

void TypesView::contextMenu( const QPoint& pos )
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
            menuName = "menuType";
        else
            menuName = "menuAttribute";
    } else {
        menuName = "menuNull";
    }

    QMenu* menu = builder()->contextMenu( menuName );
    if ( menu )
        menu->popup( m_list->viewport()->mapToGlobal( pos ) );
}

void TypesView::doubleClicked( const QModelIndex& index )
{
    if ( index.isValid() ) {
        int level = m_model->levelOf( index );
        int rowId = m_model->rowId( index );

        if ( level == 1 ) {
            ModifyAttributeDialog dialog( rowId, mainWidget() );
            dialog.exec();
        }
    }
}
