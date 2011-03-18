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

class MainWindow;
class LocalSettings;
class BookmarksStore;
class CredentialsStore;

#if defined( HAVE_OPENSSL )
class CertificatesStore;
#endif

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

public slots:
    /**
    * Show the About WebIssues dialog.
    */
    void about();

public: // overrides
    void commitData( QSessionManager& manager );

private slots:
    void settingsChanged();

private:
    bool loadTranslation( const QString& name, bool tryQtDir );

    void initializeDefaultPaths();
    void processArguments();

    bool checkAccess( const QString& path );

    void initializeSettings();
    void restoreState();

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
};

/**
* Global pointer used to access the Application.
*/
extern Application* application;

#endif
