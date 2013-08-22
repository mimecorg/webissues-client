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

#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include "data/updateevent.h"
#include "utils/definitioninfo.h"

#include <QObject>
#include <QHash>

class Command;
class Reply;
class LocalSettings;
class IssueTypeCache;
class FileCache;

class QSqlDatabase;

/**
* Access level for user or member.
*/
enum Access
{
    /**
    * Data is unavailable and the access level is not known.
    */
    UnknownAccess = -1,
    /**
    * User has no access to the system or project.
    */
    NoAccess = 0,
    /**
    * User has normal access to the system or project.
    */
    NormalAccess = 1,
    /**
    * User is a system or project administrator.
    */
    AdminAccess = 2
};

/**
* Type of a change.
*/
enum ChangeType
{
    /**
    * The issue was created with the given name.
    */
    IssueCreated = 0,
    /**
    * The issue was renamed.
    */
    IssueRenamed = 1,
    /**
    * The value of an attribute was changed.
    */
    ValueChanged = 2,
    /**
    * A comment was added.
    */
    CommentAdded = 3,
    /**
    * A file was added.
    */
    FileAdded = 4,
    /**
    * The issue was moved to another folder.
    */
    IssueMoved = 5
};

/**
* Email setting for an alert.
*/
enum AlertEmail
{
    /**
    * No emails are sent for the alert.
    */
    NoEmail = 0,
    /**
    * Immediate notifications are sent for the alert.
    */
    ImmediateNotificationEmail = 1,
    /**
    * Summary notifications are sent for the alert.
    */
    SummaryNotificationEmail = 2,
    /**
    * Summary reports are sent for the alert.
    */
    SummaryReportEmail = 3
};

/**
* Format of comment or description.
*/
enum TextFormat
{
    /**
    * Plain text format.
    */
    PlainText = 0,
    /**
    * Text with markup.
    */
    TextWithMarkup = 1
};

/**
* Class for storing data retrieved from the Webissues server.
*
* This class stores data tables which reflect the structure of the server's database.
* It also handles commands for updating the data and notifies about data updates.
*
* Each command updates a logical unit of data. The three global units are users with
* their membership, types with their attributes and projects with folders. In addition,
* each folder unit contains issues from that folder and their attribute values.
* Each issue unit contains a signle issue and its attributes, comments, files
* and history of changes.
*
* Each unit of data is updated independently. The folder and issue units are updated
* incrementally, i.e. only the changes since the last update are retrieved.
*
* Only a limited number of issue units is stored. The least recently used issues are
* removed from memory when the limit is exceeded. To prevent an issue from being removed
* you have to lock it and unlock it when you no longer need it.
*
* All data is cached in a SQLite database.
*
* The instance of this class is available using the dataManager global variable.
* It is created and owned by the ConnectionManager.
*/
class DataManager : public QObject
{
    Q_OBJECT
public:
    /**
    * Default constructor.
    */
    DataManager();

    /**
    * Destructor.
    */
    ~DataManager();

public:
    /**
    * Return @c true if the data is valid.
    */
    bool isValid() const { return m_valid; }

    /**
    * Return the name of the server.
    */
    const QString& serverName() const { return m_serverName; }

    /**
    * Return the unique identifier of the server.
    */
    const QString& serverUuid() const { return m_serverUuid; }

    /**
    * Return the version of the server.
    */
    const QString& serverVersion() const { return m_serverVersion; }

    /**
    * Check if the server is at least the given version.
    * This function ignores the part after the dash (for example beta or RC version).
    */
    bool checkServerVersion( const QString& version ) const;

    /**
    * Return the identifier of the current user.
    */
    int currentUserId() const { return m_currentUserId; }

    /**
    * Return the login of the current user.
    */
    const QString& currentUserLogin() const { return m_currentUserLogin; }

    /**
    * Return the name of the current user.
    */
    const QString& currentUserName() const { return m_currentUserName; }

    /**
    * Return the access level of the current user.
    */
    Access currentUserAccess() const { return m_currentUserAccess; }

    /**
    * Return the local settings for the connection.
    */
    LocalSettings* connectionSettings() const { return m_connectionSettings; }

    /**
    * Return the value of a server setting.
    * @param key The key of the setting.
    * @return The value of the setting or an empty string if it doesn't exist.
    */
    QString setting( const QString& key ) const;

    /**
    * Return the value of a user preference or server settings.
    * @param key The key of the setting.
    * @return The value of the setting or an empty string if it doesn't exist.
    */
    QString preferenceOrSetting( const QString& key ) const;

    /**
    * Return the value of a user preference, server setting or locale setting.
    * @param key The key of the setting.
    * @return The value of the setting.
    */
    QString localeSetting( const QString& key ) const;

    /**
    * Return the definition of the number format.
    */
    const DefinitionInfo& numberFormat() const { return m_numberFormat; }

    /**
    * Return the definition of the number format.
    */
    const DefinitionInfo& dateFormat() const { return m_dateFormat; }

    /**
    * Return the definition of the number format.
    */
    const DefinitionInfo& timeFormat() const { return m_timeFormat; }

    /**
    * Get the cached information related to an issue type.
    * @param Identifier of the issue type.
    * @return The cached issue type information.
    */
    IssueTypeCache* issueTypeCache( int typeId );

    /**
    * Return the path of a data file.
    * The data file is unique to each server.
    * @param name The name of the data file.
    */
    QString locateDataFile( const QString& name );

    /**
    * Return the path of a cache file.
    * The cache file is unique to each server.
    * @param name The name of the cache file.
    */
    QString locateCacheFile( const QString& name );

    /**
    * Add a data observer.
    * The observer receives UpdateEvent events when a unit of data is updated.
    */
    void addObserver( QObject* observer );

    /**
    * Remove the data observer.
    */
    void removeObserver( QObject* observer );

    /**
    * Check if the project summary needs updating.
    * @param projectId Identifier of the project.
    * @return @c True if the project summary needs updating.
    */
    bool summaryUpdateNeeded( int projectId ) const;

    /**
    * Check if the folder needs updating.
    * This method compares the stamp of the last modification of the folder with
    * the stamp of the last update of its issues.
    * @param folderId Identifier of the folder.
    * @return @c True if the folder needs updating.
    */
    bool folderUpdateNeeded( int folderId ) const;

    /**
    * Check if the issue needs updating.
    * This method compares the stamp of the last modification of the issue with
    * the stamp of the last update of its details.
    * @param issueId Identifier of the issue.
    * @return @c true if the issue needs updating.
    */
    bool issueUpdateNeeded( int issueId ) const;

    /**
    * Create a HELLO command.
    */
    Command* hello();

    /**
    * Create a LOGIN command.
    * @param login The user's login.
    * @param password The user's password.
    */
    Command* login( const QString& login, const QString& password );

    /**
    * Create a LOGIN NEW command.
    * @param login The user's login.
    * @param password The user's password.
    * @param newPassword The new password to set.
    */
    Command* loginNew( const QString& login, const QString& password, const QString& newPassword );

    /**
    * Create a command for retrieving settings from the server.
    */
    Command* updateSettings();

    /**
    * Create a command for updating users and their membership.
    */
    Command* updateUsers();

    /**
    * Create a command for updating types and attributes.
    */
    Command* updateTypes();

    /**
    * Create a command for updating projects and folders.
    */
    Command* updateProjects();

    /**
    * Create a command for updating the state of issues.
    */
    Command* updateStates();

    /**
    * Create a command for updating the given project summary.
    */
    Command* updateSummary( int projectId );

    /**
    * Create a command for updating the given folder.
    */
    Command* updateFolder( int folderId );

    /**
    * Create a command for updating the given issue.
    */
    Command* updateIssue( int issueId, bool markAsRead );

    /**
    * Lock the issue to prevent removing it from the cache.
    * @param issueId Identifier of the issue to lock.
    */
    void lockIssue( int issueId );

    /**
    * Release a previous issue lock.
    * When the lock count of the issue is zero, it can be removed from cache.
    * @param issueId Identifier of the issue to unlock.
    */
    void unlockIssue( int issueId );

    /**
    * Locate a file in the cache.
    * @param fileId Identifier of the file.
    * @return Path of the file or empty string if it is not cached.
    */
    QString findFilePath( int fileId ) const;

    /**
    * Generate a unique path in the cache (the file is not created).
    * @param fileId Identifier of the file.
    * @return Path of the new file in the cache.
    */
    QString generateFilePath( const QString& name ) const;

    /**
    * Allocate space in the cache deleting old files if necessary.
    * @param size Amount of space (in bytes) to allocate.
    */
    void allocFileSpace( int size );

    /**
    * Add the file to the cache.
    * @param fileId Identifier of the file.
    * @parm path Path of the file in the cache.
    * @param size Size of the file in bytes.
    */
    void commitFile( int fileId, const QString& path, int size );

private slots:
    void helloReply( const Reply& reply );
    void loginReply( const Reply& reply );
    void updateSettingsReply( const Reply& reply );
    void updateUsersReply( const Reply& reply );
    void updateTypesReply( const Reply& reply );
    void updateProjectsReply( const Reply& reply );
    void updateStatesReply( const Reply& reply );
    void updateSummaryReply( const Reply& reply );
    void updateFolderReply( const Reply& reply );
    void updateIssueReply( const Reply& reply );

private:
    void notifyObservers( UpdateEvent::Unit unit, int id = 0 );

    bool openDatabase();
    void closeDatabase();

    bool lockDatabase( const QSqlDatabase& database );
    bool installSchema( QSqlDatabase& database );

    bool updateSettingsReply( const Reply& reply, const QSqlDatabase& database );
    bool updateUsersReply( const Reply& reply, const QSqlDatabase& database );
    bool updateTypesReply( const Reply& reply, const QSqlDatabase& database );
    bool updateProjectsReply( const Reply& reply, const QSqlDatabase& database );
    bool updateStatesReply( const Reply& reply, int lastStateId, const QSqlDatabase& database );
    bool updateSummaryReply( const Reply& reply, const QSqlDatabase& database, int& projectId );
    bool updateFolderReply( const Reply& reply, const QSqlDatabase& database, QList<int>& updatedFolders );
    bool updateIssueReply( const Reply& reply, const QSqlDatabase& database, QList<int>& updatedFolders, int& issueId );

    bool lockIssue( int issueId, const QSqlDatabase& database );
    bool unlockIssue( int issueId, const QSqlDatabase& database );

    void clearIssueLocks();
    bool clearIssueLocks( const QSqlDatabase& database );

    void flushIssueDetails();
    bool flushIssueDetails( const QSqlDatabase& database );
    bool removeIssueDetails( const QList<int>& issues, const QSqlDatabase& database );

    bool recalculateAllAlerts( const QSqlDatabase& database );
    bool recalculateAlerts( int folderId, const QSqlDatabase& database );
    bool recalculateGlobalAlerts( int typeId, const QSqlDatabase& database );
    bool recalculateAlert( int alertId, int folderId, int typeId, int viewId, const QSqlDatabase& database );

    void recalculateSettings();
    bool recalculateSettings( const QSqlDatabase& database );

private:
    bool m_valid;

    QString m_serverName;
    QString m_serverUuid;
    QString m_serverVersion;

    int m_currentUserId;
    QString m_currentUserLogin;
    QString m_currentUserName;
    Access m_currentUserAccess;

    LocalSettings* m_connectionSettings;

    QMap<QString, QString> m_preferences;
    QMap<QString, QString> m_settings;

    QHash<int, IssueTypeCache*> m_issueTypesCache;

    FileCache* m_fileCache;

    DefinitionInfo m_numberFormat;
    DefinitionInfo m_dateFormat;
    DefinitionInfo m_timeFormat;

    QList<QObject*> m_observers;
};

/**
* Global pointer used to access the DataManager.
*/
extern DataManager* dataManager;

#endif
