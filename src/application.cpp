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

#include "application.h"
#include "mainwindow.h"

#include "commands/commandmanager.h"
#include "data/localsettings.h"
#include "data/bookmarksstore.h"
#include "data/credentialsstore.h"
#include "data/certificatesstore.h"
#include "data/datamanager.h"
#include "dialogs/dialogmanager.h"
#include "dialogs/aboutbox.h"
#include "utils/networkproxyfactory.h"
#include "utils/updateclient.h"
#include "utils/inifile.h"
#include "utils/iconloader.h"
#include "views/viewmanager.h"

#include <QSessionManager>
#include <QMessageBox>
#include <QDir>
#include <QLocale>
#include <QTranslator>
#include <QLibraryInfo>
#include <QDesktopServices>
#include <QNetworkAccessManager>
#include <QPushButton>
#include <QPrinter>
#include <QTimer>
#include <QSettings>

#if defined( Q_WS_WIN )
#define _WIN32_IE 0x0400
#include <shlobj.h>
#endif

#include <cstdlib>

Application* application = NULL;

Application::Application( int& argc, char** argv ) : QApplication( argc, argv ),
    m_portable( false ),
    m_printer( NULL )
{
    Q_INIT_RESOURCE( icons );
    Q_INIT_RESOURCE( resources );

    initializeDefaultPaths();
    processArguments();

    application = this;

    m_settings = new LocalSettings( locateDataFile( "settings.dat" ), this );
    m_bookmarks = new BookmarksStore( locateDataFile( "bookmarks.dat" ), this );
    m_credentials = new CredentialsStore( locateDataFile( "credentials.dat" ), this );
#if !defined( QT_NO_OPENSSL )
    m_certificates = new CertificatesStore( locateDataFile( "certificates.crt" ), this );
#endif

    initializeSettings();
    initializeLanguage();

#if defined( Q_WS_WIN )
    setStyle( "XmlUi::WindowsStyle" );
#elif defined( Q_WS_MAC )
    setStyle( "XmlUi::MacStyle" );
#endif

    setWindowIcon( IconLoader::icon( "webissues" ) );
    setQuitOnLastWindowClosed( false );

    viewManager = new ViewManager();
    dialogManager = new DialogManager();

    m_mainWindow = new MainWindow();

    m_editorFont.setFamily( "Verdana, DejaVu Sans" );
    m_editorFont.setPointSizeF( font().pointSizeF() );
    m_editorFont.setStyleHint( QFont::SansSerif );

    m_manager = new QNetworkAccessManager();

    m_manager->setProxyFactory( new NetworkProxyFactory() );

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

    delete dialogManager;
    dialogManager = NULL;

    delete m_mainWindow;
    m_mainWindow = NULL;

    delete dataManager;
    dataManager = NULL;

    delete commandManager;
    commandManager = NULL;
 
    delete m_settings;
    m_settings = NULL;

    delete m_printer;
    m_printer = NULL;

    delete m_localeIni;
    m_localeIni = NULL;

    delete m_formatsIni;
    m_formatsIni = NULL;
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
    message += "<p>" + trUtf8( "Copyright &copy; 2006 Michał Męciński" ) + "<br>" + tr( "Copyright &copy; 2007-2013 WebIssues Team" ) + "</p>";

    QString link = "<a href=\"http://webissues.mimec.org\">webissues.mimec.org</a>";

    QString helpMessage;
    helpMessage += "<h4>" + tr( "Help" ) + "</h4>";
    helpMessage += "<p>" + tr( "Open the WebIssues Manual for help." ) + "</p>";

    QString webMessage;
    webMessage += "<h4>" + tr( "Website" ) + "</h4>";
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
            m_updateSection->setMessage( header + "<p>" + tr( "Your version of WebIssues is up to date." ) + "</p>" );
            break;
        }

        case UpdateClient::UpdateAvailableState: {
            m_updateSection->setPixmap( IconLoader::pixmap( "status-warning" ) );
            m_updateSection->setMessage( header + "<p>" + tr( "The latest version of WebIssues is %1." ).arg( m_updateClient->updateVersion() ) + "</p>" );

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
    QString language = m_language;

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
    return QString( "1.1" );
}

QString Application::protocolVersion() const
{
    return QString( "1.1" );
}

void Application::initializeLanguage()
{
    m_localeIni = new IniFile( ":/resources/locale.ini" );
    m_localeIni->load( m_translationsPath + "/locale.ini" );

    m_formatsIni = new IniFile( ":/resources/formats.ini" );

    QMap<QString, QString> availableLanguages = languages();

    QString language = m_settings->value( "Language" ).toString();
    if ( language.isEmpty() )
        language = QLocale::system().name();

    m_language = "en_US";

    while ( !language.isEmpty() ) {
        if ( availableLanguages.contains( language ) ) {
            m_language = language;
            break;
        }

        int pos = language.lastIndexOf( QLatin1Char( '_' ) );
        if ( pos < 0 )
            break;

        language = language.mid( 0, pos );
    }

    QLocale::setDefault( QLocale( m_language ) );

    loadTranslation( "qt", true );
    loadTranslation( "webissues", false );
}

bool Application::loadTranslation( const QString& name, bool tryQtDir )
{
    QString fullName = name + "_" + m_language;

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

QMap<QString, QString> Application::languages() const
{
    return m_localeIni->values( "languages" );
}

QString Application::locale( const QString& key ) const
{
    QString value = m_localeIni->value( m_language, key );
    if ( value.isEmpty() )
        value = m_localeIni->value( "global", key );
    return value;
}

QMap<QString, QString> Application::formats( const QString& type ) const
{
    return m_formatsIni->values( type );
}

QString Application::format( const QString& type, const QString& key ) const
{
    return m_formatsIni->value( type, key );
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

    QString dataPath;
    QString cachePath;

#if defined( Q_WS_WIN )
    wchar_t appDataPath[ MAX_PATH ];
    if ( SHGetSpecialFolderPath( 0, appDataPath, CSIDL_APPDATA, FALSE ) )
        dataPath = QDir::fromNativeSeparators( QString::fromWCharArray( appDataPath ) );
    else
        dataPath = QDir::homePath();

    m_dataPath = dataPath + QLatin1String( "/WebIssues Client/1.1" );
    m_oldDataPath = dataPath + QLatin1String( "/WebIssues Client/1.0" );

    wchar_t localAppDataPath[ MAX_PATH ];
    if ( SHGetSpecialFolderPath( 0, localAppDataPath, CSIDL_LOCAL_APPDATA, FALSE ) )
        cachePath = QDir::fromNativeSeparators( QString::fromWCharArray( localAppDataPath ) );
    else
        cachePath = QDir::homePath();

    m_cachePath = cachePath + QLatin1String( "/WebIssues Client/1.1/cache" );
    m_sharedCachePath = cachePath + QLatin1String( "/WebIssues Client/shared/cache" );
#else
    dataPath = QDesktopServices::storageLocation( QDesktopServices::DataLocation );
    m_dataPath = dataPath + QLatin1String( "/webissues-1.1" );
    m_oldDataPath = dataPath + QLatin1String( "/webissues-1.0" );

    cachePath = QDesktopServices::storageLocation( QDesktopServices::CacheLocation );
    m_cachePath = cachePath + QLatin1String( "/webissues-1.1" );
    m_sharedCachePath = cachePath + QLatin1String( "/webissues-shared" );
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
                m_oldDataPath.clear();
                m_portable = true;
            }
        } else if ( arg == QLatin1String( "-cache" ) ) {
            if ( i + 1 < args.count() && !args.at( i + 1 ).startsWith( '-' ) )
                m_cachePath = QDir::fromNativeSeparators( args.at( ++i ) );
        } else if ( arg == QLatin1String( "-shared" ) ) {
            if ( i + 1 < args.count() && !args.at( i + 1 ).startsWith( '-' ) )
                m_sharedCachePath = QDir::fromNativeSeparators( args.at( ++i ) );
        }
    }
}

QString Application::locateDataFile( const QString& name )
{
    QString path = m_dataPath + '/' + name;

    if ( checkAccess( path ) ) {
        if ( !m_oldDataPath.isEmpty() && !QFile::exists( path ) && QFile::exists( m_oldDataPath + '/' + name ) )
            QFile::copy( m_oldDataPath + '/' + name, path );
    }

    return path;
}

QString Application::locateCacheFile( const QString& name )
{
    QString path = m_cachePath + '/' + name;

    checkAccess( path );

    return path;
}

QString Application::locateSharedCacheFile( const QString& name )
{
    QString path = m_sharedCachePath + '/' + name;

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
    struct
    {
        QString m_key;
        QVariant m_value;
    }
    defaults[] = 
    {
        { "Docked", false },
        { "ShowAtStartup", (int)RestoreAlways },
        { "ConnectAtStartup", (int)RestoreNever },
        { "AutoStart", false },
        { "AutoUpdate", true },
        { "DefaultAttachmentAction", (int)ActionAsk },
        { "FolderUpdateInterval", 1 },
        { "UpdateInterval", 5 },
        { "ProxyType", (int)QNetworkProxy::NoProxy },
    };

    for ( int i = 0; i < (int)( sizeof( defaults ) / sizeof( defaults[ 0 ] ) ); i++ ) {
        if ( !m_settings->contains( defaults[ i ].m_key ) )
            m_settings->setValue( defaults[ i ].m_key, defaults[ i ].m_value );
    }
}

void Application::settingsChanged()
{
    m_updateClient->setAutoUpdate( m_settings->value( "AutoUpdate" ).toBool() );

#if defined( Q_WS_WIN )
    if ( !m_portable ) {
        bool autoStart = m_settings->value( "AutoStart" ).toBool();

        QSettings runKey( "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat );
        if ( autoStart )
            runKey.setValue( "WebIssues", '"' + QDir::toNativeSeparators( QApplication::applicationFilePath() ) + '"' );
        else
            runKey.remove( "WebIssues" );
    }
#endif
}

void Application::restoreState()
{
    bool docked = m_settings->value( "Docked" ).toBool();
    RestoreOption show = (RestoreOption)m_settings->value( "ShowAtStartup" ).toInt();
    RestoreOption connect = (RestoreOption)m_settings->value( "ConnectAtStartup" ).toInt();

    bool wasVisible = m_settings->value( "ShutdownVisible", true ).toBool();
    bool wasConnected = m_settings->value( "ShutdownConnected", false ).toBool();

    if ( !docked || show == RestoreAlways || ( show == RestoreAuto && wasVisible ) )
        m_mainWindow->show();

    if ( connect == RestoreAlways || ( connect == RestoreAuto && wasConnected ) )
        m_mainWindow->reconnect();

    if ( m_settings->value( "LastVersion" ).toString() != version() ) {
        m_settings->setValue( "LastVersion", version() );
        QTimer::singleShot( 100, this, SLOT( about() ) );
    }
}

double Application::textSizeMultiplier() const
{
    return QFontMetrics( m_editorFont ).height() / 13.0;
}

QPrinter* Application::printer()
{
    if ( !m_printer ) {
        m_printer = new QPrinter( QPrinter::HighResolution );
        m_printer->setOutputFormat( QPrinter::NativeFormat );
    }
    return m_printer;
}

void Application::openUrl( QWidget* parent, const QUrl& url )
{
#if defined( Q_WS_WIN )
    if ( url.isValid() && url.scheme().toLower() == QLatin1String( "file" ) ) {
        QString path = url.path();
        if ( path.startsWith( QLatin1Char( '/' ) ) )
            path = path.mid( 1 );
        path = QDir::toNativeSeparators( path );
        QString host = url.host();
        if ( !host.isEmpty() )
            path = QLatin1String( "\\\\" ) + host + QLatin1String( "\\" ) + path;
        if ( !path.isEmpty() )
            ShellExecute( parent->effectiveWinId(), NULL, (LPCTSTR)path.utf16(), NULL, NULL, SW_NORMAL );
    } else
#else
    Q_UNUSED( parent )
#endif
    QDesktopServices::openUrl( url );
}
