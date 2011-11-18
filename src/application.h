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

#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#include <QPointer>
#include <QMap>

class MainWindow;
class LocalSettings;
class BookmarksStore;
class CredentialsStore;
class UpdateClient;
class AboutBoxSection;

#if defined( HAVE_OPENSSL )
class CertificatesStore;
#endif

class QNetworkAccessManager;
class QUrl;
class QPushButton;
class QPrinter;

/**
* Class representing the WebIssues application.
*
* On startup the application creates the ConfigData
* and ConnectionManager objects, opens the MainWindow
* and restores the session or the last connection if necessary.
*
* On shutdown it destroys all objects.
*/
class Application : public QApplication
{
    Q_OBJECT
public:
    /**
    * Constructor.
    */
    Application( int& argc, char** argv );

    /**
    * Destructor.
    */
    ~Application();

public:
    /**
    * Return the version of the application.
    */
    QString version() const;

    /**
    * Return the required version of the WebIssues protocol.
    */
    QString protocolVersion() const;

    /**
    * Return the path of the translations directory.
    */
    QString translationsPath() const { return m_translationsPath; }

    /**
    * Return the path of a data file.
    * @param name The name of the data file.
    */
    QString locateDataFile( const QString& name );

    /**
    * Return the path of a cache file.
    * @param name The name of the cache file.
    */
    QString locateCacheFile( const QString& name );

    /**
    * Return the path of a temporary file.
    * @param name The name of the temporary file.
    */
    QString locateTempFile( const QString& name );

    /**
    * Return @c true if application was started in portable mode.
    */
    bool isPortableMode() const { return m_portable; }

    /**
    * Return application settings.
    */
    LocalSettings* applicationSettings() const { return m_settings; }

    /**
    * Return the bookmarks store.
    */
    BookmarksStore* bookmarksStore() const { return m_bookmarks; }

    /**
    * Return the credentials store.
    */
    CredentialsStore* credentialsStore() const { return m_credentials; }

#if defined( HAVE_OPENSSL )
    /**
    * Return the certificates store.
    */
    CertificatesStore* certificatesStore() const { return m_certificates; }
#endif

    /**
    * Return the list of available languages.
    */
    const QMap<QString, QString>& languages() const { return m_languages; }

    /**
    * Return the current language.
    */
    const QString& language() const { return m_language; }

    /**
    * Return the network access manager.
    */
    QNetworkAccessManager* networkAccessManager() const { return m_manager; }

    /**
    * Return the printer.
    */
    QPrinter* printer();

public slots:
    /**
    * Show the About WebIssues dialog.
    */
    void about();

public: // overrides
    void commitData( QSessionManager& manager );

private slots:
    void openManual();

    void showUpdateState();

    void openDonations();
    void openReleaseNotes();
    void openDownloads();

    void settingsChanged();

private:
    bool loadTranslation( const QString& name, bool tryQtDir );

    void initializeDefaultPaths();
    void processArguments();

    bool checkAccess( const QString& path );

    void initializeSettings();
    void restoreState();

    void initializeLanguage();

    QUrl manualIndex() const;

private:
    MainWindow* m_mainWindow;

    QString m_manualPath;
    QString m_translationsPath;

    QString m_dataPath;
    QString m_cachePath;
    QString m_tempPath;

    bool m_portable;

    LocalSettings* m_settings;
    BookmarksStore* m_bookmarks;
    CredentialsStore* m_credentials;
#if defined( HAVE_OPENSSL )
    CertificatesStore* m_certificates;
#endif

    QMap<QString, QString> m_languages;
    QString m_language;

    QNetworkAccessManager* m_manager;

    UpdateClient* m_updateClient;

    QPointer<AboutBoxSection> m_updateSection;
    QPointer<QPushButton> m_updateButton;

    QString m_shownVersion;

    QPrinter* m_printer;
};

/**
* Global pointer used to access the Application.
*/
extern Application* application;

#endif
