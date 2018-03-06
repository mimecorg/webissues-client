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

#include "startview.h"

#include "application.h"
#include "commands/commandmanager.h"
#include "commands/loginbatch.h"
#include "data/datamanager.h"
#include "data/bookmarksstore.h"
#include "data/credentialsstore.h"
#include "data/bookmark.h"
#include "dialogs/logindialog.h"
#include "dialogs/messagebox.h"
#include "utils/treeviewhelper.h"
#include "utils/errorhelper.h"
#include "utils/iconloader.h"
#include "widgets/inputlineedit.h"
#include "xmlui/gradientwidget.h"
#include "xmlui/builder.h"

#include <QLayout>
#include <QLabel>
#include <QToolButton>
#include <QTreeWidget>
#include <QMenu>
#include <QAction>

StartView::StartView( QObject* parent, QWidget* parentWidget ) : View( parent ),
    m_batch( NULL ),
    m_remember( false )
{
    QAction* action;

    action = new QAction( IconLoader::icon( "connection-open" ), tr( "&Connect As..." ), this );
    connect( action, SIGNAL( triggered() ), this, SLOT( openConnection() ), Qt::QueuedConnection );
    setAction( "openConnection", action );

    for ( int i = 0; i < 5; i++ ) {
        action = new QAction( IconLoader::icon( "connection-open" ), QString(), this );
        action->setVisible( false );
        connect( action, SIGNAL( triggered() ), this, SLOT( openConnectionAs() ), Qt::QueuedConnection );
        setAction( QString( "openConnectionAs%1" ).arg( i + 1 ), action );
    }

    action = new QAction( IconLoader::icon( "edit-delete" ), tr( "&Remove Connection" ), this );
    connect( action, SIGNAL( triggered() ), this, SLOT( removeConnection() ), Qt::QueuedConnection );
    setAction( "removeConnection", action );

    loadXmlUiFile( ":/resources/startview.xml" );

    QWidget* topWidget = new QWidget( parentWidget );

    QVBoxLayout* topLayout = new QVBoxLayout( topWidget );
    topLayout->setMargin( 0 );
    topLayout->setSpacing( 0 );

    QFrame* separator = new QFrame( topWidget );
    separator->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    separator->setFixedHeight( 2 );
    topLayout->addWidget( separator );

    QWidget* mainWidget = new QWidget( topWidget );
#if defined( Q_WS_WIN )
    mainWidget->setAutoFillBackground( true );
#endif
    topLayout->addWidget( mainWidget );

    QHBoxLayout* mainLayout = new QHBoxLayout( mainWidget );
    mainLayout->setMargin( 9 );
    mainLayout->setSpacing( 6 );

    mainLayout->addSpacerItem( new QSpacerItem( 100, 20, QSizePolicy::Maximum, QSizePolicy::Ignored ) );

    QVBoxLayout* innerLayout = new QVBoxLayout();
    mainLayout->addLayout( innerLayout );

    innerLayout->addSpacerItem( new QSpacerItem( 20, 100, QSizePolicy::Ignored, QSizePolicy::Maximum ) );

    QLabel* label1 = new QLabel( tr( "Enter address of the WebIssues Server:" ), mainWidget );
    innerLayout->addWidget( label1 );

    QHBoxLayout* editLayout = new QHBoxLayout();
    editLayout->setSpacing( 3 );
    innerLayout->addLayout( editLayout );

    m_edit = new InputLineEdit( mainWidget );
    m_edit->setRequired( true );
    m_edit->setFormat( InputLineEdit::UrlFormat );
    editLayout->addWidget( m_edit );

    connect( m_edit, SIGNAL( returnPressed() ), this, SLOT( returnPressed() ) );

    m_button = new QToolButton( mainWidget );
    m_button->setAutoRaise( true );
    m_button->setIcon( IconLoader::icon( "connection-go" ) );
    m_button->setIconSize( QSize( 16, 16 ) );
    m_button->setToolTip( tr( "Connect" ) );
    editLayout->addWidget( m_button );

    connect( m_button, SIGNAL( clicked() ), this, SLOT( buttonClicked() ) );

    innerLayout->addSpacing( 20 );

    QLabel* label2 = new QLabel( tr( "Recent connections:" ), mainWidget );
    innerLayout->addWidget( label2 );

    m_list = new QTreeWidget( mainWidget );
    m_list->setMinimumSize( QSize( 400, 200 ) );
    innerLayout->addWidget( m_list );

    TreeViewHelper helper( m_list );
    helper.initializeView( TreeViewHelper::NotSortable );

    connect( m_list, SIGNAL( customContextMenuRequested( const QPoint& ) ), this, SLOT( contextMenu( const QPoint& ) ) );
    connect( m_list, SIGNAL( doubleClicked( const QModelIndex& ) ), this, SLOT( doubleClicked( const QModelIndex& ) ) );
    connect( m_list->selectionModel(), SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
        this, SLOT( selectionChanged() ) );

    connect( m_edit, SIGNAL( textEdited( const QString& ) ), m_list, SLOT( clearSelection() ) );

    innerLayout->addSpacerItem( new QSpacerItem( 20, 100, QSizePolicy::Ignored, QSizePolicy::Maximum ) );
    mainLayout->addSpacerItem( new QSpacerItem( 100, 20, QSizePolicy::Maximum, QSizePolicy::Ignored ) );

    topWidget->setFocusProxy( m_edit );

    setMainWidget( topWidget );
}

StartView::~StartView()
{
    TreeViewHelper helper( m_list );
    helper.saveColumnWidths( "StartViewWidths" );
}

void StartView::reconnect()
{
    QList<Bookmark> bookmarks = application->bookmarksStore()->bookmarks();

    if ( bookmarks.isEmpty() )
        return;

    Bookmark bookmark = bookmarks.first();

    QList<ServerCredential> credentials = application->credentialsStore()->serverCredentials( bookmark.serverUuid() );
    ServerCredential credential = !credentials.isEmpty() ? credentials.first() : ServerCredential();

    openConnection( bookmark.url(), credential );
}

void StartView::openConnection()
{
    Bookmark bookmark = selectedBookmark();
    if ( !bookmark.url().isEmpty() )
        openConnection( bookmark.url() );
}

void StartView::openConnectionAs()
{
    Bookmark bookmark = selectedBookmark();
    if ( !bookmark.url().isEmpty() ) {
        QAction* action = (QAction*)sender();
        ServerCredential credential = action->data().value<ServerCredential>();
        openConnection( bookmark.url(), credential );
    }
}

void StartView::removeConnection()
{
    Bookmark bookmark = selectedBookmark();
    if ( !bookmark.url().isEmpty() ) {
        if ( MessageBox::warning( mainWidget(), tr( "Warning" ),
             tr( "Are you sure you want to remove the selected connection?" ),
             QMessageBox::Ok | QMessageBox::Cancel ) == QMessageBox::Ok ) {
            application->bookmarksStore()->deleteBookmark( bookmark );
            delete m_list->selectedItems().first();
        }
    }
}

void StartView::buttonClicked()
{
    if ( !m_batch )
        returnPressed();
    else
        commandManager->abort( m_batch );
}

void StartView::returnPressed()
{
    if ( m_edit->validate() )
        openConnection( m_edit->inputValue() );
    else
        showWarning( tr( "The address you entered is not valid." ) );
}

void StartView::openConnection( const QString& url, const ServerCredential& credential /*= ServerCredential()*/ )
{
    QUrl parsedUrl = QUrl::fromEncoded( url.toUtf8(), QUrl::TolerantMode );

    showBusy( tr( "Connecting to server..." ) );

    commandManager = new CommandManager( application->networkAccessManager() );
    commandManager->setServerUrl( parsedUrl );

    dataManager = new DataManager();

    m_edit->setInputValue( commandManager->serverUrl().toString() );
    m_edit->setEnabled( false );
    m_list->setEnabled( false );

    m_button->setIcon( IconLoader::icon( "connection-abort" ) );
    m_button->setToolTip( tr( "Abort" ) );

    m_login = credential.login();
    m_password = credential.password();
    m_remember = !m_password.isEmpty();

    LoginBatch* batch = new LoginBatch();
    batch->hello();
    if ( m_remember ) {
        batch->setExpectedUuid( credential.serverUuid() );
        batch->login( m_login, m_password );
    }

    executeBatch( batch );
}

void StartView::cancelConnection()
{
    delete dataManager;
    dataManager = NULL;

    commandManager->deleteLater();
    commandManager = NULL;

    m_batch = NULL;

    m_edit->setEnabled( true );
    m_list->setEnabled( true );

    m_button->setIcon( IconLoader::icon( "connection-go" ) );
    m_button->setToolTip( tr( "Connect" ) );
}

void StartView::executeBatch( AbstractBatch* batch )
{
    m_batch = batch;

    connect( batch, SIGNAL( completed( bool ) ), this, SLOT( loginCompleted( bool ) ) );

    commandManager->execute( batch );
}

void StartView::loginCompleted( bool successful )
{
    if ( successful ) {
        if ( !dataManager->isValid() ) {
            showError( tr( "Local cache database cannot be opened." ) );
            cancelConnection();
        } else if ( dataManager->currentUserAccess() == NoAccess ) {
            showLoginDialog( false );
        } else {
            Bookmark bookmark( dataManager->serverName(), dataManager->serverUuid(), commandManager->serverUrl().toString() );
            application->bookmarksStore()->addBookmark( bookmark );

            ServerCredential credential( dataManager->serverUuid(), m_login, m_login, m_remember ? m_password : QString() );
            application->credentialsStore()->addServerCredential( credential );

            emit connectionOpened();
        }
    } else {
        if ( commandManager->error() == CommandManager::WebIssuesError && commandManager->errorCode() == ErrorHelper::IncorrectLogin ) {
            showLoginDialog( true );
        } else if ( commandManager->error() == CommandManager::WebIssuesError && commandManager->errorCode() == ErrorHelper::MustChangePassword ) {
            showLoginNewDialog();
        } else {
            showError( tr( "Connection could not be opened: %1." ).arg( commandManager->errorMessage() ) );
            cancelConnection();
        }
    }
}

void StartView::showLoginDialog( bool error )
{
    LoginDialog dialog( mainWidget() );

    dialog.setLoginAndPassword( m_login, m_remember ? m_password : QString() );

    if ( error )
        dialog.showLoginError();

    if ( dialog.exec() == QDialog::Accepted ) {
        m_login = dialog.login();
        m_password = dialog.password();
        m_remember = dialog.rememberPassword();

        LoginBatch* batch = new LoginBatch();
        batch->login( m_login, m_password );
        executeBatch( batch );
    } else {
        showWarning( tr( "Connection was cancelled." ) );
        cancelConnection();
    }
}

void StartView::showLoginNewDialog()
{
    LoginNewDialog dialog( m_password, mainWidget() );

    if ( dialog.exec() == QDialog::Accepted ) {
        QString oldPassword = m_password;
        m_password = dialog.password();

        LoginBatch* batch = new LoginBatch();
        batch->loginNew( m_login, oldPassword, m_password );
        executeBatch( batch );
    } else {
        showWarning( tr( "Connection was cancelled." ) );
        cancelConnection();
    }
}

void StartView::initialUpdate()
{
    QTreeWidgetItem* header = new QTreeWidgetItem();
    header->setText( 0, tr( "Name" ) );
    header->setText( 1, tr( "Address" ) );
    m_list->setHeaderItem( header );

    QList<Bookmark> bookmarks = application->bookmarksStore()->bookmarks();

    for ( int i = 0; i < bookmarks.count(); i++ )
    {
        Bookmark bookmark = bookmarks.at( i );
        QTreeWidgetItem* item = new QTreeWidgetItem( m_list );
        item->setText( 0, bookmark.serverName() );
        item->setText( 1, bookmark.url() );
        item->setIcon( 0, IconLoader::icon( "connection" ) );
        item->setData( 0, Qt::UserRole, QVariant::fromValue( bookmark ) );
    }

    TreeViewHelper helper( m_list );
    helper.loadColumnWidths( "StartViewWidths", QList<int>() << 150 << 300 );

    setAccess( NormalAccess, true );
}

void StartView::contextMenu( const QPoint& pos )
{
    QModelIndex index = m_list->indexAt( pos );

    if ( index.isValid() ) {
        m_list->selectionModel()->setCurrentIndex( index, QItemSelectionModel::Current );
        m_list->selectionModel()->select( index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );

        QMenu* menu = builder()->contextMenu( "menuConnection" );
        if ( menu )
            menu->popup( m_list->viewport()->mapToGlobal( pos ) );
    }
}

void StartView::doubleClicked( const QModelIndex& index )
{
    Bookmark bookmark = index.sibling( index.row(), 0 ).data( Qt::UserRole ).value<Bookmark>();

    QList<ServerCredential> credentials = application->credentialsStore()->serverCredentials( bookmark.serverUuid() );
    ServerCredential credential = !credentials.isEmpty() ? credentials.first() : ServerCredential();

    openConnection( bookmark.url(), credential );
}

void StartView::selectionChanged()
{
    QList<ServerCredential> credentials;

    Bookmark bookmark = selectedBookmark();

    if ( !bookmark.url().isEmpty() ) {
        m_edit->setInputValue( bookmark.url() );
        credentials = application->credentialsStore()->serverCredentials( bookmark.serverUuid() );
    }

    QAction* defaultAction = NULL;

    for ( int i = 0; i < 5; i++ ) {
        QAction* connectAction = action( QString( "openConnectionAs%1" ).arg( i + 1 ) );
        if ( i < credentials.count() ) {
            connectAction->setText( tr( "Connect As %1" ).arg( credentials.at( i ).userName() ) );
            connectAction->setData( QVariant::fromValue<ServerCredential>( credentials.at( i ) ) );
            connectAction->setVisible( true );
            if ( !defaultAction )
                defaultAction = connectAction;
        } else {
            connectAction->setVisible( false );
        }
    }

    QMenu* menu = builder()->contextMenu( "menuConnection" );
    if ( menu )
        menu->setDefaultAction( defaultAction ? defaultAction : action( "openConnection" ) );
}

Bookmark StartView::selectedBookmark()
{
    QList<QTreeWidgetItem*> items = m_list->selectedItems();
    if ( items.isEmpty() )
        return Bookmark();
    return items.first()->data( 0, Qt::UserRole ).value<Bookmark>();
}
