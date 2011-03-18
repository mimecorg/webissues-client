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

#include "typesview.h"

#include "commands/updatebatch.h"
#include "data/datamanager.h"
#include "dialogs/typedialogs.h"
#include "dialogs/viewsettingsdialog.h"
#include "models/tablemodels.h"
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
    TreeViewHelper::initializeView( m_list, TreeViewHelper::TreeStyle );

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
    TreeViewHelper::saveColumnWidths( m_list, "TypesView" );
    TreeViewHelper::saveExpandedNodes( m_list, "TypesView" );
}

void TypesView::initialUpdate()
{
    m_model = new RDB::TableItemModel( this );
    m_model->setRootTableModel( new TypesTableModel( m_model ), dataManager->types()->index() );
    m_model->addChildTableModel( new AttributesTableModel( m_model ),
        dataManager->attributes()->index(), dataManager->attributes()->parentIndex() );

    QList<int> columns;
    columns.append( Column_Name );
    columns.append( Column_Type );
    columns.append( Column_DefaultValue );
    columns.append( Column_Required );
    columns.append( Column_Details );
    m_model->setColumns( columns );

    m_list->setModel( m_model );

    TreeViewHelper::setSortOrder( m_list, qMakePair( (int)Column_Name, Qt::AscendingOrder ) );
    TreeViewHelper::loadColumnWidths( m_list, "TypesView" );
    TreeViewHelper::loadExpandedNodes( m_list, "TypesView" );

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

    QModelIndex index = TreeViewHelper::selectedIndex( m_list );
    if ( index.isValid() ) {
        int level = m_model->data( index, RDB::TableItemModel::LevelRole ).toInt();
        int rowId = m_model->data( index, RDB::TableItemModel::RowIdRole ).toInt();
        if ( level == 0 ) {
            m_selectedTypeId = rowId;
            m_currentTypeId = rowId;
        } else {
            m_selectedAttributeId = rowId;
            const AttributeRow* attribute = dataManager->attributes()->find( rowId );
            if ( attribute )
                m_currentTypeId = attribute->typeId();
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
        if ( dialog.exec() == QDialog::Accepted )
            m_list->expand( TreeViewHelper::selectedIndex( m_list ) );
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
        bool isPublic = true;
        for ( ; ; ) {
            ViewSettingsDialog dialog( m_selectedTypeId, isPublic, mainWidget() );
            if ( dialog.exec() == ViewSettingsDialog::SwitchMode )
                isPublic = !isPublic;
            else
                break;
        }
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
        int level = m_model->data( index, RDB::TableItemModel::LevelRole ).toInt();
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
        int level = m_model->data( index, RDB::TableItemModel::LevelRole ).toInt();
        int rowId = m_model->data( index, RDB::TableItemModel::RowIdRole ).toInt();

        if ( level == 1 ) {
            ModifyAttributeDialog dialog( rowId, mainWidget() );
            dialog.exec();
        }
    }
}
