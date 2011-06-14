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

#include "folderview.h"

#include "commands/updatebatch.h"
#include "data/datamanager.h"
#include "data/entities.h"
#include "data/issuetypecache.h"
#include "data/updateevent.h"
#include "dialogs/issuedialogs.h"
#include "dialogs/reportdialog.h"
#include "dialogs/viewsettingsdialog.h"
#include "dialogs/viewdialogs.h"
#include "dialogs/statedialogs.h"
#include "models/foldermodel.h"
#include "utils/definitioninfo.h"
#include "utils/treeviewhelper.h"
#include "utils/viewsettingshelper.h"
#include "utils/iconloader.h"
#include "views/viewmanager.h"
#include "widgets/searcheditbox.h"
#include "widgets/separatorcombobox.h"
#include "xmlui/builder.h"

#include <QTreeView>
#include <QHeaderView>
#include <QLayout>
#include <QLabel>
#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QKeyEvent>

FolderView::FolderView( QObject* parent, QWidget* parentWidget ) : View( parent ),
    m_model( NULL ),
    m_currentViewId( 0 ),
    m_gotoIssueId( 0 ),
    m_gotoItemId( 0 ),
    m_selectedIssueId( 0 ),
    m_isRead( false )
{
    QAction* action;

    action = new QAction( IconLoader::icon( "file-reload" ), tr( "&Update Folder" ), this );
    action->setShortcut( QKeySequence::Refresh );
    connect( action, SIGNAL( triggered() ), this, SLOT( updateFolder() ), Qt::QueuedConnection );
    setAction( "updateFolder", action );

    action = new QAction( IconLoader::icon( "issue-open" ), tr( "&Open Issue" ), this );
    action->setShortcut( QKeySequence::Open );
    connect( action, SIGNAL( triggered() ), this, SLOT( openIssue() ), Qt::QueuedConnection );
    setAction( "openIssue", action );

    action = new QAction( IconLoader::icon( "issue-new" ), tr( "&Add Issue..." ), this );
    action->setShortcut( QKeySequence::New );
    connect( action, SIGNAL( triggered() ), this, SLOT( addIssue() ), Qt::QueuedConnection );
    setAction( "addIssue", action );

    action = new QAction( IconLoader::icon( "edit-modify" ), tr( "&Edit Attributes..." ), this );
    action->setShortcut( tr( "F2" ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( editIssue() ), Qt::QueuedConnection );
    setAction( "editIssue", action );

    action = new QAction( IconLoader::icon( "issue-move" ), tr( "&Move Issue..." ), this );
    action->setIconText( tr( "Move" ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( moveIssue() ), Qt::QueuedConnection );
    setAction( "moveIssue", action );

    action = new QAction( IconLoader::icon( "edit-delete" ), tr( "&Delete Issue" ), this );
    action->setIconText( tr( "Delete" ) );
    action->setShortcut( QKeySequence::Delete );
    connect( action, SIGNAL( triggered() ), this, SLOT( deleteIssue() ), Qt::QueuedConnection );
    setAction( "deleteIssue", action );

    action = new QAction( IconLoader::icon( "issue" ), tr( "Mark As Read" ), this );
    connect( action, SIGNAL( triggered() ), this, SLOT( markAsRead() ), Qt::QueuedConnection );
    setAction( "markAsRead", action );

    action = new QAction( IconLoader::icon( "folder-read" ), tr( "Mark All As Read" ), this );
    action->setIconText( tr( "Mark All As Read", "icon text" ) );
    setAction( "popupMarkAll", action );

    action = new QAction( IconLoader::icon( "folder-read" ), tr( "Mark All As Read" ), this );
    connect( action, SIGNAL( triggered() ), this, SLOT( markAllAsRead() ), Qt::QueuedConnection );
    setAction( "markAllAsRead", action );

    action = new QAction( IconLoader::icon( "folder-unread" ), tr( "Mark All As Unread" ), this );
    connect( action, SIGNAL( triggered() ), this, SLOT( markAllAsUnread() ), Qt::QueuedConnection );
    setAction( "markAllAsUnread", action );

    action = new QAction( IconLoader::icon( "file-print" ), tr( "Print List" ), this );
    action->setIconText( tr( "Print" ) );
    action->setShortcut( QKeySequence::Print );
    connect( action, SIGNAL( triggered() ), this, SLOT( printReport() ), Qt::QueuedConnection );
    setAction( "printReport", action );

    action = new QAction( IconLoader::icon( "export-pdf" ), tr( "Export List" ), this );
    action->setIconText( tr( "Export" ) );
    setAction( "popupExport", action );

    action = new QAction( IconLoader::icon( "export-csv" ), tr( "Export To CSV" ), this );
    connect( action, SIGNAL( triggered() ), this, SLOT( exportCsv() ), Qt::QueuedConnection );
    setAction( "exportCsv", action );

    action = new QAction( IconLoader::icon( "export-html" ), tr( "Export To HTML" ), this );
    connect( action, SIGNAL( triggered() ), this, SLOT( exportHtml() ), Qt::QueuedConnection );
    setAction( "exportHtml", action );

    action = new QAction( IconLoader::icon( "export-pdf" ), tr( "Export To PDF" ), this );
    connect( action, SIGNAL( triggered() ), this, SLOT( exportPdf() ), Qt::QueuedConnection );
    setAction( "exportPdf", action );

    action = new QAction( IconLoader::icon( "configure-views" ), tr( "&Manage Views..." ), this );
    connect( action, SIGNAL( triggered() ), this, SLOT( manageViews() ), Qt::QueuedConnection );
    setAction( "manageViews", action );

    action = new QAction( IconLoader::icon( "view-new" ), tr( "&Add View" ), this );
    action->setIconText( tr( "Add" ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( addView() ), Qt::QueuedConnection );
    setAction( "addView", action );

    action = new QAction( IconLoader::icon( "edit-modify" ), tr( "M&odify View" ), this );
    action->setIconText( tr( "Modify" ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( modifyView() ), Qt::QueuedConnection );
    setAction( "modifyView", action );

    setTitle( "sectionAdd", tr( "Add" ) );
    setTitle( "sectionFolder", tr( "Folder" ) );
    setTitle( "sectionIssue", tr( "Issue" ) );
    setTitle( "sectionViews", tr( "Views" ) );
    setTitle( "sectionReport", tr( "Report" ) );

    setPopupMenu( "popupExport", "menuExport", "exportPdf" );
    setPopupMenu( "popupMarkAll", "menuMarkAll", "markAllAsRead" );

    setDefaultMenuAction( "menuIssue", "openIssue" );

    loadXmlUiFile( ":/resources/folderview.xml" );

    QWidget* main = new QWidget( parentWidget );

    QVBoxLayout* mainLayout = new QVBoxLayout( main );
    mainLayout->setMargin( 0 );
    mainLayout->setSpacing( 0 );

    QHBoxLayout* viewLayout = new QHBoxLayout();
    viewLayout->setMargin( 3 );
    viewLayout->setSpacing( 5 );

    mainLayout->addLayout( viewLayout );

    QLabel* viewLabel = new QLabel( tr( "&Select view:" ), main );
    viewLayout->addWidget( viewLabel );

    m_viewCombo = new SeparatorComboBox( main );
    m_viewCombo->setMaxVisibleItems( 15 );
    m_viewCombo->setMaximumWidth( 200 );
    m_viewCombo->setMinimumWidth( 100 );

    connect( m_viewCombo, SIGNAL( activated( int ) ), this, SLOT( viewActivated( int ) ) );

    viewLayout->addWidget( m_viewCombo, 1 );

    viewLabel->setBuddy( m_viewCombo );

    viewLayout->addStretch( 0 );

    QLabel* searchLabel = new QLabel( tr( "&Quick search:" ), main );
    viewLayout->addWidget( searchLabel );

    m_searchBox = new SearchEditBox( main );
    m_searchBox->setMaximumWidth( 200 );
    m_searchBox->setMinimumWidth( 100 );

    connect( m_searchBox, SIGNAL( textChanged( const QString& ) ), this, SLOT( quickSearchChanged( const QString& ) ) );

    viewLayout->addWidget( m_searchBox, 1 );

    searchLabel->setBuddy( m_searchBox );

    m_list = new QTreeView( main );
    mainLayout->addWidget( m_list );

    TreeViewHelper helper( m_list );
    helper.initializeView();

    connect( m_list, SIGNAL( customContextMenuRequested( const QPoint& ) ),
        this, SLOT( listContextMenu( const QPoint& ) ) );
    connect( m_list->header(), SIGNAL( customContextMenuRequested( const QPoint& ) ),
        this, SLOT( headerContextMenu( const QPoint& ) ) );
    connect( m_list, SIGNAL( doubleClicked( const QModelIndex& ) ),
        this, SLOT( doubleClicked( const QModelIndex& ) ) );

    connect( m_searchBox, SIGNAL( deactivate() ), m_list, SLOT( setFocus() ) );

    main->installEventFilter( this );
    m_list->installEventFilter( this );
    m_list->viewport()->installEventFilter( this );

    setMainWidget( main );

    setViewerSizeHint( QSize( 700, 500 ) );

    m_list->setFocus();
}

FolderView::~FolderView()
{
    cleanUp();
}

void FolderView::cleanUp()
{
    if ( isEnabled() ) {
        TreeViewHelper helper( m_list );
        helper.saveColumnWidths( "FolderViewWidths", m_model->columns() );
    }

    m_searchBox->clear();
    m_currentViewId = 0;

    delete m_model;
    m_model = NULL;
}

void FolderView::initialUpdate()
{
    cleanUp();

    m_model = new FolderModel( id(), this );

    connect( m_model, SIGNAL( layoutChanged() ), this, SLOT( updateActions() ) );
    connect( m_model, SIGNAL( layoutChanged() ), this, SLOT( updateSummary() ) );
    connect( m_model, SIGNAL( modelReset() ), this, SLOT( updateActions() ) );
    connect( m_model, SIGNAL( modelReset() ), this, SLOT( updateSummary() ) );

    setAccess( checkDataAccess(), true );

    initialUpdateFolder();
}

Access FolderView::checkDataAccess()
{
    FolderEntity folder = FolderEntity::find( id() );
    if ( !folder.isValid() )
        return NoAccess;

    m_projectId = folder.projectId();
    m_typeId = folder.typeId();

    if ( dataManager->currentUserAccess() != AdminAccess ) {
        MemberEntity member = MemberEntity::find( m_projectId, dataManager->currentUserId() );
        if ( !member.isValid() )
            return NoAccess;
    }

    TypeEntity type = folder.type();
    if ( !type.isValid() )
        return UnknownAccess;

    if ( FolderEntity::isAdmin( id() ) )
        return AdminAccess;

    return NormalAccess;
}

void FolderView::enableView()
{
    m_list->setModel( m_model );

    connect( m_list->selectionModel(), SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
        this, SLOT( updateActions() ) );

    updateActions();
    updateCaption();

    updateViews();
    loadCurrentView( true );
}

void FolderView::disableView()
{
    m_list->setModel( NULL );

    disconnect( m_list->selectionModel(), SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
        this, SLOT( updateActions() ) );

    updateCaption();
}

void FolderView::updateAccess( Access access )
{
    action( "moveIssue" )->setVisible( access == AdminAccess );
    action( "deleteIssue" )->setVisible( access == AdminAccess );
}

void FolderView::updateCaption()
{
    QString name = tr( "Unknown Folder" );
    if ( isEnabled() ) {
    FolderEntity folder = FolderEntity::find( id() );
        if ( folder.isValid() )
            name = folder.name();
    }
    setCaption( name );
}

void FolderView::updateActions()
{
    m_selectedIssueId = 0;
    m_isRead = true;

    TreeViewHelper helper( m_list );
    QModelIndex index = helper.selectedIndex();

    if ( index.isValid() ) {
        m_selectedIssueId = m_model->rowId( index );
        if ( m_selectedIssueId ) {
            IssueEntity issue = IssueEntity::find( m_selectedIssueId );
            m_isRead = issue.isValid() ? issue.stampId() == issue.readId() : false;
        }
    }

    action( "editIssue" )->setEnabled( m_selectedIssueId != 0 );
    action( "openIssue" )->setEnabled( m_selectedIssueId != 0 );

    action( "moveIssue" )->setEnabled( m_selectedIssueId != 0 && access() == AdminAccess );
    action( "deleteIssue" )->setEnabled( m_selectedIssueId != 0 && access() == AdminAccess );

    action( "markAsRead" )->setEnabled( m_selectedIssueId != 0 );
    action( "markAsRead" )->setText( m_isRead ? tr( "Mark As Unread" ) : tr( "Mark As Read" ) );
    action( "markAsRead" )->setIcon( IconLoader::icon( m_isRead ? "issue-unread" : "issue" ) );

    bool isPersonalView = false;

    if ( m_currentViewId != 0 ) {
        ViewEntity view = ViewEntity::find( m_currentViewId );
        if ( view.isValid() )
            isPersonalView = !view.isPublic();
    }

    action( "modifyView" )->setEnabled( isPersonalView );

    emit selectedIssueChanged( m_selectedIssueId );
}

void FolderView::updateSummary()
{
    int items = m_model->rowCount();
    showSummary( QPixmap(), tr( "%1 issues" ).arg( items ) );
}

void FolderView::initialUpdateFolder()
{
    if ( isEnabled() && dataManager->issueUpdateNeeded( id() ) ) {
        UpdateBatch* batch = new UpdateBatch();
        batch->setIfNeeded( true );
        batch->updateFolder( id() );

        executeUpdate( batch );
    }
}

void FolderView::updateFolder()
{
    if ( isEnabled() && !isUpdating() ) {
        UpdateBatch* batch = new UpdateBatch();
        batch->updateFolder( id() );

        executeUpdate( batch );
    }
}

void FolderView::cascadeUpdateFolder()
{
    if ( isEnabled() && !isUpdating() && dataManager->folderUpdateNeeded( id() ) ) {
        UpdateBatch* batch = new UpdateBatch( -10 );
        batch->setIfNeeded( true );
        batch->updateFolder( id() );

        executeUpdate( batch );
    }
}

void FolderView::openIssue()
{
    if ( isEnabled() && m_selectedIssueId != 0 )
        viewManager->openIssueView( m_selectedIssueId );
}

void FolderView::addIssue()
{
    if ( isEnabled() ) {
        AddIssueDialog dialog( id(), mainWidget() );
        if ( dialog.exec() == QDialog::Accepted ) {
            int issueId = dialog.issueId();

            setSelectedIssueId( issueId );

            if ( viewManager->isStandAlone( this ) )
                viewManager->openIssueView( issueId );
        }
    }
}

void FolderView::editIssue()
{
    if ( isEnabled() && m_selectedIssueId != 0 ) {
        EditIssueDialog dialog( m_selectedIssueId, mainWidget() );
        dialog.exec();
    }
}

void FolderView::moveIssue()
{
    if ( isEnabled() && FolderEntity::isAdmin( id() ) ) {
        MoveIssueDialog dialog( m_selectedIssueId, mainWidget() );
        dialog.exec();
    }
}

void FolderView::deleteIssue()
{
    if ( isEnabled() && FolderEntity::isAdmin( id() ) ) {
        DeleteIssueDialog dialog( m_selectedIssueId, mainWidget() );
        dialog.exec();
    }
}

void FolderView::markAsRead()
{
    if ( isEnabled() && m_selectedIssueId != 0 ) {
        IssueStateDialog dialog( m_selectedIssueId, !m_isRead, mainWidget() );
        dialog.accept();
        dialog.exec();
    }
}

void FolderView::markAllAsRead()
{
    if ( isEnabled() ) {
        FolderStateDialog dialog( id(), true, mainWidget() );
        dialog.exec();
    }
}

void FolderView::markAllAsUnread()
{
    if ( isEnabled() ) {
        FolderStateDialog dialog( id(), false, mainWidget() );
        dialog.exec();
    }
}

void FolderView::printReport()
{
    IssueTypeCache* cache = dataManager->issueTypeCache( m_typeId );

    ReportDialog dialog( ReportDialog::FolderSource, ReportDialog::Print, mainWidget() );
    dialog.setFolder( id(), visibleIssues() );
    dialog.setColumns( m_model->columns(), cache->availableColumns() );
    dialog.exec();
}

void FolderView::exportCsv()
{
    IssueTypeCache* cache = dataManager->issueTypeCache( m_typeId );

    ReportDialog dialog( ReportDialog::FolderSource, ReportDialog::ExportCsv, mainWidget() );
    dialog.setFolder( id(), visibleIssues() );
    dialog.setColumns( m_model->columns(), cache->availableColumns() );
    dialog.exec();
}

void FolderView::exportHtml()
{
    IssueTypeCache* cache = dataManager->issueTypeCache( m_typeId );

    ReportDialog dialog( ReportDialog::FolderSource, ReportDialog::ExportHtml, mainWidget() );
    dialog.setFolder( id(), visibleIssues() );
    dialog.setColumns( m_model->columns(), cache->availableColumns() );
    dialog.exec();
}

void FolderView::exportPdf()
{
    IssueTypeCache* cache = dataManager->issueTypeCache( m_typeId );

    ReportDialog dialog( ReportDialog::FolderSource, ReportDialog::ExportPdf, mainWidget() );
    dialog.setFolder( id(), visibleIssues() );
    dialog.setColumns( m_model->columns(), cache->availableColumns() );
    dialog.exec();
}

void FolderView::manageViews()
{
    bool isPublic = false;
    for ( ; ; ) {
        ViewSettingsDialog dialog( m_typeId, isPublic, mainWidget() );
        if ( dialog.exec() == ViewSettingsDialog::SwitchMode )
            isPublic = !isPublic;
        else
            break;
    }
}

void FolderView::addView()
{
    AddViewDialog dialog( m_typeId, false, mainWidget() );
    if ( dialog.exec() == QDialog::Accepted ) {
        m_currentViewId = dialog.viewId();
        updateViews();
        loadCurrentView( true );
    }
}

void FolderView::modifyView()
{
    if ( m_currentViewId != 0 ) {
        ModifyViewDialog dialog( m_currentViewId, mainWidget() );
        dialog.exec();
    }
}

void FolderView::updateEvent( UpdateEvent* e )
{
    setAccess( checkDataAccess() );

    if ( isEnabled() ) {
        if ( e->unit() == UpdateEvent::Projects )
            updateCaption();

        if ( e->unit() == UpdateEvent::Types ) {
            updateViews();
            loadCurrentView( false );
        }

        if ( e->unit() == UpdateEvent::States )
            updateActions();
    }

    if ( isEnabled() && m_gotoIssueId != 0 && e->unit() == UpdateEvent::Folder && e->id() == id() ) {
        gotoIssue( m_gotoIssueId, m_gotoItemId );
        m_gotoIssueId = 0;
        m_gotoItemId = 0;
    }

    if ( id() != 0 && e->unit() == UpdateEvent::Projects )
        cascadeUpdateFolder();
}

void FolderView::headerContextMenu( const QPoint& pos )
{
    QMenu* menu = builder()->contextMenu( "menuHeader" );
    if ( menu )
        menu->popup( m_list->header()->mapToGlobal( pos ) );
}

void FolderView::listContextMenu( const QPoint& pos )
{
    QModelIndex index = m_list->indexAt( pos );

    if ( index.isValid() ) {
        m_list->selectionModel()->setCurrentIndex( index, QItemSelectionModel::Current );
        m_list->selectionModel()->select( index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
    }

    QString menuName;
    if ( index.isValid() )
        menuName = "menuIssue";
    else
        menuName = "menuNull";

    QMenu* menu = builder()->contextMenu( menuName );
    if ( menu )
        menu->popup( m_list->viewport()->mapToGlobal( pos ) );
}

void FolderView::doubleClicked( const QModelIndex& index )
{
    if ( index.isValid() ) {
        int issueId = m_model->rowId( index );
        viewManager->openIssueView( issueId );
    }
}

void FolderView::setSelectedIssueId( int issueId )
{
    if ( m_selectedIssueId == issueId )
        return;

    QModelIndex index = m_model->findIndex( 0, issueId, 0 );

    if ( index.isValid() ) {
        m_list->selectionModel()->setCurrentIndex( index, QItemSelectionModel::Current );
        m_list->selectionModel()->select( index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
    }
}

void FolderView::gotoIssue( int issueId, int itemId )
{
    if ( issueId != m_selectedIssueId ) {
        if ( isUpdating() ) {
            m_gotoIssueId = issueId;
            m_gotoItemId = itemId;
        }

        setSelectedIssueId( issueId );
    }

    if ( issueId == m_selectedIssueId )
        emit gotoItem( itemId );
}

void FolderView::quickSearchChanged( const QString& text )
{
    m_model->setSearchText( text );
}

bool FolderView::eventFilter( QObject* obj, QEvent* e )
{
    if ( obj == mainWidget() ) {
        if ( e->type() == QEvent::ShortcutOverride ) {
            QKeyEvent* ke = (QKeyEvent*)e;
            if ( ke->key() == Qt::Key_F3 && ( ke->modifiers() & ~Qt::ShiftModifier ) == 0 ) {
                if ( isEnabled() ) {
                    m_searchBox->setFocus();
                    ke->accept();
                    return true;
                }
            }
        }
    }

    return View::eventFilter( obj, e );
}

void FolderView::updateViews()
{
    m_viewCombo->clear();

    m_viewCombo->addItem( tr( "All Issues" ), 0 );

    TypeEntity type = TypeEntity::find( m_typeId );

    QList<ViewEntity> personalViews;
    QList<ViewEntity> publicViews;

    foreach ( const ViewEntity& view, type.views() ) {
        if ( view.isPublic() )
            publicViews.append( view );
        else
            personalViews.append( view );
    }

    if ( !personalViews.isEmpty() ) {
        m_viewCombo->addSeparator();
        m_viewCombo->addParentItem( tr( "Personal Views" ) );
        foreach ( const ViewEntity& view, personalViews ) {
            m_viewCombo->addChildItem( view.name(), view.id() );
            if ( view.id() == m_currentViewId )
                m_viewCombo->setCurrentIndex( m_viewCombo->count() - 1 );
        }
    }

    if ( !publicViews.isEmpty() ) {
        m_viewCombo->addSeparator();
        m_viewCombo->addParentItem( tr( "Public Views" ) );
        foreach ( const ViewEntity& view, publicViews ) {
            m_viewCombo->addChildItem( view.name(), view.id() );
            if ( view.id() == m_currentViewId )
                m_viewCombo->setCurrentIndex( m_viewCombo->count() - 1 );
        }
    }

    if ( m_viewCombo->currentIndex() == 0 )
        m_currentViewId = 0;

    updateActions();
}

void FolderView::viewActivated( int index )
{
    m_currentViewId = m_viewCombo->itemData( index ).toInt();

    loadCurrentView( true );

    updateActions();
}

void FolderView::setCurrentViewId( int viewId )
{
    if ( m_currentViewId == viewId )
        return;

    for ( int i = 0; i < m_viewCombo->count(); i++ ) {
        if ( m_viewCombo->itemData( i ).toInt() == viewId ) {
            m_viewCombo->setCurrentIndex( i );
            viewActivated( i );
            break;
        }
    }
}

void FolderView::loadCurrentView( bool resort )
{
    TreeViewHelper helper( m_list );
    helper.saveColumnWidths( "FolderViewWidths", m_model->columns() );

    m_model->setView( m_currentViewId, resort );

    if ( resort )
        m_list->sortByColumn( m_model->sortColumn(), m_model->sortOrder() );

    IssueTypeCache* cache = dataManager->issueTypeCache( m_typeId );

    helper.loadColumnWidths( "FolderViewWidths", m_model->columns(), cache->defaultWidths() );

    if ( resort )
        emit currentViewChanged( m_currentViewId );
}

QList<int> FolderView::visibleIssues()
{
    QList<int> list;

    int rows = m_model->rowCount();
    for ( int i = 0; i < rows; i++ ) {
        int issueId = m_model->rowId( m_model->index( i, 0 ) );
        list.append( issueId );
    }

    return list;
}
