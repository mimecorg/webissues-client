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

#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include "datarows.h"
#include "staterows.h"
#include "updateevent.h"

#include "rdb/rdb.h"

#include <QObject>
#include <QPair>
#include <QMap>

class Command;
class Reply;
class ReplyLine;
class LocaleCache;
class AttachmentsCache;
class LocalSettings;

/**
* A table of users.
*/
typedef RDB::SimpleTable<UserRow> UsersTable;

/**
* A table of project members.
*/
typedef RDB::CrossTable<MemberRow> MembersTable;

/**
* A table of issue types.
*/
typedef RDB::SimpleTable<TypeRow> TypesTable;

/**
* A table of attribute definitions.
*/
typedef RDB::ChildTable<AttributeRow> AttributesTable;

/**
* A table of projects.
*/
typedef RDB::SimpleTable<ProjectRow> ProjectsTable;

/**
* A table of folders.
*/
typedef RDB::ChildTable<FolderRow> FoldersTable;

/**
* A table of issues.
*/
typedef RDB::ChildTable<IssueRow> IssuesTable;

/**
* A table of attribute values.
*/
typedef RDB::CrossTable<ValueRow> ValuesTable;

/**
* A table of value changes.
*/
typedef RDB::ChildTable<ChangeRow> ChangesTable;

/**
* A table of comments.
*/
typedef RDB::SimpleTable<CommentRow> CommentsTable;

/**
* A table of files.
*/
typedef RDB::SimpleTable<FileRow> FilesTable;

/**
* A table of views.
*/
typedef RDB::ChildTable<ViewRow> ViewsTable;

/**
* A table of alerts.
*/
typedef RDB::ChildTable<AlertRow> AlertsTable;

/**
* An internal table storing the state of folders.
*/
typedef RDB::SimpleTable<FolderState> FolderStatesTable;

/**
* An internal table storing the state of issues.
*/
typedef RDB::SimpleTable<IssueState> IssueStatesTable;

/**
* An internal table storing the state of alerts.
*/
typedef RDB::SimpleTable<AlertState> AlertStatesTable;

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
* Folder units are cached in data files. When the folder is updated for the first time,
* its previous contents is automatically read from the cache and only the changed issues
* are retrieved from the server.
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
    * Return the name of the server.
    */
    const QString& serverName() const { return m_serverName; }

    /**
    * Return the unique identifier of the server.
    */
    const QString& serverUuid() const { return m_serverUuid; }

    /**
    * Return the version of the server.
    * Before version 0.8.3 this string is empty.
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
    * Return the table of users.
    */
    const UsersTable* users() const { return &m_users; }

    /**
    * Return the table of project members.
    */
    const MembersTable* members() const { return &m_members; }

    /**
    * Return the table of issue types.
    */
    const TypesTable* types() const { return &m_types; }

    /**
    * Return the table of attribute definitions.
    */
    const AttributesTable* attributes() const { return &m_attributes; }

    /**
    * Return the table of projects.
    */
    const ProjectsTable* projects() const { return &m_projects; }

    /**
    * Return the table of folders.
    */
    const FoldersTable* folders() const { return &m_folders; }

    /**
    * Return the table of issues.
    */
    const IssuesTable* issues() const { return &m_issues; }

    /**
    * Return the table of attribute values.
    */
    const ValuesTable* values() const { return &m_values; }

    /**
    * Return the table of comments.
    */
    const CommentsTable* comments() const { return &m_comments; }

    /**
    * Return the table of files.
    */
    const FilesTable* files() const { return &m_files; }

    /**
    * Return the table of value changes.
    */
    const ChangesTable* changes() const { return &m_changes; }

    /**
    * Return the table of views.
    */
    const ViewsTable* views() const { return &m_views; }

    /**
    * Return the table of alerts.
    */
    const AlertsTable* alerts() const { return &m_alerts; }

    /**
    * Return the locale cache.
    */
    LocaleCache* localeCache() const { return m_localeCache; }

    /**
    * Return the attachments cache.
    */
    AttachmentsCache* attachmentsCache() const { return m_attachmentsCache; }

    /**
    * Return the local settings for the connection.
    */
    LocalSettings* connectionSettings() const { return m_connectionSettings; }

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
    * Check if the folder needs updating.
    * This method compares the stamp of the last modification of the folder with
    * the stamp of the last update of its issues.
    * @param folderId Identifier of the folder.
    * @return @c True if the folder needs updating.
    */
    bool folderUpdateNeeded( int folderId );

    /**
    * Check if the issue needs updating.
    * This method compares the stamp of the last modification of the issue with
    * the stamp of the last update of its details.
    * @param issueId Identifier of the issue.
    * @return @c true if the issue needs updating.
    */
    bool issueUpdateNeeded( int issueId );

    /**
    * Lock the issue to prevent removing it from the cache.
    * @param issueId Identifier of the issue to lock.
    */
    void lockIssue( int issueId );

    /**
    * Release a previous issue lock.
    * When the lock count of the issue is zero, it can be removed from memory.
    * @param issueId Identifier of the issue to unlock.
    */
    void unlockIssue( int issueId );

    /**
    * Return the stamp of the issue when it was last read.
    * @param issueId Identifier of the issue.
    * @return The stamp of the issue.
    */
    int issueReadStamp( int issueId );

    /**
    * Return the statistics of the given alert.
    * @param alertId Identifier of the alert.
    * @param unread Pointer to variable which receives number of new issues.
    * @param unread Pointer to variable which receives number of modified issues.
    * @param unread Pointer to variable which receives total number of issues.
    */
    void getAlertIssuesCount( int alertId, int* unread, int* modified, int* total );

    /**
    * Return the value of a server setting.
    * @param key The key of the setting.
    * @return The value of the setting or an empty string if it doesn't exist.
    */
    QString setting( const QString& key ) const;

    /**
    * Return the value of a view setting.
    * @param int Identifier of the issue type.
    * @param key The key of the setting.
    * @return The value of the setting or an empty string if it doesn't exist.
    */
    QString viewSetting( int typeId, const QString& key ) const;

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
    * Create a command for updating the given folder.
    */
    Command* updateFolder( int folderId );

    /**
    * Create a command for updating the given issue.
    */
    Command* updateIssue( int issueId );

    /**
    * Find the issue containing the given item.
    * @param itemId Identifier of an issue, comment or file.
    * @return Identifier of the issue or 0 if the item wasn't found.
    */
    int findItem( int itemId );

private slots:
    void helloReply( const Reply& reply );
    void loginReply( const Reply& reply );
    void updateSettingsReply( const Reply& reply );
    void updateUsersReply( const Reply& reply );
    void updateTypesReply( const Reply& reply );
    void updateProjectsReply( const Reply& reply );
    void updateStatesReply( const Reply& reply );
    void updateFolderReply( const Reply& reply );
    void updateIssueReply( const Reply& reply );

private:
    void notifyObservers( UpdateEvent::Unit unit, int id = 0 );

    FolderState* folderState( int folderId );
    IssueState* issueState( int issueId );
    AlertState* alertState( int alertId );

    void flushIssueCache();
    void removeIssueDetails( int issueId );

    void recalculateAllAlerts();
    void recalculateAlerts( int folderId );

    UserRow* readUserRow( const ReplyLine& line );
    MemberRow* readMemberRow( const ReplyLine& line );
    TypeRow* readTypeRow( const ReplyLine& line );
    AttributeRow* readAttributeRow( const ReplyLine& line );
    ProjectRow* readProjectRow( const ReplyLine& line );
    FolderRow* readFolderRow( const ReplyLine& line );
    IssueRow* readIssueRow( const ReplyLine& line );
    ValueRow* readValueRow( const ReplyLine& line );
    ChangeRow* readChangeRow( const ReplyLine& line );
    CommentRow* readCommentRow( const ReplyLine& line );
    FileRow* readFileRow( const ReplyLine& line );
    ViewRow* readViewRow( const ReplyLine& line );
    AlertRow* readAlertRow( const ReplyLine& line );

    void updateFolderCache( int folderId );
    void readFolderCache( int folderId );
    void saveFolderCache();
    void writeFolderCache( int folderId );

    void updateStateCache();
    void saveStateCache();

private:
    QString m_serverName;
    QString m_serverUuid;
    QString m_serverVersion;

    int m_currentUserId;
    QString m_currentUserLogin;
    QString m_currentUserName;
    Access m_currentUserAccess;

    UsersTable m_users;
    MembersTable m_members;
    TypesTable m_types;
    AttributesTable m_attributes;
    ProjectsTable m_projects;
    FoldersTable m_folders;
    IssuesTable m_issues;
    ValuesTable m_values;
    ChangesTable m_changes;
    CommentsTable m_comments;
    FilesTable m_files;
    ViewsTable m_views;
    AlertsTable m_alerts;

    FolderStatesTable m_folderStates;
    IssueStatesTable m_issueStates;
    AlertStatesTable m_alertStates;

    bool m_stateCached;
    int m_lastStateId;

    QMap<QString, QString> m_settings;
    QMap<QPair<int, QString>, QString> m_viewSettings;

    LocaleCache* m_localeCache;
    AttachmentsCache* m_attachmentsCache;
    LocalSettings* m_connectionSettings;

    QList<QObject*> m_observers;
};

/**
* Global pointer used to access the DataManager.
*/
extern DataManager* dataManager;

#endif
