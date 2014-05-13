/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2014 WebIssues Team
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

#include "mainwindow.h"

#include "application.h"
#include "commands/commandmanager.h"
#include "data/datamanager.h"
#include "data/entities.h"
#include "data/localsettings.h"
#include "dialogs/dialogmanager.h"
#include "dialogs/messagebox.h"
#include "dialogs/userdialogs.h"
#include "dialogs/finditemdialog.h"
#include "dialogs/preferencesdialog.h"
#include "dialogs/settingsdialog.h"
#include "dialogs/connectioninfodialog.h"
#include "utils/iconloader.h"
#include "views/projectsview.h"
#include "views/summaryview.h"
#include "views/folderview.h"
#include "views/globallistview.h"
#include "views/issueview.h"
#include "views/startview.h"
#include "views/viewmanager.h"
#include "widgets/statuslabel.h"
#include "widgets/panewidget.h"
#include "xmlui/builder.h"
#include "xmlui/toolstrip.h"

#include <QAction>
#include <QCloseEvent>
#include <QApplication>
#include <QSettings>
#include <QSplitter>
#include <QFileDialog>
#include <QMessageBox>
#include <QLayout>
#include <QLabel>
#include <QTimer>
#include <QMenu>
#include <QStatusBar>
#include <QAccessible>
#include <QDesktopWidget>
#include <QSslCipher>

#if defined( Q_WS_WIN )
static const int TrayIconSize = 16;
#else
static const int TrayIconSize = 22;
#endif

MainWindow::MainWindow() :
    m_view( NULL ),
    m_summaryView( NULL ),
    m_folderView( NULL ),
    m_globalListView( NULL ),
    m_issueView( NULL ),
    m_activeView( NULL ),
    m_stackedWidget( NULL ),
    m_folderPane( NULL ),
    m_issuePane( NULL ),
    m_selectedProjectId( 0 ),
    m_selectedFolderId( 0 ),
    m_selectedTypeId( 0 ),
    m_selectedIssueId( 0 ),
    m_currentViewId( 0 ),
    m_supressFilter( false )
{
    setWindowTitle( tr( "WebIssues Desktop Client" ) );

    QAction* action;

    action = new QAction( IconLoader::icon( "file-quit" ), tr( "Quit" ), this );
    action->setMenuRole( QAction::QuitRole );
    action->setShortcut( tr( "Ctrl+Q" ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( quit() ), Qt::QueuedConnection );
    setAction( "quit", action );

    action = new QAction( IconLoader::icon( "connection-close" ), tr( "Close Connection" ), this );
    action->setIconText( tr( "Close" ) );
    action->setVisible( false );
    connect( action, SIGNAL( triggered() ), this, SLOT( closeConnection() ), Qt::QueuedConnection );
    setAction( "closeConnection", action );

    action = new QAction( IconLoader::icon( "status-info" ), tr( "Connection Details" ), this );
    action->setIconText( tr( "Details" ) );
    action->setVisible( false );
    connect( action, SIGNAL( triggered() ), this, SLOT( connectionInfo() ), Qt::QueuedConnection );
    setAction( "connectionInfo", action );

    action = new QAction( IconLoader::icon( "view-users" ), tr( "User Accounts" ), this );
    action->setIconText( tr( "Users" ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( showUsers() ), Qt::QueuedConnection );
    setAction( "showUsers", action );

    action = new QAction( IconLoader::icon( "view-types" ), tr( "Issue Types" ), this );
    action->setIconText( tr( "Types" ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( showTypes() ), Qt::QueuedConnection );
    setAction( "showTypes", action );

    action = new QAction( IconLoader::icon( "edit-goto" ), tr( "Go To Item" ), this );
    action->setShortcut( tr( "Ctrl+G" ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( gotoItem() ), Qt::QueuedConnection );
    setAction( "gotoItem", action );

    action = new QAction( IconLoader::icon( "edit-password" ), tr( "Change Password" ), this );
    action->setIconText( tr( "Password" ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( changePassword() ), Qt::QueuedConnection );
    setAction( "changePassword", action );

    action = new QAction( IconLoader::icon( "preferences" ), tr( "User Preferences" ), this );
    action->setIconText( tr( "Preferences" ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( userPreferences() ), Qt::QueuedConnection );
    setAction( "userPreferences", action );

    action = new QAction( IconLoader::icon( "configure" ), tr( "WebIssues Settings" ), this );
    action->setMenuRole( QAction::PreferencesRole );
    connect( action, SIGNAL( triggered() ), this, SLOT( configure() ), Qt::QueuedConnection );
    setAction( "configure", action );

    action = new QAction( IconLoader::icon( "about" ), tr( "About WebIssues" ), this );
    action->setShortcut( QKeySequence( Qt::Key_F1 ) );
    connect( action, SIGNAL( triggered() ), qApp, SLOT( about() ), Qt::QueuedConnection );
    setAction( "about", action );

    setTitle( "sectionTools", tr( "Tools" ) );
    setTitle( "sectionManage", tr( "Manage" ) );
    setTitle( "sectionConnection", tr( "Connection" ) );

    loadXmlUiFile( ":/resources/mainwindow.xml" );

    XmlUi::ToolStrip* strip = new XmlUi::ToolStrip( this );
    setMenuWidget( strip );

    XmlUi::Builder* builder = new XmlUi::Builder( this );
    builder->registerToolStrip( "stripMain", strip );
    builder->addClient( this );

    updateActions( false );

    QStatusBar* bar = statusBar();

    m_statusLabel = new StatusLabel( bar );
    bar->addWidget( m_statusLabel, 1 );

    m_summaryLabel = new StatusLabel( bar );
    bar->addWidget( m_summaryLabel, 0 );

    m_encryptionLabel = new StatusLabel( bar );
    bar->addWidget( m_encryptionLabel, 0 );

    m_userLabel = new StatusLabel( bar );
    bar->addWidget( m_userLabel, 0 );

    showStartPage();

    LocalSettings* settings = application->applicationSettings();
    if ( settings->contains( "MainWindowGeometry" ) ) {
        restoreGeometry( settings->value( "MainWindowGeometry" ).toByteArray() );
    } else {
        QRect available = QApplication::desktop()->availableGeometry( this );
        resize( available.width() * 4 / 5, available.height() * 4 / 5 );
        setWindowState( Qt::WindowMaximized );
    }

    connect( settings, SIGNAL( settingsChanged() ), this, SLOT( settingsChanged() ) );

    m_trayIcon = new QSystemTrayIcon( this );
    m_trayIcon->setIcon( IconLoader::pixmap( "webissues", TrayIconSize ) );
    m_trayIcon->setToolTip( tr( "WebIssues Desktop Client\nNot connected" ) );

    connect( m_trayIcon, SIGNAL( activated( QSystemTrayIcon::ActivationReason ) ), this, SLOT( trayIconActivated( QSystemTrayIcon::ActivationReason ) ) );

    connect( builder, SIGNAL( reset() ), this, SLOT( builderReset() ) );
    builderReset();

    settingsChanged();

    showStatus( IconLoader::pixmap( "status-info" ), tr( "Not connected to server." ) );

    m_summaryLabel->hide();
    m_encryptionLabel->hide();
    m_userLabel->hide();

    m_selectionTimer = new QTimer( this );
    m_selectionTimer->setInterval( 400 );
    m_selectionTimer->setSingleShot( true );

    connect( m_selectionTimer, SIGNAL( timeout() ), this, SLOT( updateSelection() ) );
}

MainWindow::~MainWindow()
{
    storeViewState();

    LocalSettings* settings = application->applicationSettings();
    settings->setValue( "MainWindowGeometry", saveGeometry() );

    delete m_startView;
    m_startView = NULL;

    delete m_view;
    m_view = NULL;

    delete m_summaryView;
    m_summaryView = NULL;

    delete m_folderView;
    m_folderView = NULL;

    delete m_globalListView;
    m_globalListView = NULL;

    delete m_issueView;
    m_issueView = NULL;
}

void MainWindow::closeEvent( QCloseEvent* e )
{
    if ( m_trayIcon->isVisible() )
        hide();
    else
        quit();

    e->ignore();
}

QMenu* MainWindow::createPopupMenu()
{
    return NULL;
}

void MainWindow::reconnect()
{
    m_startView->reconnect();
}

void MainWindow::quit()
{
    if ( !queryCloseConnection() )
        return;

    qApp->quit();
}

void MainWindow::closeConnection()
{
    if ( !queryCloseConnection() )
        return;

    viewManager->closeAllViews();
    dialogManager->closeAllDialogs();

    storeViewState();

    qApp->removeEventFilter( this );

    m_selectionTimer->stop();

    builder()->supressUpdate();

    delete m_view;
    m_view = NULL;

    delete m_summaryView;
    m_summaryView = NULL;

    delete m_folderView;
    m_folderView = NULL;

    delete m_globalListView;
    m_globalListView = NULL;

    delete m_issueView;
    m_issueView = NULL;

    m_activeView = NULL;

    m_folderPane = NULL;
    m_issuePane = NULL;

    delete dataManager;
    dataManager = NULL;

    delete commandManager;
    commandManager = NULL;

    updateActions( false );

    builder()->resumeUpdate();

    m_selectedProjectId = 0;
    m_selectedFolderId = 0;
    m_selectedTypeId = 0;
    m_selectedIssueId = 0;

    showStartPage();

    m_trayIcon->setIcon( IconLoader::pixmap( "webissues", TrayIconSize ) );
    m_trayIcon->setToolTip( tr( "WebIssues Desktop Client\nNot connected" ) );

    setWindowTitle( tr( "WebIssues Desktop Client" ) );

    showStatus( IconLoader::pixmap( "status-info" ), tr( "Disconnected from server." ) );

    m_summaryLabel->hide();
    m_encryptionLabel->hide();
    m_userLabel->hide();
}

bool MainWindow::queryCloseConnection()
{
    if ( commandManager && commandManager->preventClose() ) {
        MessageBox::warning( this, tr( "Warning" ), tr( "Connection cannot be closed because there are pending commands.\nPlease wait until these commands are completed." ) );
        return false;
    }

    if ( viewManager && !viewManager->queryCloseViews() )
        return false;

    if ( dialogManager && !dialogManager->queryCloseDialogs() )
        return false;

    return true;
}

void MainWindow::showStartPage()
{
    XmlUi::ToolStrip* strip = qobject_cast<XmlUi::ToolStrip*>( menuWidget() );

    QWidget* headerWidget = new QWidget( strip );
    QHBoxLayout* headerLayout = new QHBoxLayout( headerWidget );

    QLabel* pixmapLabel = new QLabel( headerWidget );
    pixmapLabel->setPixmap( IconLoader::pixmap( "webissues", 32 ) );
    headerLayout->addWidget( pixmapLabel );

    QLabel* titleLabel = new QLabel( "<font size=\"+2\"><b>" + tr( "WebIssues Desktop Client" ) + "</b></font>&nbsp;&nbsp;" + application->version(), headerWidget );
    headerLayout->addWidget( titleLabel );

    strip->setHeaderWidget( headerWidget );

    m_startView = new StartView( this, this );
    viewManager->addView( m_startView );

    setCentralWidget( m_startView->mainWidget() );
    m_startView->mainWidget()->show();

    m_startView->mainWidget()->setFocus();

    connect( m_startView, SIGNAL( statusChanged( const QPixmap&, const QString&, int ) ), this, SLOT( showStatus( const QPixmap&, const QString&, int ) ) );
    connect( m_startView, SIGNAL( summaryChanged( const QPixmap&, const QString& ) ), this, SLOT( showSummary( const QPixmap&, const QString& ) ) );

    connect( m_startView, SIGNAL( connectionOpened() ), this, SLOT( connectionOpened() ), Qt::QueuedConnection );

    builder()->addClient( m_startView );

    m_startView->initialUpdate();
}

void MainWindow::connectionOpened()
{
    builder()->supressUpdate();

    delete m_startView;
    m_startView = NULL;

    XmlUi::ToolStrip* strip = qobject_cast<XmlUi::ToolStrip*>( menuWidget() );
    strip->setHeaderWidget( NULL );

    QSplitter* horizSplitter = new QSplitter( Qt::Horizontal, this );

    m_view = new ProjectsView( this, horizSplitter );
    viewManager->addView( m_view );

    horizSplitter->addWidget( m_view->mainWidget() );

    m_stackedWidget = new QStackedWidget( horizSplitter );

    m_summaryView = new SummaryView( this, m_stackedWidget );
    viewManager->addView( m_summaryView );

    m_stackedWidget->addWidget( m_summaryView->mainWidget() );

    QSplitter* vertSplitter = new QSplitter( Qt::Vertical, m_stackedWidget );

    m_folderPane = new PaneWidget( vertSplitter ); // TODO: change API of the pane widget to allow adding more widgets
    m_folderPane->setPlaceholderText( tr( "No folder selected" ) );
    vertSplitter->addWidget( m_folderPane );

    m_folderView = new FolderView( this, m_folderPane );
    m_folderView->mainWidget()->setFocusPolicy( Qt::ClickFocus );
    viewManager->addView( m_folderView );

    m_folderPane->addWidget( m_folderView->mainWidget() );

    m_globalListView = new GlobalListView( this, m_folderPane );
    m_globalListView->mainWidget()->setFocusPolicy( Qt::ClickFocus );
    viewManager->addView( m_globalListView );

    m_folderPane->addWidget( m_globalListView->mainWidget() );

    m_issuePane = new PaneWidget( vertSplitter );
    m_issuePane->setPlaceholderText( tr( "No issue selected" ) );
    vertSplitter->addWidget( m_issuePane );

    m_issueView = new IssueView( this, m_issuePane );
    m_issueView->mainWidget()->setFocusPolicy( Qt::ClickFocus );
    viewManager->addView( m_issueView );

    m_issuePane->addWidget( m_issueView->mainWidget() );

    m_stackedWidget->addWidget( vertSplitter );

    horizSplitter->addWidget( m_stackedWidget );

    setCentralWidget( horizSplitter );
    restoreViewState();

    m_stackedWidget->setCurrentIndex( 1 );
    horizSplitter->show();

    updateActions( true );

    connect( m_view, SIGNAL( captionChanged( const QString& ) ), this, SLOT( captionChanged( const QString& ) ) );

    connect( m_view, SIGNAL( statusChanged( const QPixmap&, const QString&, int ) ), this, SLOT( showStatus( const QPixmap&, const QString&, int ) ) );
    connect( m_view, SIGNAL( summaryChanged( const QPixmap&, const QString& ) ), this, SLOT( showSummary( const QPixmap&, const QString& ) ) );

#if !defined( QT_NO_OPENSSL )
    QSslCipher cipher = commandManager->sslConfiguration().sessionCipher();
    if ( !cipher.isNull() ) {
        m_encryptionLabel->setPixmap( IconLoader::pixmap( "edit-access" ) );
        m_encryptionLabel->setText( tr( "Encrypted" ) );
        m_encryptionLabel->show();
    }
#endif

    QPixmap userPixmap = ( dataManager->currentUserAccess() == AdminAccess ) ? IconLoader::overlayedPixmap( "user", "overlay-admin" ) : IconLoader::pixmap( "user" );
    m_userLabel->setPixmap( userPixmap );
    m_userLabel->setText( dataManager->currentUserName() );
    m_userLabel->show();

    dataManager->addObserver( this );

    m_activeView = m_view;

    builder()->addClient( m_view );

    builder()->resumeUpdate();

    qApp->installEventFilter( this );

    m_trayIcon->setIcon( IconLoader::overlayedPixmap( "webissues", "overlay-connected", TrayIconSize ) );
    m_trayIcon->setToolTip( tr( "WebIssues Desktop Client\nConnected to %1" ).arg( dataManager->serverName() ) );

    m_view->initialUpdate();

    connect( m_folderView, SIGNAL( enabledChanged( bool ) ), this, SLOT( folderEnabledChanged( bool ) ) );
    connect( m_globalListView, SIGNAL( enabledChanged( bool ) ), this, SLOT( globalListEnabledChanged( bool ) ) );
    connect( m_issueView, SIGNAL( enabledChanged( bool ) ), this, SLOT( issueEnabledChanged( bool ) ) );

    connect( m_folderView, SIGNAL( captionChanged( const QString& ) ), this, SLOT( captionChanged( const QString& ) ) );
    connect( m_globalListView, SIGNAL( captionChanged( const QString& ) ), this, SLOT( captionChanged( const QString& ) ) );
    connect( m_summaryView, SIGNAL( captionChanged( const QString& ) ), this, SLOT( captionChanged( const QString& ) ) );

    connect( m_view, SIGNAL( selectionChanged( int, int, int ) ), this, SLOT( selectionChanged( int, int, int ) ) );
    connect( m_view, SIGNAL( projectSelected( int ) ), this, SLOT( projectSelected( int ) ) );

    connect( m_folderView, SIGNAL( selectedIssueChanged( int ) ), this, SLOT( selectedIssueChanged( int ) ) );
    connect( m_folderView, SIGNAL( currentViewChanged( int ) ), this, SLOT( currentViewChanged( int ) ) );
    connect( m_globalListView, SIGNAL( selectedIssueChanged( int ) ), this, SLOT( selectedIssueChanged( int ) ) );
    connect( m_globalListView, SIGNAL( currentViewChanged( int ) ), this, SLOT( currentViewChanged( int ) ) );

    connect( m_issueView, SIGNAL( issueActivated( int, int ) ), this, SLOT( gotoIssue( int, int ) ), Qt::QueuedConnection );
    connect( m_folderView, SIGNAL( issueActivated( int, int ) ), this, SLOT( gotoIssue( int, int ) ), Qt::QueuedConnection );
    connect( m_globalListView, SIGNAL( issueActivated( int, int ) ), this, SLOT( gotoIssue( int, int ) ), Qt::QueuedConnection );
    connect( m_summaryView, SIGNAL( issueActivated( int, int ) ), this, SLOT( gotoIssue( int, int ) ), Qt::QueuedConnection );

    connect( m_folderView, SIGNAL( itemActivated( int ) ), this, SLOT( gotoItem( int ) ) );
    connect( m_globalListView, SIGNAL( itemActivated( int ) ), this, SLOT( gotoItem( int ) ) );

    folderEnabledChanged( m_folderView->isEnabled() );
    globalListEnabledChanged( m_folderView->isEnabled() );
    issueEnabledChanged( m_issueView->isEnabled() );

    m_folderView->initialUpdate();
    m_globalListView->initialUpdate();
    m_issueView->initialUpdate();

    m_view->mainWidget()->setFocus();
}

void MainWindow::customEvent( QEvent* e )
{
    if ( e->type() == UpdateEvent::Type ) {
        UpdateEvent* ue = (UpdateEvent*)e;
        if ( ue->unit() == UpdateEvent::GlobalAccess ) {
            QPixmap userPixmap = ( dataManager->currentUserAccess() == AdminAccess ) ? IconLoader::overlayedPixmap( "user", "overlay-admin" ) : IconLoader::pixmap( "user" );
            m_userLabel->setPixmap( userPixmap );
            m_userLabel->setText( dataManager->currentUserName() );

            updateActions( true );
        }
    }
}

void MainWindow::restoreViewState()
{
    LocalSettings* settings = application->applicationSettings();

    if ( QSplitter* horizSplitter = qobject_cast<QSplitter*>( centralWidget() ) ) {
        QByteArray horizData = settings->value( "MainWindowHSplit" ).toByteArray();

        if ( !horizData.isEmpty() )
            horizSplitter->restoreState( horizData );
        else
            horizSplitter->setSizes( QList<int>() << 350 << width() - 350 );

        if ( QSplitter* vertSplitter = qobject_cast<QSplitter*>( m_stackedWidget->widget( 1 ) ) ) {
            QByteArray vertData = settings->value( "MainWindowVSplit" ).toByteArray();

            if ( !vertData.isEmpty() )
                vertSplitter->restoreState( vertData );
            else
                vertSplitter->setSizes( QList<int>() << height() / 3 << height() * 2 / 3 );
        }
    }
}

void MainWindow::storeViewState()
{
    LocalSettings* settings = application->applicationSettings();
    if ( QSplitter* horizSplitter = qobject_cast<QSplitter*>( centralWidget() ) ) {
        settings->setValue( "MainWindowHSplit", horizSplitter->saveState() );
        if ( QSplitter* vertSplitter = qobject_cast<QSplitter*>( m_stackedWidget->widget( 1 ) ) )
            settings->setValue( "MainWindowVSplit", vertSplitter->saveState() );
    }
}

void MainWindow::updateActions( bool connected )
{
    bool isAdmin = connected ? dataManager->currentUserAccess() == AdminAccess : false;

    action( "closeConnection" )->setVisible( connected );
    action( "connectionInfo" )->setVisible( connected );
    action( "showUsers" )->setVisible( connected && isAdmin );
    action( "showTypes" )->setVisible( connected && isAdmin );
    action( "gotoItem" )->setVisible( connected );
    action( "changePassword" )->setVisible( connected );
    action( "userPreferences" )->setVisible( connected );

    builder()->rebuildAll();
}

bool MainWindow::eventFilter( QObject* object, QEvent* e )
{
    if ( e->type() != QEvent::MouseButtonPress &&
        e->type() != QEvent::MouseButtonDblClick &&
        e->type() != QEvent::FocusIn &&
        e->type() != QEvent::ContextMenu )
        return false;

    if ( !object->isWidgetType() )
        return false;

    QWidget* widget = static_cast<QWidget*>( object );

    if ( ( ( widget->windowFlags().testFlag( Qt::Dialog ) ) && widget->isModal() ) ||
        ( widget->windowFlags().testFlag( Qt::Popup ) ) || ( widget->windowFlags().testFlag( Qt::Tool ) ) )
        return false;

    while ( widget ) {
        if ( widget->topLevelWidget() != this )
            return false;

        View* view = NULL;
        if ( m_view->mainWidget() == widget )
            view = m_view;
        else if ( m_summaryView && m_summaryView->mainWidget() == widget )
            view = m_summaryView;
        else if ( m_folderView && m_folderView->mainWidget() == widget )
            view = m_folderView;
        else if ( m_globalListView && m_globalListView->mainWidget() == widget )
            view = m_globalListView;
        else if ( m_issueView && m_issueView->mainWidget() == widget )
            view = m_issueView;

        if ( view ) {
            setActiveView( view );
            return false;
        }

        widget = widget->parentWidget();
    }

    return false;
}

void MainWindow::setActiveView( View* view )
{
    if ( m_activeView == view )
        return;

    builder()->supressUpdate();

    if ( m_activeView ) {
        disconnect( m_activeView, SIGNAL( statusChanged( const QPixmap&, const QString&, int ) ), this, NULL );
        disconnect( m_activeView, SIGNAL( summaryChanged( const QPixmap&, const QString& ) ), this, NULL );
        builder()->removeClient( m_activeView );
    }

    m_activeView = view;

    connect( view, SIGNAL( statusChanged( const QPixmap&, const QString&, int ) ), this, SLOT( showStatus( const QPixmap&, const QString&, int ) ) );
    connect( view, SIGNAL( summaryChanged( const QPixmap&, const QString& ) ), this, SLOT( showSummary( const QPixmap&, const QString& ) ) );

    showStatus( view->statusPixmap(), view->statusText() );
    showSummary( view->summaryPixmap(), view->summaryText() );

    if ( view->isEnabled() )
        builder()->addClient( view );

    builder()->resumeUpdate();

    QCoreApplication::sendPostedEvents();
}

void MainWindow::folderEnabledChanged( bool enabled )
{
    if ( enabled )
        m_folderPane->setCurrentWidget( m_folderView->mainWidget() );
    else if ( !m_globalListView->isEnabled() )
        m_folderPane->showPlaceholder();

    if ( m_folderView == m_activeView ) {
        if ( enabled )
           builder()->addClient( m_folderView );
        else
            builder()->removeClient( m_folderView );
    }
}

void MainWindow::globalListEnabledChanged( bool enabled )
{
    if ( enabled )
        m_folderPane->setCurrentWidget( m_globalListView->mainWidget() );
    else if ( !m_folderView->isEnabled() )
        m_folderPane->showPlaceholder();

    if ( m_globalListView == m_activeView ) {
        if ( enabled )
           builder()->addClient( m_globalListView );
        else
            builder()->removeClient( m_globalListView );
    }
}

void MainWindow::issueEnabledChanged( bool enabled )
{
    if ( enabled )
        m_issuePane->setCurrentWidget( m_issueView->mainWidget() );
    else
        m_issuePane->showPlaceholder();

    if ( m_issueView == m_activeView ) {
        if ( enabled )
           builder()->addClient( m_issueView );
        else
            builder()->removeClient( m_issueView );
    }
}

void MainWindow::selectionChanged( int folderId, int typeId, int viewId )
{
    if ( !m_supressFilter && ( m_selectedFolderId != folderId || m_selectedTypeId != typeId || m_currentViewId != viewId || m_selectedProjectId != 0 ) ) {
        m_selectedProjectId = 0;
        m_selectedFolderId = folderId;
        m_selectedTypeId = typeId;
        m_currentViewId = viewId;
        m_selectionTimer->start();
    }
}

void MainWindow::projectSelected( int projectId )
{
    if ( m_selectedProjectId != projectId ) {
        m_selectedProjectId = projectId;
        m_selectedFolderId = 0;
        m_selectedTypeId = 0;
        m_currentViewId = 0;
        m_selectionTimer->start();
    }
}

void MainWindow::selectedIssueChanged( int issueId )
{
    if ( m_selectedIssueId != issueId ) {
        m_selectedIssueId = issueId;
        m_selectionTimer->start();
    }
}

void MainWindow::currentViewChanged( int viewId )
{
    if ( !m_supressFilter && m_currentViewId != viewId ) {
        m_currentViewId = viewId;
        m_selectionTimer->start();
    }
}

void MainWindow::updateSelection()
{
    if ( m_selectedProjectId != 0 ) {
        m_stackedWidget->setCurrentIndex( 0 );
    } else {
        m_stackedWidget->setCurrentIndex( 1 );

        m_supressFilter = true;
        m_view->setSelection( m_selectedFolderId, m_selectedTypeId, m_currentViewId );
        m_supressFilter = false;
    }

    if ( m_summaryView->id() != m_selectedProjectId ) {
        m_summaryView->setId( m_selectedProjectId );
        m_summaryView->initialUpdate();
    }

    if ( m_folderView->id() != m_selectedFolderId || m_globalListView->id() != m_selectedTypeId ) {
        m_supressFilter = true;

        if ( m_globalListView->id() != 0 && m_selectedTypeId == 0 ) {
            m_globalListView->setId( m_selectedTypeId );
            m_globalListView->initialUpdate();
        }

        if ( m_folderView->id() != m_selectedFolderId ) {
            m_folderView->setId( m_selectedFolderId );
            m_folderView->initialUpdate();
        }

        if ( m_globalListView->id() != m_selectedTypeId ) {
            m_globalListView->setId( m_selectedTypeId );
            m_globalListView->initialUpdate();
        }

        m_supressFilter = false;
        m_selectedIssueId = 0;
    }

    m_folderView->setCurrentViewId( m_currentViewId );
    m_globalListView->setCurrentViewId( m_currentViewId );

    if ( m_issueView->id() != m_selectedIssueId ) {
        m_issueView->setId( m_selectedIssueId );
        m_issueView->initialUpdate();
    }
}

void MainWindow::configure()
{
    SettingsDialog dialog( this );
    dialog.exec();
}

void MainWindow::settingsChanged()
{
    LocalSettings* settings = application->applicationSettings();
    bool docked = settings->value( "Docked" ).toBool();

    m_trayIcon->setVisible( docked );

    XmlUi::ToolStrip* strip = qobject_cast<XmlUi::ToolStrip*>( menuWidget() );

    strip->clearAuxiliaryActions();
    strip->addAuxiliaryAction( this->action( "configure" ) );
    strip->addAuxiliaryAction( this->action( "about" ) );
    if ( docked )
        strip->addAuxiliaryAction( this->action( "quit" ) );
}

void MainWindow::connectionInfo()
{
    ConnectionInfoDialog dialog( this );
    dialog.exec();
}

void MainWindow::showUsers()
{
    if ( dataManager->currentUserAccess() == AdminAccess )
        viewManager->openUsersView();
}

void MainWindow::showTypes()
{
    if ( dataManager->currentUserAccess() == AdminAccess )
        viewManager->openTypesView();
}

void MainWindow::gotoItem()
{
    FindItemDialog dialog( this );
    if ( dialog.exec() == QDialog::Accepted )
        gotoIssue( dialog.issueId(), dialog.itemId() );
}

void MainWindow::gotoIssue( int issueId, int itemId )
{
    IssueEntity issue = IssueEntity::find( issueId );

    if ( issue.isValid() ) {
        if ( m_selectedTypeId == issue.folder().typeId() ) {
            m_globalListView->gotoIssue( issueId, itemId );
        } else {
            int folderId = issue.folderId();

            if ( folderId != m_folderView->id() ) {
                selectionChanged( folderId, 0, 0 );
                updateSelection();
            }

            if ( issueId != m_issueView->id() )
                m_folderView->gotoIssue( issueId, itemId );
            else
                m_issueView->gotoItem( itemId );
        }

        if ( m_trayIcon->isVisible() )
            showFromTray( false );
    } else {
        viewManager->openIssueView( issueId, itemId );
    }
}

void MainWindow::gotoItem( int itemId )
{
    updateSelection();
    m_issueView->gotoItem( itemId );
}

void MainWindow::changePassword()
{
    SetPasswordDialog dialog( dataManager->currentUserId(), this );
    dialog.exec();
}

void MainWindow::userPreferences()
{
    PreferencesDialog dialog( dataManager->currentUserId(), this );
    dialog.exec();
}

void MainWindow::trayIconActivated( QSystemTrayIcon::ActivationReason reason )
{
    if ( reason == QSystemTrayIcon::Trigger )
        showFromTray( true );

    // workaround for [QTBUG-14807]
    if ( reason == QSystemTrayIcon::Context )
        m_trayIcon->contextMenu()->activateWindow();
}

void MainWindow::showFromTray( bool toggle )
{
    if ( isMinimized() ) {
#if defined( Q_WS_WIN )
        setWindowState( windowState() & ~Qt::WindowMinimized );
#else
        // hide needed when changing desktop
        hide();
        showNormal();
#endif
        raise();
        activateWindow();
    } else if ( !isVisible() ) {
        show();
        raise();
        activateWindow();
    } else if ( !toggle ) {
        raise();
        activateWindow();
    } else {
        hide();
    }
}

void MainWindow::captionChanged( const QString& /*caption*/ )
{
    QString caption = m_view->caption();

    if ( m_summaryView->isEnabled() )
        caption = QString( "%1 - %2" ).arg( m_summaryView->caption(), caption );
    else if ( m_folderView->isEnabled() )
        caption = QString( "%1 - %2" ).arg( m_folderView->caption(), caption );
    else if ( m_globalListView->isEnabled() )
        caption = QString( "%1 - %2" ).arg( m_globalListView->caption(), caption );

    setWindowTitle( tr( "%1 - WebIssues Desktop Client" ).arg( caption ) );
}

void MainWindow::builderReset()
{
    m_trayIcon->setContextMenu( builder()->contextMenu( "menuTray" ) );
}

void MainWindow::showStatus( const QPixmap& pixmap, const QString& text, int icon )
{
    m_statusLabel->setPixmap( pixmap );
    m_statusLabel->setText( text );

    if ( icon != 0 && topLevelWidget()->isActiveWindow() ) {
        QMessageBox box;
        box.setIcon( (QMessageBox::Icon)icon );
        QAccessible::updateAccessibility( &box, 0, QAccessible::Alert );
    }
}

void MainWindow::showSummary( const QPixmap& pixmap, const QString& text )
{
    if ( !text.isEmpty() || !pixmap.isNull() ) {
        m_summaryLabel->setPixmap( pixmap );
        m_summaryLabel->setText( text );
        m_summaryLabel->show();
    } else {
        m_summaryLabel->hide();
    }
}
