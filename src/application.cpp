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

#include "application.h"
#include "mainwindow.h"

#include "commands/commandmanager.h"
#include "data/localsettings.h"
#include "data/bookmarksstore.h"
#include "data/credentialsstore.h"
#include "data/datamanager.h"
#include "dialogs/aboutbox.h"
#include "utils/updateclient.h"
#include "utils/iconloader.h"
#include "views/viewmanager.h"

#if defined( HAVE_OPENSSL )
#include "data/certificatesstore.h"
#endif

#include <QSettings>
#include <QSessionManager>
#include <QMessageBox>
#include <QDir>
#include <QLocale>
#include <QTranslator>
#include <QLibraryInfo>
#include <QNetworkProxy>
#include <QDesktopServices>
#include <QNetworkAccessManager>
#include <QPushButton>

#if !defined( NO_DEFAULT_PROXY )
#include <QNetworkProxyFactory>
#endif

#if defined( Q_WS_WIN )
#define _WIN32_IE 0x0400
#include <shlobj.h>
#endif

#include <cstdlib>

Application* application = NULL;

#if defined( NO_DEFAULT_PROXY )

static QNetworkProxy networkProxy()
{
    LocalSettings* settings = application->applicationSettings();
    QNetworkProxy::ProxyType type = (QNetworkProxy::ProxyType)settings->value( "ProxyType" ).toInt();

    if ( type == QNetworkProxy::NoProxy || type == QNetworkProxy::DefaultProxy )
        return QNetworkProxy::NoProxy;

    QString hostName = settings->value( "ProxyHost" ).toString();
    quint16 port = (quint16)settings->value( "ProxyPort" ).toInt();
    return QNetworkProxy( type, hostName, port );
}

#else // defined( NO_DEFAULT_PROXY )

class NetworkProxyFactory : public QNetworkProxyFactory
{
public:
    NetworkProxyFactory()
    {
    }

    ~NetworkProxyFactory()
    {
    }

    QList<QNetworkProxy> queryProxy( const QNetworkProxyQuery& query )
    {
        LocalSettings* settings = application->applicationSettings();
        QNetworkProxy::ProxyType type = (QNetworkProxy::ProxyType)settings->value( "ProxyType" ).toInt();

        if ( type == QNetworkProxy::DefaultProxy )
            return QNetworkProxyFactory::systemProxyForQuery( query );

        if ( type == QNetworkProxy::NoProxy )
            return QList<QNetworkProxy>() << QNetworkProxy::NoProxy;

        QString hostName = settings->value( "ProxyHost" ).toString();
        quint16 port = (quint16)settings->value( "ProxyPort" ).toInt();
        return QList<QNetworkProxy>() << QNetworkProxy( type, hostName, port );
    }
};

#endif // defined( NO_DEFAULT_PROXY )

Application::Application( int& argc, char** argv ) : QApplication( argc, argv ),
    m_portable( false )
{
    Q_INIT_RESOURCE( icons );
    Q_INIT_RESOURCE( resources );

    initializeDefaultPaths();
    processArguments();

    application = this;

    m_settings = new LocalSettings( locateDataFile( "settings.dat" ), this );
    m_bookmarks = new BookmarksStore( locateDataFile( "bookmarks.dat" ), this );
    m_credentials = new CredentialsStore( locateDataFile( "credentials.dat" ), this );
#if defined( HAVE_OPENSSL )
    m_certificates = new CertificatesStore( locateDataFile( "certificates.crt" ), this );
#endif

    initializeSettings();

    QString language = m_settings->value( "Language" ).toString();
    if ( language.isEmpty() )
        language = QLocale::system().name();
    QLocale::setDefault( QLocale( language ) );

    loadTranslation( "qt", true );
    loadTranslation( "webissues", false );

#if defined( Q_WS_WIN )
    setStyle( "XmlUi::WindowsStyle" );
#endif

    setWindowIcon( IconLoader::icon( "webissues" ) );
    setQuitOnLastWindowClosed( false );

    viewManager = new ViewManager();

    m_mainWindow = new MainWindow();

    m_manager = new QNetworkAccessManager();

    m_updateClient = new UpdateClient( "webissues", version(), m_manager );

    connect( m_updateClient, SIGNAL( stateChanged() ), this, SLOT( showUpdateState() ) );

    settingsChanged();

    connect( m_settings, SIGNAL( settingsChanged() ), this, SLOT( settingsChanged() ) );

    restoreState();
}

Application::~Application()
{
    m_settings->setValue( "ShutdownVisible", m_mainWindow->isVisible() );
    m_settings->setValue( "ShutdownConnected", dataManager != NULL && dataManager->currentUserAccess() != NoAccess );

    delete m_updateSection;

    delete viewManager;
    viewManager = NULL;

    delete m_mainWindow;
    m_mainWindow = NULL;

    delete dataManager;
    dataManager = NULL;

    delete commandManager;
    commandManager = NULL;
 
    delete m_settings;
    m_settings = NULL;
}

void Application::commitData( QSessionManager& manager )
{
#if !defined( QT_NO_SESSIONMANAGER )
    if ( manager.allowsInteraction() ) {
        if ( viewManager && !viewManager->queryCloseViews() )
            manager.cancel();
    }
#endif
}

void Application::about()
{
    QString message;
    message += "<h3>" + tr( "WebIssues Desktop Client %1" ).arg( version() ) + "</h3>";
    message += "<p>" + tr( "Desktop Client for the WebIssues team collaboration system." ) + "</p>";
    message += "<p>" + tr( "This program is free software: you can redistribute it and/or modify"
        " it under the terms of the GNU General Public License as published by"
        " the Free Software Foundation, either version 3 of the License, or"
        " (at your option) any later version." ) + "</p>";
    message += "<p>" + trUtf8( "Copyright (C) 2006 Michał Męciński" ) + "<br>" + tr( "Copyright (C) 2007-2011 WebIssues Team" ) + "</p>";

    QString link = "<a href=\"http://webissues.mimec.org\">webissues.mimec.org</a>";

    QString helpMessage;
    helpMessage += "<h4>" + tr( "Help" ) + "</h4>";
    helpMessage += "<p>" + tr( "Open the WebIssues Manual for help." ) + "</p>";

    QString webMessage;
    webMessage += "<h4>" + tr( "Web Page" ) + "</h4>";
    webMessage += "<p>" + tr( "Visit %1 for more information about WebIssues." ).arg( link ) + "</p>";

    QString donateMessage;
    donateMessage += "<h4>" + tr( "Donations" ) + "</h4>";
    donateMessage += "<p>" + tr( "If you like this program, your donation will help us dedicate more time for it, support it and implement new features." ) + "</p>";

    QString updateMessage;
    updateMessage += "<h4>" + tr( "Latest Version" ) + "</h4>";
    updateMessage += "<p>" + tr( "Automatic checking for latest version is disabled. You can enable it in program settings." ) + "</p>";

    AboutBox aboutBox( tr( "About WebIssues" ), message, activeWindow() );

    AboutBoxSection* helpSection = aboutBox.addSection( IconLoader::pixmap( "help" ), helpMessage );

    QPushButton* helpButton = helpSection->addButton( tr( "&Manual" ) );
    connect( helpButton, SIGNAL( clicked() ), this, SLOT( openManual() ) );

    aboutBox.addSection( IconLoader::pixmap( "web" ), webMessage );

    AboutBoxSection* donateSection = aboutBox.addSection( IconLoader::pixmap( "donate" ), donateMessage );

    QPushButton* donateButton = donateSection->addButton( tr( "&Donate" ) );
    connect( donateButton, SIGNAL( clicked() ), this, SLOT( openDonations() ) );

    delete m_updateSection;

    m_updateSection = aboutBox.addSection( IconLoader::pixmap( "status-info" ), updateMessage );

    if ( m_updateClient->autoUpdate() ) {
        showUpdateState();
    } else {
        m_updateButton = m_updateSection->addButton( tr( "&Check Now" ) );
        connect( m_updateButton, SIGNAL( clicked() ), m_updateClient, SLOT( checkUpdate() ) );
    }

    aboutBox.exec();
}

void Application::showUpdateState()
{
    if ( !m_updateSection ) {
        if ( m_updateClient->state() != UpdateClient::UpdateAvailableState || m_updateClient->updateVersion() == m_shownVersion )
            return;

        m_updateSection = new AboutBoxToolSection();
    } else {
        m_updateSection->clearButtons();
    }

    QString header = "<h4>" + tr( "Latest Version" ) + "</h4>";

    switch ( m_updateClient->state() ) {
        case UpdateClient::CheckingState: {
            m_updateSection->setPixmap( IconLoader::pixmap( "status-info" ) );
            m_updateSection->setMessage( header + "<p>" + tr( "Checking for latest version..." ) + "</p>" );
            break;
        }

        case UpdateClient::ErrorState: {
            m_updateSection->setPixmap( IconLoader::pixmap( "status-warning" ) );
            m_updateSection->setMessage( header + "<p>" + tr( "Checking for latest version failed." ) + "</p>" );

            m_updateButton = m_updateSection->addButton( tr( "&Retry" ) );
            connect( m_updateButton, SIGNAL( clicked() ), m_updateClient, SLOT( checkUpdate() ) );
            break;
        }

        case UpdateClient::CurrentVersionState: {
            m_updateSection->setPixmap( IconLoader::pixmap( "status-info" ) );
            m_updateSection->setMessage( header + "<p>" + tr( "Your version of WebIssues Desktop Client is up to date." ) + "</p>" );
            break;
        }

        case UpdateClient::UpdateAvailableState: {
            m_updateSection->setPixmap( IconLoader::pixmap( "status-warning" ) );
            m_updateSection->setMessage( header + "<p>" + tr( "The latest version of WebIssues Desktop Client is %1." ).arg( m_updateClient->updateVersion() ) + "</p>" );

            QPushButton* notesButton = m_updateSection->addButton( tr( "&Release Notes" ) );
            connect( notesButton, SIGNAL( clicked() ), this, SLOT( openReleaseNotes() ) );

            QPushButton* downloadButton = m_updateSection->addButton( tr( "Do&wnload" ) );
            connect( downloadButton, SIGNAL( clicked() ), this, SLOT( openDownloads() ) );

            m_shownVersion = m_updateClient->updateVersion();
            break;
        }
    }
}

QUrl Application::manualIndex() const
{
    QString language = QLocale().name();

    while ( !language.isEmpty() ) {
        QString path = QString( "%1/%2/index.html" ).arg( m_manualPath, language );
        if ( QFile::exists( path ) )
            return QUrl::fromLocalFile( path );

        int pos = language.lastIndexOf( QLatin1Char( '_' ) );
        if ( pos < 0 )
            break;

        language = language.mid( 0, pos );
    }

    return QUrl::fromLocalFile( m_manualPath + "/en/index.html" );
}

void Application::openManual()
{
    QDesktopServices::openUrl( manualIndex() );
}

void Application::openDonations()
{
    QDesktopServices::openUrl( QUrl( "http://webissues.mimec.org/donations" ) );
}

void Application::openReleaseNotes()
{
    QDesktopServices::openUrl( m_updateClient->notesUrl() );
}

void Application::openDownloads()
{
    QDesktopServices::openUrl( m_updateClient->downloadUrl() );
}

QString Application::version() const
{
    return QString( "1.0-beta2" );
}

QString Application::protocolVersion() const
{
    return QString( "1.0-beta2" );
}

bool Application::loadTranslation( const QString& name, bool tryQtDir )
{
    QString fullName = name + "_" + QLocale().name();

    QTranslator* translator = new QTranslator( this );

    if ( translator->load( fullName, m_translationsPath ) ) {
        installTranslator( translator );
        return true;
    }

    if ( tryQtDir && translator->load( fullName, QLibraryInfo::location( QLibraryInfo::TranslationsPath ) ) ) {
        installTranslator( translator );
        return true;
    }

    delete translator;
    return false;
}

void Application::initializeDefaultPaths()
{
    // NOTE: update these paths after changing them in the project file

    QString appPath = applicationDirPath();

#if defined( Q_WS_WIN )
    m_manualPath = QDir::cleanPath( appPath + "/../doc" );
    m_translationsPath = QDir::cleanPath( appPath + "/../translations" );
#elif defined( Q_WS_MAC )
    m_manualPath = QDir::cleanPath( appPath + "/../Resources/doc" );
    m_translationsPath = QDir::cleanPath( appPath + "/../Resources/translations" );
#else
    m_manualPath = QDir::cleanPath( appPath + "/../share/doc/webissues/doc" );
    m_translationsPath = QDir::cleanPath( appPath + "/../share/webissues/translations" );
#endif

#if defined( Q_WS_WIN )
    wchar_t appDataPath[ MAX_PATH ];
    if ( SHGetSpecialFolderPath( 0, appDataPath, CSIDL_APPDATA, FALSE ) )
        m_dataPath = QDir::fromNativeSeparators( QString::fromWCharArray( appDataPath ) );
    else
        m_dataPath = QDir::homePath();

    m_dataPath += QLatin1String( "/WebIssues Client/1.0" );

    wchar_t localAppDataPath[ MAX_PATH ];
    if ( SHGetSpecialFolderPath( 0, localAppDataPath, CSIDL_LOCAL_APPDATA, FALSE ) )
        m_cachePath = QDir::fromNativeSeparators( QString::fromWCharArray( localAppDataPath ) );
    else
        m_cachePath = QDir::homePath();

    m_cachePath += QLatin1String( "/WebIssues Client/1.0/cache" );

    m_tempPath = QDir::tempPath() + "/WebIssues Client";
#else
    m_dataPath = QDesktopServices::storageLocation( QDesktopServices::DataLocation );
    m_dataPath += QLatin1String( "/webissues-1.0" );

    m_cachePath = QDesktopServices::storageLocation( QDesktopServices::CacheLocation );
    m_cachePath += QLatin1String( "/webissues-1.0" );

    m_tempPath = QDir::tempPath() + "/webissues";
#endif
}

void Application::processArguments()
{
    QStringList args = arguments();

    for ( int i = 1; i < args.count(); i++ ) {
        QString arg = args.at( i );
        if ( arg == QLatin1String( "-data" ) ) {
            if ( i + 1 < args.count() && !args.at( i + 1 ).startsWith( '-' ) ) {
                m_dataPath = QDir::fromNativeSeparators( args.at( ++i ) );
                m_portable = true;
            }
        } else if ( arg == QLatin1String( "-cache" ) ) {
            if ( i + 1 < args.count() && !args.at( i + 1 ).startsWith( '-' ) )
                m_cachePath = QDir::fromNativeSeparators( args.at( ++i ) );
        } else if ( arg == QLatin1String( "-temp" ) ) {
            if ( i + 1 < args.count() && !args.at( i + 1 ).startsWith( '-' ) )
                m_tempPath = QDir::fromNativeSeparators( args.at( ++i ) );
        }
    }
}

QString Application::locateDataFile( const QString& name )
{
    QString path = m_dataPath + '/' + name;

    checkAccess( path );

    return path;
}

QString Application::locateCacheFile( const QString& name )
{
    QString path = m_cachePath + '/' + name;

    checkAccess( path );

    return path;
}

QString Application::locateTempFile( const QString& name )
{
    QString path = m_tempPath + '/' + name;

    checkAccess( path );

    return path;
}

bool Application::checkAccess( const QString& path )
{
    QFileInfo fileInfo( path );
    if ( fileInfo.exists() )
        return fileInfo.isReadable();

    QDir dir = fileInfo.absoluteDir();
    if ( dir.exists() )
        return dir.isReadable();

    return dir.mkpath( dir.absolutePath() );
}

void Application::initializeSettings()
{
    if ( !m_settings->contains( "Docked" ) )
        m_settings->setValue( "Docked", false );
    if ( !m_settings->contains( "ShowAtStartup" ) )
        m_settings->setValue( "ShowAtStartup", (int)RestoreAlways );
    if ( !m_settings->contains( "ConnectAtStartup" ) )
        m_settings->setValue( "ConnectAtStartup", (int)RestoreNever );
    if ( !m_settings->contains( "UpdateInterval" ) )
        m_settings->setValue( "UpdateInterval", 5 );
    if ( !m_settings->contains( "DefaultAttachmentAction" ) )
        m_settings->setValue( "DefaultAttachmentAction", (int)ActionAsk );
    if ( !m_settings->contains( "AttachmentsCacheSize" ) )
        m_settings->setValue( "AttachmentsCacheSize", 10 );
    if ( !m_settings->contains( "AutoStart" ) )
        m_settings->setValue( "AutoStart", false );
    if ( !m_settings->contains( "AutoUpdate" ) )
        m_settings->setValue( "AutoUpdate", true );

    if ( !m_settings->contains( "ProxyType" ) ) {
#if defined( NO_DEFAULT_PROXY )
        m_settings->setValue( "ProxyType", (int)QNetworkProxy::NoProxy );
#else
        m_settings->setValue( "ProxyType", (int)QNetworkProxy::DefaultProxy );
#endif
    }
}

void Application::settingsChanged()
{
#if defined( NO_DEFAULT_PROXY )
    m_manager->setProxy( networkProxy() );
#else
    m_manager->setProxyFactory( new NetworkProxyFactory() );
#endif

    m_updateClient->setAutoUpdate( m_settings->value( "AutoUpdate" ).toBool() );

#if defined( Q_WS_WIN )
    if ( m_portable )
        return;

    bool autoStart = m_settings->value( "AutoStart" ).toBool();

    QSettings runKey( "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat );
    if ( autoStart )
        runKey.setValue( "WebIssues", '"' + QDir::toNativeSeparators( QApplication::applicationFilePath() ) + '"' );
    else
        runKey.remove( "WebIssues" );
#endif
}

void Application::restoreState()
{
    bool docked = m_settings->value( "Docked" ).toBool();
    RestoreOption show = (RestoreOption)m_settings->value( "ShowAtStartup" ).toInt();
    RestoreOption connect = (RestoreOption)m_settings->value( "ConnectAtStartup" ).toInt();

    bool wasVisible = m_settings->value( "ShutdownVisible", true ).toBool();
    bool wasConnected = m_settings->value( "ShutdownConnected", false ).toBool();

    if ( !docked || show == RestoreAlways || show == RestoreAuto && wasVisible )
        m_mainWindow->show();

    if ( connect == RestoreAlways || connect == RestoreAuto && wasConnected )
        m_mainWindow->reconnect();

    if ( m_settings->value( "LastVersion" ).toString() != version() ) {
        m_settings->setValue( "LastVersion", version() );
        about();
    }
}
