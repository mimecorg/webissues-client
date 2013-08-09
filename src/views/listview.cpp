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

#include "listview.h"

#include "commands/updatebatch.h"
#include "data/datamanager.h"
#include "data/entities.h"
#include "data/issuetypecache.h"
#include "data/updateevent.h"
#include "dialogs/dialogmanager.h"
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
#include <QKeyEvent>

ListView::ListView( QObject* parent, QWidget* parentWidget ) : View( parent ),
    m_model( NULL ),
    m_selectedIssueId( 0 ),
    m_isRead( false ),
    m_isSubscribed( false ),
    m_isAdmin( false ),
    m_hasIssues( false ),
    m_currentViewId( 0 ),
    m_searchColumn( Column_Name )
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

    action = new QAction( IconLoader::icon( "issue-clone" ), tr( "Clone Issue..." ), this );
    connect( action, SIGNAL( triggered() ), this, SLOT( cloneIssue() ), Qt::QueuedConnection );
    setAction( "cloneIssue", action );

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

    action = new QAction( IconLoader::icon( "issue-subscribe" ), tr( "Subscribe" ), this );
    connect( action, SIGNAL( triggered() ), this, SLOT( subscribe() ), Qt::QueuedConnection );
    setAction( "subscribe", action );

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

    action = new QAction( IconLoader::icon( "view-new" ), tr( "Add View" ), this );
    setAction( "popupAddView", action );

    action = new QAction( IconLoader::icon( "view-new" ), tr( "&Add View" ), this );
    connect( action, SIGNAL( triggered() ), this, SLOT( addView() ), Qt::QueuedConnection );
    setAction( "addView", action );

    action = new QAction( IconLoader::icon( "view-clone" ), tr( "&Clone View" ), this );
    connect( action, SIGNAL( triggered() ), this, SLOT( cloneView() ), Qt::QueuedConnection );
    setAction( "cloneView", action );

    action = new QAction( IconLoader::icon( "edit-modify" ), tr( "M&odify View" ), this );
    connect( action, SIGNAL( triggered() ), this, SLOT( modifyView() ), Qt::QueuedConnection );
    setAction( "modifyView", action );

    setTitle( "sectionAdd", tr( "Add" ) );
    setTitle( "sectionFolder", tr( "Folder" ) );
    setTitle( "sectionIssue", tr( "Issue" ) );
    setTitle( "sectionViews", tr( "Views" ) );
    setTitle( "sectionReport", tr( "Report" ) );

    setPopupMenu( "popupExport", "menuExport", "exportPdf" );
    setPopupMenu( "popupMarkAll", "menuMarkAll", "markAllAsRead" );
    setPopupMenu( "popupAddView", "menuAddView", "addView" );

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

    QLabel* viewLabel = new QLabel( tr( "&View:" ), main );
    viewLayout->addWidget( viewLabel );

    m_viewCombo = new SeparatorComboBox( main );
    m_viewCombo->setMaxVisibleItems( 15 );
    m_viewCombo->setMaximumWidth( 200 );
    m_viewCombo->setMinimumWidth( 100 );

    connect( m_viewCombo, SIGNAL( activated( int ) ), this, SLOT( viewActivated( int ) ) );

    viewLayout->addWidget( m_viewCombo, 1 );

    viewLabel->setBuddy( m_viewCombo );

    viewLayout->addStretch( 0 );

    QLabel* searchLabel = new QLabel( tr( "&Search:" ), main );
    viewLayout->addWidget( searchLabel );

    m_searchBox = new SearchEditBox( main );
    m_searchBox->setMaximumWidth( 200 );
    m_searchBox->setMinimumWidth( 100 );

    connect( m_searchBox, SIGNAL( textChanged( const QString& ) ), this, SLOT( quickSearchChanged( const QString& ) ) );

    m_searchMenu = new QMenu( m_searchBox );
    m_searchBox->setOptionsMenu( m_searchMenu );

    m_searchActionGroup = new QActionGroup( this );

    connect( m_searchActionGroup, SIGNAL( triggered( QAction* ) ), this, SLOT( searchActionTriggered( QAction* ) ) );

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

ListView::~ListView()
{
    cleanUp();
}

void ListView::cleanUp()
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

void ListView::initialUpdate()
{
    cleanUp();

    m_model = new FolderModel( this );

    connect( m_model, SIGNAL( layoutChanged() ), this, SLOT( updateActions() ) );
    connect( m_model, SIGNAL( layoutChanged() ), this, SLOT( updateSummary() ) );
    connect( m_model, SIGNAL( modelReset() ), this, SLOT( updateActions() ) );
    connect( m_model, SIGNAL( modelReset() ), this, SLOT( updateSummary() ) );

    initializeList();
}

void ListView::enableView()
{
    m_list->setModel( m_model );

    connect( m_list->selectionModel(), SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
        this, SLOT( updateActions() ) );

    updateActions();
    updateCaption();

    updateViews();
    loadCurrentView( true );

    updateSearchOptions();
}

void ListView::disableView()
{
    m_list->setModel( NULL );

    disconnect( m_list->selectionModel(), SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
        this, SLOT( updateActions() ) );

    updateCaption();
}

void ListView::updateAccess( Access access )
{
    bool emailEnabled = dataManager->setting( "email_enabled" ).toInt();

    action( "moveIssue" )->setVisible( access == AdminAccess );
    action( "deleteIssue" )->setVisible( access == AdminAccess );
    action( "subscribe" )->setVisible( emailEnabled );
}

void ListView::updateActions()
{
    m_selectedIssueId = 0;
    m_isRead = true;
    m_isSubscribed = false;
    m_isAdmin = false;
    m_hasIssues = false;

    TreeViewHelper helper( m_list );
    QModelIndex index = helper.selectedIndex();

    if ( index.isValid() )
        m_selectedIssueId = m_model->rowId( index );

    updateSelection();

    action( "editIssue" )->setEnabled( m_selectedIssueId != 0 );
    action( "openIssue" )->setEnabled( m_selectedIssueId != 0 );
    action( "cloneIssue" )->setEnabled( m_selectedIssueId != 0 );

    action( "moveIssue" )->setEnabled( m_isAdmin );
    action( "deleteIssue" )->setEnabled( m_isAdmin );

    action( "markAsRead" )->setEnabled( m_selectedIssueId != 0 );
    action( "markAsRead" )->setText( m_isRead ? tr( "Mark As Unread" ) : tr( "Mark As Read" ) );
    action( "markAsRead" )->setIcon( IconLoader::icon( m_isRead ? "issue-unread" : "issue" ) );

    action( "popupMarkAll" )->setEnabled( m_hasIssues );
    action( "markAllAsRead" )->setEnabled( m_hasIssues );
    action( "markAllAsUnread" )->setEnabled( m_hasIssues );

    action( "subscribe" )->setEnabled( m_selectedIssueId != 0 );
    action( "subscribe" )->setText( m_isSubscribed ? tr( "Unsubscribe" ) : tr( "Subscribe" ) );
    action( "subscribe" )->setIcon( IconLoader::icon( m_isSubscribed ? "issue-unsubscribe" : "issue-subscribe" ) );

    action( "cloneView" )->setEnabled( m_currentViewId != 0 );

    bool isPersonalView = false;

    if ( m_currentViewId != 0 ) {
        ViewEntity view = ViewEntity::find( m_currentViewId );
        if ( view.isValid() )
            isPersonalView = !view.isPublic();
    }

    action( "modifyView" )->setEnabled( isPersonalView );

    emit selectedIssueChanged( m_selectedIssueId );
}

void ListView::updateSummary()
{
    int items = m_model->rowCount();
    showSummary( QPixmap(), tr( "%1 issues" ).arg( items ) );
}

void ListView::openIssue()
{
    if ( isEnabled() && m_selectedIssueId != 0 )
        viewManager->openIssueView( m_selectedIssueId );
}

void ListView::editIssue()
{
    if ( isEnabled() && m_selectedIssueId != 0 ) {
        if ( dialogManager->activateDialog( "EditIssueDialog", m_selectedIssueId ) )
            return;
        EditIssueDialog* dialog = new EditIssueDialog( m_selectedIssueId );
        dialogManager->addDialog( dialog, m_selectedIssueId );
        if ( viewManager->isStandAlone( this ) )
            dialog->setUpdateFolder( true );
        dialog->show();
    }
}

void ListView::cloneIssue()
{
    if ( isEnabled() ) {
        if ( dialogManager->activateDialog( "CloneIssueDialog", m_selectedIssueId ) )
            return;

        CloneIssueDialog* dialog = new CloneIssueDialog( m_selectedIssueId );
        dialogManager->addDialog( dialog, m_selectedIssueId );

        connect( dialog, SIGNAL( issueAdded( int, int ) ), this, SLOT( issueAdded( int, int ) ) );

        dialog->show();
    }
}

void ListView::moveIssue()
{
    if ( isEnabled() && m_isAdmin ) {
        MoveIssueDialog dialog( m_selectedIssueId, mainWidget() );
        dialog.setUpdateFolder( true );
        dialog.exec();
    }
}

void ListView::deleteIssue()
{
    if ( isEnabled() && m_isAdmin ) {
        DeleteIssueDialog dialog( m_selectedIssueId, mainWidget() );
        dialog.exec();
    }
}

void ListView::markAsRead()
{
    if ( isEnabled() && m_selectedIssueId != 0 ) {
        int readId;
        if ( m_isRead ) {
            readId = 0;
        } else {
            IssueEntity issue = IssueEntity::find( m_selectedIssueId );
            readId = issue.stampId();
        }

        IssueStateDialog dialog( m_selectedIssueId, readId, mainWidget() );
        dialog.accept();
        dialog.exec();
    }
}

void ListView::subscribe()
{
    if ( isEnabled() && m_selectedIssueId != 0 ) {
        if ( m_isSubscribed ) {
            DeleteSubscriptionDialog dialog( m_selectedIssueId, mainWidget() );
            dialog.exec();
        } else {
            AddSubscriptionDialog dialog( m_selectedIssueId, mainWidget() );
            dialog.exec();
        }
    }
}

void ListView::printReport()
{
    executeReport( ReportDialog::Print );
}

void ListView::exportCsv()
{
    executeReport( ReportDialog::ExportCsv );
}

void ListView::exportHtml()
{
    executeReport( ReportDialog::ExportHtml );
}

void ListView::exportPdf()
{
    executeReport( ReportDialog::ExportPdf );
}

void ListView::executeReport( int mode )
{
    IssueTypeCache* cache = dataManager->issueTypeCache( m_typeId );

    ReportDialog dialog( ReportDialog::FolderSource, (ReportDialog::ReportMode)mode, mainWidget() );

    initializeReport( &dialog );
    dialog.setIssues( visibleIssues() );
    dialog.setColumns( visibleColumns(), cache->availableColumns() );

    dialog.exec();
}

void ListView::manageViews()
{
    if ( dialogManager->activateDialog( "PersonalViewSettingsDialog", m_typeId ) )
        return;
    PersonalViewSettingsDialog* dialog = new PersonalViewSettingsDialog( m_typeId );
    dialogManager->addDialog( dialog, m_typeId );
    dialog->show();
}

void ListView::addView()
{
    AddViewDialog dialog( m_typeId, false, mainWidget() );
    if ( dialog.exec() == QDialog::Accepted ) {
        m_currentViewId = dialog.viewId();
        updateViews();
        loadCurrentView( true );
    }
}

void ListView::cloneView()
{
    CloneViewDialog dialog( m_currentViewId, false, mainWidget() );
    if ( dialog.exec() == QDialog::Accepted ) {
        m_currentViewId = dialog.viewId();
        updateViews();
        loadCurrentView( true );
    }
}

void ListView::modifyView()
{
    if ( m_currentViewId != 0 ) {
        ModifyViewDialog dialog( m_currentViewId, mainWidget() );
        dialog.exec();
    }
}

void ListView::updateEvent( UpdateEvent* e )
{
    if ( isEnabled() ) {
        if ( e->unit() == UpdateEvent::Types ) {
            updateViews();
            loadCurrentView( false );
            updateSearchOptions();
        }

        if ( e->unit() == UpdateEvent::States )
            updateActions();

        if ( e->unit() == UpdateEvent::Settings )
            loadCurrentView( false );
    }
}

void ListView::headerContextMenu( const QPoint& pos )
{
    QMenu* menu = builder()->contextMenu( "menuHeader" );
    if ( menu )
        menu->popup( m_list->header()->mapToGlobal( pos ) );
}

void ListView::listContextMenu( const QPoint& pos )
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

void ListView::doubleClicked( const QModelIndex& index )
{
    if ( index.isValid() ) {
        int issueId = m_model->rowId( index );
        viewManager->openIssueView( issueId );
    }
}

void ListView::setSelectedIssueId( int issueId )
{
    if ( m_selectedIssueId == issueId )
        return;

    QModelIndex index = m_model->findIndex( 0, issueId, 0 );

    if ( index.isValid() ) {
        m_list->selectionModel()->setCurrentIndex( index, QItemSelectionModel::Current );
        m_list->selectionModel()->select( index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
    }
}

void ListView::gotoIssue( int issueId, int itemId )
{
    if ( issueId != m_selectedIssueId )
        setSelectedIssueId( issueId );

    if ( issueId == m_selectedIssueId )
        emit itemActivated( itemId );
}

void ListView::updateSearchOptions()
{
    IssueTypeCache* cache = dataManager->issueTypeCache( m_typeId );

    QList<int> allColumns = cache->availableColumns();

    QList<int> columns;
    foreach ( int column, allColumns ) {
        if ( cache->availableOperators( column ).contains( "CON" ) )
            columns.append( column );
    }

    if ( !columns.contains( m_searchColumn ) ) {
        m_searchColumn = Column_Name;
        m_model->setSearchText( m_searchColumn, m_searchBox->text() );
    }

    m_searchMenu->clear();

    ViewSettingsHelper helper( m_typeId );

    foreach ( int column, columns )
    {
        QString name = helper.columnName( column );
        QAction* action = m_searchMenu->addAction( name );
        action->setData( column );
        action->setCheckable( true );
        if ( column == m_searchColumn )
            action->setChecked( true );
        action->setActionGroup( m_searchActionGroup );
    }

    m_searchBox->setPlaceholderText( helper.columnName( m_searchColumn ) );
}

void ListView::quickSearchChanged( const QString& text )
{
    m_model->setSearchText( m_searchColumn, text );
}

void ListView::searchActionTriggered( QAction* action )
{
    m_searchColumn = action->data().toInt();

    ViewSettingsHelper helper( m_typeId );
    m_searchBox->setPlaceholderText( helper.columnName( m_searchColumn ) );

    m_model->setSearchText( m_searchColumn, m_searchBox->text() );
}

bool ListView::eventFilter( QObject* obj, QEvent* e )
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

void ListView::updateViews()
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

void ListView::viewActivated( int index )
{
    m_currentViewId = m_viewCombo->itemData( index ).toInt();

    loadCurrentView( true );

    updateActions();
}

void ListView::setCurrentViewId( int viewId )
{
    if ( m_currentViewId == viewId || !isEnabled() )
        return;

    for ( int i = 0; i < m_viewCombo->count(); i++ ) {
        if ( m_viewCombo->itemData( i ).toInt() == viewId ) {
            m_viewCombo->setCurrentIndex( i );
            viewActivated( i );
            break;
        }
    }
}

void ListView::loadCurrentView( bool resort )
{
    TreeViewHelper helper( m_list );
    helper.saveColumnWidths( "FolderViewWidths", m_model->columns() );

    m_model->setView( m_currentViewId, resort );

    if ( resort )
        m_list->sortByColumn( m_model->sortColumn(), m_model->sortOrder() );

    IssueTypeCache* cache = dataManager->issueTypeCache( m_typeId );

    helper.loadColumnWidths( "FolderViewWidths", m_model->columns(), cache->defaultWidths() );

    m_list->setColumnHidden( 0, dataManager->setting( "hide_id_column" ).toInt() != 0 );

    if ( resort )
        emit currentViewChanged( m_currentViewId );
}

QList<int> ListView::visibleIssues()
{
    QList<int> list;

    int rows = m_model->rowCount();
    for ( int i = 0; i < rows; i++ ) {
        int issueId = m_model->rowId( m_model->index( i, 0 ) );
        list.append( issueId );
    }

    return list;
}

QList<int> ListView::visibleColumns()
{
    QList<int> list;

    QList<int> columns = m_model->columns();

    for ( int i = 0; i < columns.count(); i++ ) {
        if ( !m_list->isColumnHidden( i ) )
            list.append( columns.at( i ) );
    }

    return list;
}
