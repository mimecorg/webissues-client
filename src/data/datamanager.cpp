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

#include "datamanager.h"

#include "application.h"
#include "commands/command.h"
#include "commands/commandmanager.h"
#include "data/localsettings.h"
#include "data/issuetypecache.h"
#include "data/filecache.h"
#include "data/query.h"
#include "models/querygenerator.h"

#include <QSqlDatabase>
#include <QFile>

DataManager* dataManager = NULL;

DataManager::DataManager() :
    m_valid( false ),
    m_currentUserId( 0 ),
    m_currentUserAccess( NoAccess ),
    m_connectionSettings( NULL ),
    m_fileCache( NULL ),
    m_localeUpdated( false )
{
}

DataManager::~DataManager()
{
    if ( m_valid ) {
        clearIssueLocks();
        closeDatabase();
    }

    if ( m_fileCache )
        m_fileCache->flush();
}

static int parseVersion( const QString& version )
{
    QRegExp versionRegExp( "(\\d+)\\.(\\d+)(?:\\.(\\d+))?.*" );

    if ( !versionRegExp.exactMatch( version ) )
        return -1;

    int major = versionRegExp.cap( 1 ).toInt();
    int minor = versionRegExp.cap( 2 ).toInt();
    int patch = versionRegExp.cap( 3 ).toInt();

    return ( major << 16 ) + ( minor << 8 ) + patch;
}

bool DataManager::checkServerVersion( const QString& version ) const
{
    return parseVersion( m_serverVersion ) >= parseVersion( version );
}

QString DataManager::setting( const QString& key ) const
{
    return m_settings.value( key );
}

IssueTypeCache* DataManager::issueTypeCache( int typeId )
{
    IssueTypeCache* cache = m_issueTypesCache.value( typeId );
    if ( !cache ) {
        cache = new IssueTypeCache( typeId, this );
        m_issueTypesCache.insert( typeId, cache );
    }
    return cache;
}

QString DataManager::locateDataFile( const QString& name )
{
    return application->locateDataFile( m_serverUuid + '/' + name );
}

QString DataManager::locateCacheFile( const QString& name )
{
    return application->locateCacheFile( m_serverUuid + '/' + name );
}

void DataManager::addObserver( QObject* observer )
{
    m_observers.append( observer );
}

void DataManager::removeObserver( QObject* observer )
{
    m_observers.removeAt( m_observers.indexOf( observer ) );
}

void DataManager::notifyObservers( UpdateEvent::Unit unit, int id )
{
    for ( int i = 0; i < m_observers.count(); i++ ) {
        UpdateEvent* updateEvent = new UpdateEvent( unit, id );
        QApplication::postEvent( m_observers.at( i ), updateEvent );
    }
}

bool DataManager::openDatabase()
{
    QSqlDatabase database = QSqlDatabase::addDatabase( "SQLITEX" );

    database.setDatabaseName( locateCacheFile( "cache.db" ) );

    if ( !database.open() )
        return false;

    if ( !lockDatabase( database ) ) {
        database.close();
        return false;
    }

    database.transaction();

    bool ok = installSchema( database );
    if ( ok )
        ok = database.commit();

    if ( !ok ) {
        database.rollback();
        database.close();
    }

    return ok;
}

bool DataManager::lockDatabase( const QSqlDatabase& database )
{
    Query query( database );

    if ( !query.execQuery( "PRAGMA locking_mode = EXCLUSIVE" ) )
        return false;
    if ( !query.execQuery( "BEGIN EXCLUSIVE" ) )
        return false;
    if ( !query.execQuery( "COMMIT" ) )
        return false;

    return true;
}

bool DataManager::installSchema( const QSqlDatabase& database )
{
    const int schemaVersion = 2;

    Query query( database );

    if ( !query.execQuery( "PRAGMA user_version" ) )
        return false;

    int currentVersion = query.readScalar().toInt();

    if ( currentVersion == schemaVersion )
        return true;

    if ( currentVersion > schemaVersion )
        return false;

    if ( currentVersion < 1 ) {
        const char* schema[] = {
            "CREATE TABLE alerts ( alert_id integer UNIQUE, folder_id integer, view_id integer, alert_email integer )",
            "CREATE TABLE alerts_cache ( alert_id integer UNIQUE, total_count integer, modified_count integer, new_count integer )",
            "CREATE TABLE attr_types ( attr_id integer UNIQUE, type_id integer, attr_name text, attr_def text )",
            "CREATE TABLE attr_values ( attr_id integer, issue_id integer, attr_value text, UNIQUE ( attr_id, issue_id ) )",
            "CREATE TABLE changes ( change_id integer UNIQUE, issue_id integer, change_type integer, stamp_id integer, created_time integer, created_user_id integer, "
                "modified_time integer, modified_user_id integer, attr_id integer, old_value text, new_value text, from_folder_id integer, to_folder_id integer )",
            "CREATE INDEX changes_issue_idx ON changes ( issue_id )",
            "CREATE TABLE comments ( comment_id integer UNIQUE, comment_text text )",
            "CREATE TABLE files ( file_id integer UNIQUE, file_name text, file_size integer, file_descr text )",
            "CREATE TABLE folders ( folder_id integer UNIQUE, project_id integer, folder_name text, type_id integer, stamp_id integer )",
            "CREATE TABLE folders_cache ( folder_id integer UNIQUE, list_id integer )",
            "CREATE TABLE formats ( format_type text, format_key text, format_def text )",
            "CREATE TABLE issue_locks ( issue_id integer UNIQUE, lock_count integer, last_access integer )",
            "CREATE TABLE issue_states ( user_id integer, issue_id integer, read_id integer, UNIQUE ( user_id, issue_id ) )",
            "CREATE TABLE issue_types ( type_id integer UNIQUE, type_name text )",
            "CREATE TABLE issues ( issue_id integer UNIQUE, folder_id integer, issue_name text, stamp_id integer, created_time integer, created_user_id integer, "
                "modified_time integer, modified_user_id integer )",
            "CREATE INDEX issues_folder_idx ON issues ( folder_id )",
            "CREATE TABLE issues_cache ( issue_id integer UNIQUE, details_id integer )",
            "CREATE TABLE languages ( lang_code text, lang_name text )",
            "CREATE TABLE preferences ( user_id integer, pref_key text, pref_value text, UNIQUE ( user_id, pref_key ) )",
            "CREATE TABLE projects ( project_id integer UNIQUE, project_name text )",
            "CREATE TABLE rights ( project_id integer, user_id integer, project_access integer, UNIQUE ( project_id, user_id ) )",
            "CREATE TABLE settings ( set_key text UNIQUE, set_value text )",
            "CREATE TABLE time_zones ( tz_name text, tz_offset integer )",
            "CREATE TABLE users ( user_id integer UNIQUE, user_login text, user_name text, user_access integer )",
            "CREATE TABLE users_cache ( user_id integer UNIQUE, state_id integer )",
            "CREATE TABLE view_settings ( type_id integer, set_key text, set_value text, UNIQUE ( type_id, set_key ) )",
            "CREATE TABLE views ( view_id integer UNIQUE, type_id integer, view_name text, view_def text, is_public integer )"
        };

        for ( int i = 0; i < (int)( sizeof( schema ) / sizeof( schema[ 0 ] ) ); i++ ) {
            if ( !query.execQuery( schema[ i ] ) )
                return false;
        }

        currentVersion = schemaVersion;
    }

    if ( currentVersion < 2 ) {
        if ( !query.execQuery( "SELECT file_id, file_path, file_size FROM files_cache ORDER BY last_access ASC" ) )
            return false;

        while ( query.next() ) {
            int fileId = query.value( 0 ).toInt();
            QString path = query.value( 1 ).toString();
            int size = query.value( 2 ).toInt();

            if ( QFile::exists( path ) )
                m_fileCache->commitFile( fileId, path, size );
        }

        m_fileCache->flush();

        if ( !query.execQuery( "DROP TABLE files_cache" ) )
            return false;
    }

    QString sql = QString( "PRAGMA user_version = %1" ).arg( schemaVersion );

    if ( !query.execQuery( sql ) )
        return false;

    return true;
}

void DataManager::closeDatabase()
{
    QSqlDatabase database = QSqlDatabase::database();
    database.close();
}

bool DataManager::localeUpdateNeeded() const
{
    return !m_localeUpdated;
}

bool DataManager::folderUpdateNeeded( int folderId ) const
{
    QSqlDatabase database = QSqlDatabase::database();
    Query query( database );

    if ( !query.execQuery( "SELECT stamp_id FROM folders WHERE folder_id = ?", folderId ) )
        return false;

    int stampId = query.readScalar().toInt();

    if ( stampId == 0 )
        return true;

    if ( !query.execQuery( "SELECT list_id FROM folders_cache WHERE folder_id = ?", folderId ) )
        return false;

    int lastStampId = query.readScalar().toInt();

    if ( stampId > lastStampId )
        return true;

    return false;
}

bool DataManager::issueUpdateNeeded( int issueId ) const
{
    QSqlDatabase database = QSqlDatabase::database();
    Query query( database );

    if ( !query.execQuery( "SELECT stamp_id FROM issues WHERE issue_id = ?", issueId ) )
        return false;

    int stampId = query.readScalar().toInt();

    if ( stampId == 0 )
        return true;

    if ( !query.execQuery( "SELECT details_id FROM issues_cache WHERE issue_id = ?", issueId ) )
        return false;

    int lastStampId = query.readScalar().toInt();

    if ( stampId > lastStampId )
        return true;

    return false;
}

Command* DataManager::hello()
{
    Command* command = new Command();

    command->setKeyword( "HELLO" );

    command->addRule( "S sss", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( helloReply( const Reply& ) ) );

    return command;
}

void DataManager::helloReply( const Reply& reply )
{
    m_serverName = reply.at( 0 ).argString( 0 );
    m_serverUuid = reply.at( 0 ).argString( 1 );
    m_serverVersion = reply.at( 0 ).argString( 2 );

    m_connectionSettings = new LocalSettings( locateDataFile( "connection.dat" ), this );

    m_fileCache = new FileCache( m_serverUuid, application->locateCacheFile( "filecache.dat" ), this );
    m_fileCache->flush();

    connect( application->applicationSettings(), SIGNAL( settingsChanged() ), this, SLOT( settingsChanged() ) );

    m_valid = openDatabase();

    if ( m_valid )
        clearIssueLocks();
}

Command* DataManager::login( const QString& login, const QString& password )
{
    m_currentUserLogin = login;

    Command* command = new Command();

    command->setKeyword( "LOGIN" );
    command->addArg( login );
    command->addArg( password );

    command->addRule( "U isi", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( loginReply( const Reply& ) ) );

    return command;
}

Command* DataManager::loginNew( const QString& login, const QString& password, const QString& newPassword )
{
    m_currentUserLogin = login;

    Command* command = new Command();

    command->setKeyword( "LOGIN NEW" );
    command->addArg( login );
    command->addArg( password );
    command->addArg( newPassword );

    command->addRule( "U isi", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( loginReply( const Reply& ) ) );

    return command;
}

void DataManager::loginReply( const Reply& reply )
{
    m_currentUserId = reply.at( 0 ).argInt( 0 );
    m_currentUserName = reply.at( 0 ).argString( 1 );
    m_currentUserAccess = (Access)reply.at( 0 ).argInt( 2 );
}

Command* DataManager::updateSettings()
{
    Command* command = new Command();

    command->setKeyword( "GET SETTINGS" );
    command->addArg( application->language() );

    command->setAcceptNullReply( true );
    command->addRule( "S ss", ReplyRule::ZeroOrMore );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( updateSettingsReply( const Reply& ) ) );

    return command;
}

void DataManager::updateSettingsReply( const Reply& reply )
{
    QSqlDatabase database = QSqlDatabase::database();
    database.transaction();

    bool ok = updateSettingsReply( reply, database );
    if ( ok )
        ok = database.commit();

    if ( !ok )
        database.rollback();

    if ( ok )
        notifyObservers( UpdateEvent::Settings );
}

bool DataManager::updateSettingsReply( const Reply& reply, const QSqlDatabase& database )
{
    Query query( database );

    if ( !query.execQuery( "DELETE FROM settings" ) )
        return false;

    QMap<QString, QString> settings;

    query.setQuery( "INSERT INTO settings VALUES ( ?, ? )" );

    for ( int i = 0; i < reply.count() && reply.at( i ).keyword() == QLatin1String( "S" ); i++ ) {
        if ( !query.exec( reply.at( i ).args() ) )
            return false;

        settings.insert( reply.at( i ).argString( 0 ), reply.at( i ).argString( 1 ) );
    }

    m_settings = settings;

    m_numberFormat = DefinitionInfo::fromString( m_settings.value( "number_format" ) );
    m_dateFormat = DefinitionInfo::fromString( m_settings.value( "date_format" ) );
    m_timeFormat = DefinitionInfo::fromString( m_settings.value( "time_format" ) );

    return true;
}

Command* DataManager::updateLocale()
{
    Command* command = new Command();

    command->setKeyword( "GET LOCALE" );

    command->setAcceptNullReply( true );
    command->addRule( "L ss", ReplyRule::ZeroOrMore );
    command->addRule( "F sss", ReplyRule::ZeroOrMore );
    command->addRule( "Z si", ReplyRule::ZeroOrMore );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( updateLocaleReply( const Reply& ) ) );

    return command;
}

void DataManager::updateLocaleReply( const Reply& reply )
{
    QSqlDatabase database = QSqlDatabase::database();
    database.transaction();

    bool ok = updateLocaleReply( reply, database );
    if ( ok )
        ok = database.commit();

    if ( !ok )
        database.rollback();
}

bool DataManager::updateLocaleReply( const Reply& reply, const QSqlDatabase& database )
{
    Query query( database );

    if ( !query.execQuery( "DELETE FROM languages" ) )
        return false;

    int i = 0;

    query.setQuery( "INSERT INTO languages VALUES ( ?, ? )" );

    for ( ; i < reply.count() && reply.at( i ).keyword() == QLatin1String( "L" ); i++ ) {
        if ( !query.exec( reply.at( i ).args() ) )
            return false;
    }

    if ( !query.execQuery( "DELETE FROM formats" ) )
        return false;

    query.setQuery( "INSERT INTO formats VALUES ( ?, ?, ? )" );

    for ( ; i < reply.count() && reply.at( i ).keyword() == QLatin1String( "F" ); i++ ) {
        if ( !query.exec( reply.at( i ).args() ) )
            return false;
    }

    if ( !query.execQuery( "DELETE FROM time_zones" ) )
        return false;

    query.setQuery( "INSERT INTO time_zones VALUES ( ?, ? )" );

    for ( ; i < reply.count() && reply.at( i ).keyword() == QLatin1String( "Z" ); i++ ) {
        if ( !query.exec( reply.at( i ).args() ) )
            return false;
    }

    m_localeUpdated = true;

    return true;
}

Command* DataManager::updatePreferences( int userId )
{
    Command* command = new Command();

    command->setKeyword( "LIST PREFERENCES" );
    command->addArg( userId );

    command->setAcceptNullReply( true );
    command->setReportNullReply( true );
    command->addRule( "P ss", ReplyRule::ZeroOrMore );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( updatePreferencesReply( const Reply& ) ) );

    return command;
}

void DataManager::updatePreferencesReply( const Reply& reply )
{
    Command* command = static_cast<Command*>( sender() );
    int userId = command->argInt( 0 );

    QSqlDatabase database = QSqlDatabase::database();
    database.transaction();

    bool ok = updatePreferencesReply( reply, userId, database );
    if ( ok )
        ok = database.commit();

    if ( !ok )
        database.rollback();
}

bool DataManager::updatePreferencesReply( const Reply& reply, int userId, const QSqlDatabase& database )
{
    Query query( database );

    if ( !query.execQuery( "DELETE FROM preferences WHERE user_id = ?", userId ) )
        return false;

    query.setQuery( "INSERT INTO preferences VALUES ( ?, ?, ? )" );

    for ( int i = 0; i < reply.count(); i++ ) {
        if ( !query.exec( userId, reply.at( i ).arg( 0 ), reply.at( i ).arg( 1 ) ) )
            return false;
    }

    return true;
}

Command* DataManager::updateUsers()
{
    Command* command = new Command();

    command->setKeyword( "LIST USERS" );

    command->setAcceptNullReply( true );
    command->addRule( "U issi", ReplyRule::ZeroOrMore );
    command->addRule( "M iii", ReplyRule::ZeroOrMore );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( updateUsersReply( const Reply& ) ) );

    return command;
}

void DataManager::updateUsersReply( const Reply& reply )
{
    QSqlDatabase database = QSqlDatabase::database();
    database.transaction();

    bool ok = updateUsersReply( reply, database );
    if ( ok )
        ok = database.commit();

    if ( !ok )
        database.rollback();

    if ( ok )
        notifyObservers( UpdateEvent::Users );
}

bool DataManager::updateUsersReply( const Reply& reply, const QSqlDatabase& database )
{
    Query query( database );

    if ( !query.execQuery( "DELETE FROM users" ) )
        return false;

    int i = 0;

    query.setQuery( "INSERT INTO users VALUES ( ?, ?, ?, ? )" );

    for ( ; i < reply.count() && reply.at( i ).keyword() == QLatin1String( "U" ); i++ ) {
        if ( !query.exec( reply.at( i ).args() ) )
            return false;
    }

    if ( !query.execQuery( "DELETE FROM rights" ) )
        return false;

    query.setQuery( "INSERT INTO rights ( user_id, project_id, project_access ) VALUES ( ?, ?, ? )" );

    for ( ; i < reply.count() && reply.at( i ).keyword() == QLatin1String( "M" ); i++ ) {
        if ( !query.exec( reply.at( i ).args() ) )
            return false;
    }

    return true;
}

Command* DataManager::updateTypes()
{
    Command* command = new Command();

    command->setKeyword( "LIST TYPES" );

    command->setAcceptNullReply( true );
    command->setReportNullReply( true );
    command->addRule( "T is", ReplyRule::ZeroOrMore );
    command->addRule( "A iiss", ReplyRule::ZeroOrMore );
    command->addRule( "V iissi", ReplyRule::ZeroOrMore );
    command->addRule( "S iss", ReplyRule::ZeroOrMore );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( updateTypesReply( const Reply& ) ) );

    return command;
}

void DataManager::updateTypesReply( const Reply& reply )
{
    QSqlDatabase database = QSqlDatabase::database();
    database.transaction();

    bool ok = updateTypesReply( reply, database );
    if ( ok )
        ok = database.commit();

    if ( !ok )
        database.rollback();

    if ( ok )
        notifyObservers( UpdateEvent::Types );
}

bool DataManager::updateTypesReply( const Reply& reply, const QSqlDatabase& database )
{
    Query query( database );

    if ( !query.execQuery( "DELETE FROM issue_types" ) )
        return false;

    int i = 0;

    query.setQuery( "INSERT INTO issue_types VALUES ( ?, ? )" );

    for ( ; i < reply.count() && reply.at( i ).keyword() == QLatin1String( "T" ); i++ ) {
        if ( !query.exec( reply.at( i ).args() ) )
            return false;
    }

    if ( !query.execQuery( "DELETE FROM attr_types" ) )
        return false;

    query.setQuery( "INSERT INTO attr_types VALUES ( ?, ?, ?, ? )" );

    for ( ; i < reply.count() && reply.at( i ).keyword() == QLatin1String( "A" ); i++ ) {
        if ( !query.exec( reply.at( i ).args() ) )
            return false;
    }

    if ( !query.execQuery( "DELETE FROM views" ) )
        return false;

    query.setQuery( "INSERT INTO views VALUES ( ?, ?, ?, ?, ? )" );

    for ( ; i < reply.count() && reply.at( i ).keyword() == QLatin1String( "V" ); i++ ) {
        if ( !query.exec( reply.at( i ).args() ) )
            return false;
    }

    if ( !query.execQuery( "DELETE FROM view_settings" ) )
        return false;

    query.setQuery( "INSERT INTO view_settings VALUES ( ?, ?, ? )" );

    for ( ; i < reply.count() && reply.at( i ).keyword() == QLatin1String( "S" ); i++ ) {
        if ( !query.exec( reply.at( i ).args() ) )
            return false;
    }

    qDeleteAll( m_issueTypesCache );
    m_issueTypesCache.clear();

    if ( !recalculateAllAlerts( database ) )
        return false;

    return true;
}

Command* DataManager::updateProjects()
{
    Command* command = new Command();

    command->setKeyword( "LIST PROJECTS" );

    command->setAcceptNullReply( true );
    command->setReportNullReply( true );
    command->addRule( "P is", ReplyRule::ZeroOrMore );
    command->addRule( "F iisii", ReplyRule::ZeroOrMore );
    command->addRule( "A iiii", ReplyRule::ZeroOrMore );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( updateProjectsReply( const Reply& ) ) );

    return command;
}

void DataManager::updateProjectsReply( const Reply& reply )
{
    QSqlDatabase database = QSqlDatabase::database();
    database.transaction();

    bool ok = updateProjectsReply( reply, database );
    if ( ok )
        ok = database.commit();

    if ( !ok )
        database.rollback();

    if ( ok )
        notifyObservers( UpdateEvent::Projects );
}

bool DataManager::updateProjectsReply( const Reply& reply, const QSqlDatabase& database )
{
    Query query( database );

    if ( !query.execQuery( "DELETE FROM projects" ) )
        return false;

    int i = 0;

    query.setQuery( "INSERT INTO projects VALUES ( ?, ? )" );

    for ( ; i < reply.count() && reply.at( i ).keyword() == QLatin1String( "P" ); i++ ) {
        if ( !query.exec( reply.at( i ).args() ) )
            return false;
    }

    if ( !query.execQuery( "DELETE FROM folders" ) )
        return false;

    query.setQuery( "INSERT INTO folders VALUES ( ?, ?, ?, ?, ? )" );

    for ( ; i < reply.count() && reply.at( i ).keyword() == QLatin1String( "F" ); i++ ) {
        if ( !query.exec( reply.at( i ).args() ) )
            return false;
    }

    if ( !query.execQuery( "DELETE FROM alerts" ) )
        return false;

    query.setQuery( "INSERT INTO alerts VALUES ( ?, ?, ?, ? )" );

    for ( ; i < reply.count() && reply.at( i ).keyword() == QLatin1String( "A" ); i++ ) {
        if ( !query.exec( reply.at( i ).args() ) )
            return false;
    }

    if ( !recalculateAllAlerts( database ) )
        return false;

    return true;
}

Command* DataManager::updateStates()
{
    QSqlDatabase database = QSqlDatabase::database();
    Query query( database );

    if ( !query.execQuery( "SELECT state_id FROM users_cache WHERE user_id = ?", m_currentUserId ) )
        return NULL;

    int lastStateId = query.readScalar().toInt();

    Command* command = new Command();

    command->setKeyword( "LIST STATES" );
    command->addArg( lastStateId );

    command->setAcceptNullReply( true );
    command->addRule( "S iii", ReplyRule::ZeroOrMore );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( updateStatesReply( const Reply& ) ) );

    return command;
}

void DataManager::updateStatesReply( const Reply& reply )
{
    Command* command = static_cast<Command*>( sender() );
    int lastStateId = command->argInt( 0 );

    QSqlDatabase database = QSqlDatabase::database();
    database.transaction();

    bool ok = updateStatesReply( reply, lastStateId, database );
    if ( ok )
        ok = database.commit();

    if ( !ok )
        database.rollback();

    if ( ok )
        notifyObservers( UpdateEvent::States );
}

bool DataManager::updateStatesReply( const Reply& reply, int lastStateId, const QSqlDatabase& database )
{
    Query query( database );

    query.setQuery( "INSERT OR REPLACE INTO issue_states VALUES ( ?, ?, ? )" );

    for ( int i = 0; i < reply.count(); i++ ) {
        if ( !query.exec( m_currentUserId, reply.at( i ).arg( 1 ), reply.at( i ).arg( 2 ) ) )
            return false;

        int stateId = reply.at( i ).argInt( 0 );
        if ( stateId > lastStateId )
            lastStateId = stateId;
    }

    if ( !query.execQuery( "INSERT OR REPLACE INTO users_cache VALUES ( ?, ? )", m_currentUserId, lastStateId ) )
        return false;

    if ( !recalculateAllAlerts( database ) )
        return false;

    return true;
}

Command* DataManager::updateFolder( int folderId )
{
    QSqlDatabase database = QSqlDatabase::database();
    Query query( database );

    if ( !query.execQuery( "SELECT list_id FROM folders_cache WHERE folder_id = ?", folderId ) )
        return false;

    int lastStampId = query.readScalar().toInt();

    Command* command = new Command();

    command->setKeyword( "LIST ISSUES" );
    command->addArg( folderId );
    command->addArg( lastStampId );

    command->setAcceptNullReply( true );
    command->addRule( "F iisii", ReplyRule::One );
    command->addRule( "I iisiiiii", ReplyRule::ZeroOrMore );
    command->addRule( "V iis", ReplyRule::ZeroOrMore );
    command->addRule( "X iii", ReplyRule::ZeroOrMore );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( updateFolderReply( const Reply& ) ) );

    return command;
}

void DataManager::updateFolderReply( const Reply& reply )
{
    QList<int> updatedFolders;

    QSqlDatabase database = QSqlDatabase::database();
    database.transaction();

    bool ok = updateFolderReply( reply, database, updatedFolders );
    if ( ok )
        ok = database.commit();

    if ( !ok )
        database.rollback();

    if ( ok ) {
        foreach ( int folderId, updatedFolders )
            notifyObservers( UpdateEvent::Folder, folderId );

        notifyObservers( UpdateEvent::AlertStates );
    }
}

bool DataManager::updateFolderReply( const Reply& reply, const QSqlDatabase& database, QList<int>& updatedFolders )
{
    int folderId = reply.at( 0 ).argInt( 0 );
    int lastStampId = reply.at( 0 ).argInt( 4 );

    updatedFolders.append( folderId );

    Query query( database );

    if ( !query.execQuery( "INSERT OR REPLACE INTO folders VALUES ( ?, ?, ?, ?, ? )", reply.at( 0 ).args() ) )
        return false;

    if ( !query.execQuery( "INSERT OR REPLACE INTO folders_cache VALUES ( ?, ? )", folderId, lastStampId ) )
        return false;

    Query oldIssueQuery( "SELECT folder_id FROM issues WHERE issue_id = ?", database );
    Query deleteValuesQuery( "DELETE FROM attr_values WHERE issue_id = ?", database );
    Query insertIssueQuery( "INSERT OR REPLACE INTO issues VALUES ( ?, ?, ?, ?, ?, ?, ?, ? )", database );

    int i = 1;

    for ( ; i < reply.count() && reply.at( i ).keyword() == QLatin1String( "I" ); i++ ) {
        int issueId = reply.at( i ).argInt( 0 );

        if ( !oldIssueQuery.exec( issueId ) )
            return false;

        if ( oldIssueQuery.next() ) {
            int oldFolderId = oldIssueQuery.value( 0 ).toInt();

            if ( !updatedFolders.contains( oldFolderId ) )
                updatedFolders.append( oldFolderId );

            if ( !deleteValuesQuery.exec( issueId ) )
                return false;
        }

        if ( !insertIssueQuery.exec( reply.at( i ).args() ) )
            return false;
    }

    query.setQuery( "INSERT INTO attr_values VALUES ( ?, ?, ? )" );

    for ( ; i < reply.count() && reply.at( i ).keyword() == QLatin1String( "V" ); i++ ) {
        if ( !query.exec( reply.at( i ).args() ) )
            return false;
    }

    Query moveIssueQuery( "UPDATE issues SET folder_id = ?, stamp_id = ? WHERE issue_id = ?", database );
    Query deleteIssueQuery( "DELETE FROM issues WHERE issue_id = ?", database );

    QList<int> deletedIssues;

    for ( ; i < reply.count() && reply.at( i ).keyword() == QLatin1String( "X" ); i++ ) {
        int issueId = reply.at( i ).argInt( 0 );
        int toFolderId = reply.at( i ).argInt( 1 );
        int stampId = reply.at( i ).argInt( 2 );

        if ( toFolderId != 0 ) {
            if ( !oldIssueQuery.exec( issueId ) )
                return false;

            if ( oldIssueQuery.next() ) {
                int oldFolderId = oldIssueQuery.value( 0 ).toInt();

                if ( oldFolderId != toFolderId ) {
                    if ( !updatedFolders.contains( toFolderId ) )
                        updatedFolders.append( toFolderId );

                    if ( !moveIssueQuery.exec( toFolderId, stampId, issueId ) )
                        return false;
                }
            }
        } else {
            deletedIssues.append( issueId );

            if ( !deleteIssueQuery.exec( issueId ) )
                return false;

            if ( !deleteValuesQuery.exec( issueId ) )
                return false;
        }
    }

    if ( !deletedIssues.isEmpty() ) {
        if ( !removeIssueDetails( deletedIssues, database ) )
            return false;
    }

    foreach ( int folderId, updatedFolders ) {
        if ( !recalculateAlerts( folderId, database ) )
            return false;
    }

    return true;
}

Command* DataManager::updateIssue( int issueId, bool markAsRead )
{
    QSqlDatabase database = QSqlDatabase::database();
    Query query( database );

    if ( !query.execQuery( "SELECT details_id FROM issues_cache WHERE issue_id = ?", issueId ) )
        return NULL;

    int lastStampId = query.readScalar().toInt();

    Command* command = new Command();

    command->setKeyword( "GET DETAILS" );
    command->addArg( issueId );
    command->addArg( lastStampId );
    command->addArg( markAsRead ? 1 : 0 );

    command->setAcceptNullReply( true );
    command->addRule( "I iisiiiii", ReplyRule::One );
    command->addRule( "V iis", ReplyRule::ZeroOrMore );
    command->addRule( "H iiiiiiiiissii", ReplyRule::ZeroOrMore );
    command->addRule( "C is", ReplyRule::ZeroOrMore );
    command->addRule( "A isis", ReplyRule::ZeroOrMore );
    command->addRule( "X i", ReplyRule::ZeroOrMore );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( updateIssueReply( const Reply& ) ) );

    return command;
}

void DataManager::updateIssueReply( const Reply& reply )
{
    QList<int> updatedFolders;
    int issueId;

    QSqlDatabase database = QSqlDatabase::database();
    database.transaction();

    bool ok = updateIssueReply( reply, database, updatedFolders, issueId );
    if ( ok )
        ok = database.commit();

    if ( !ok )
        database.rollback();

    if ( ok ) {
        foreach ( int folderId, updatedFolders )
            notifyObservers( UpdateEvent::IssueList, folderId );

        notifyObservers( UpdateEvent::Issue, issueId );

        notifyObservers( UpdateEvent::AlertStates );
    }
}

bool DataManager::updateIssueReply( const Reply& reply, const QSqlDatabase& database, QList<int>& updatedFolders, int& issueId )
{
    Command* command = static_cast<Command*>( sender() );
    bool markAsRead = (bool)command->argInt( 2 );

    issueId = reply.at( 0 ).argInt( 0 );
    int folderId = reply.at( 0 ).argInt( 1 );
    int lastStampId = reply.at( 0 ).argInt( 3 );

    updatedFolders.append( folderId );

    Query query( database );

    if ( !query.execQuery( "SELECT folder_id FROM issues WHERE issue_id = ?", issueId ) )
        return false;

    if ( query.next() ) {
        int oldFolderId = query.value( 0 ).toInt();

        if ( !updatedFolders.contains( oldFolderId ) )
            updatedFolders.append( oldFolderId );

        if ( !query.execQuery( "DELETE FROM attr_values WHERE issue_id = ?", issueId ) )
            return false;
    }

    if ( !query.execQuery( "INSERT OR REPLACE INTO issues VALUES ( ?, ?, ?, ?, ?, ?, ?, ? )", reply.at( 0 ).args() ) )
        return false;

    if ( !query.execQuery( "INSERT OR REPLACE INTO issues_cache VALUES ( ?, ? )", issueId, lastStampId ) )
        return false;

    if ( markAsRead ) {
        if ( !query.execQuery( "INSERT OR REPLACE INTO issue_states VALUES ( ?, ?, ? )", m_currentUserId, issueId, lastStampId ) )
            return false;
    }

    int i = 1;

    query.setQuery( "INSERT INTO attr_values VALUES ( ?, ?, ? )" );

    for ( ; i < reply.count() && reply.at( i ).keyword() == QLatin1String( "V" ); i++ ) {
        if ( !query.exec( reply.at( i ).args() ) )
            return false;
    }

    query.setQuery( "INSERT OR REPLACE INTO changes VALUES ( ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ? )" );

    for ( ; i < reply.count() && reply.at( i ).keyword() == QLatin1String( "H" ); i++ ) {
        if ( !query.exec( reply.at( i ).args() ) )
            return false;
    }

    query.setQuery( "INSERT OR REPLACE INTO comments VALUES ( ?, ? )" );

    for ( ; i < reply.count() && reply.at( i ).keyword() == QLatin1String( "C" ); i++ ) {
        if ( !query.exec( reply.at( i ).args() ) )
            return false;
    }

    query.setQuery( "INSERT OR REPLACE INTO files VALUES ( ?, ?, ?, ? )" );

    for ( ; i < reply.count() && reply.at( i ).keyword() == QLatin1String( "A" ); i++ ) {
        if ( !query.exec( reply.at( i ).args() ) )
            return false;
    }

    Query oldChangeQuery( "SELECT change_type FROM changes WHERE change_id = ?", database );
    Query deleteCommentQuery( "DELETE FROM comments WHERE comment_id = ?", database );
    Query deleteFileQuery( "DELETE FROM files WHERE file_id = ?", database );
    Query deleteChangeQuery( "DELETE FROM changes WHERE change_id = ?", database );

    for ( ; i < reply.count() && reply.at( i ).keyword() == QLatin1String( "X" ); i++ ) {
        int changeId = reply.at( i ).argInt( 0 );

        if ( !oldChangeQuery.exec( changeId ) )
            return false;

        if ( oldChangeQuery.next() ) {
            int changeType = oldChangeQuery.value( 0 ).toInt();

            if ( changeType == CommentAdded ) {
                if ( !deleteCommentQuery.exec( changeId ) )
                    return false;
            } else if ( changeType == FileAdded ) {
                if ( !deleteFileQuery.exec( changeId ) )
                    return false;
            }

            if ( !deleteChangeQuery.exec( changeId ) )
                return false;
        }
    }

    if ( !flushIssueDetails( database ) )
        return false;

    foreach ( int folderId, updatedFolders ) {
        if ( !recalculateAlerts( folderId, database ) )
            return false;
    }

    return true;
}

void DataManager::lockIssue( int issueId )
{
    QSqlDatabase database = QSqlDatabase::database();
    database.transaction();

    bool ok = lockIssue( issueId, database );
    if ( ok )
        ok = database.commit();

    if ( !ok )
        database.rollback();
}

bool DataManager::lockIssue( int issueId, const QSqlDatabase& database )
{
    Query query( database );

    if ( !query.execQuery( "UPDATE issue_locks SET lock_count = lock_count + 1 WHERE issue_id = ?", issueId ) )
        return false;

    if ( query.numRowsAffected() == 0 ) {
        if ( !query.execQuery( "INSERT INTO issue_locks VALUES ( ?, 1, strftime( '%s', 'now' ) )", issueId ) )
            return false;
    }

    return true;
}

void DataManager::unlockIssue( int issueId )
{
    QSqlDatabase database = QSqlDatabase::database();
    database.transaction();

    bool ok = unlockIssue( issueId, database );
    if ( ok )
        ok = database.commit();

    if ( !ok )
        database.rollback();
}

bool DataManager::unlockIssue( int issueId, const QSqlDatabase& database )
{
    Query query( database );

    if ( !query.execQuery( "UPDATE issue_locks SET lock_count = lock_count - 1, last_access = strftime( '%s', 'now' ) WHERE issue_id = ?", issueId ) )
        return false;

    if ( !flushIssueDetails( database ) )
        return false;

    return true;
}

void DataManager::clearIssueLocks()
{
    QSqlDatabase database = QSqlDatabase::database();
    database.transaction();

    bool ok = clearIssueLocks( database );
    if ( ok )
        ok = database.commit();

    if ( !ok )
        database.rollback();
}

bool DataManager::clearIssueLocks( const QSqlDatabase& database )
{
    Query query( database );

    if ( !query.execQuery( "UPDATE issue_locks SET lock_count = 0" ) )
        return false;

    if ( !flushIssueDetails( database ) )
        return false;

    return true;
}

void DataManager::flushIssueDetails()
{
    QSqlDatabase database = QSqlDatabase::database();
    database.transaction();

    bool ok = flushIssueDetails( database );
    if ( ok )
        ok = database.commit();

    if ( !ok )
        database.rollback();
}

bool DataManager::flushIssueDetails( const QSqlDatabase& database )
{
    Query query( database );

    if ( !query.execQuery( "SELECT COUNT(*) FROM issue_locks" ) )
        return false;

    LocalSettings* settings = application->applicationSettings();
    int limit = settings->value( "IssuesCacheSize" ).toInt();

    int count = query.readScalar().toInt();

    if ( count > limit ) {
        if ( !query.execQuery( "SELECT issue_id FROM issue_locks WHERE lock_count = 0 ORDER BY last_access LIMIT ?", count - limit ) )
            return false;

        QList<int> deletedIssues;

        while ( query.next() )
            deletedIssues.append( query.value( 0 ).toInt() );

        if ( !removeIssueDetails( deletedIssues, database ) )
            return false;
    }

    return true;
}

bool DataManager::removeIssueDetails( const QList<int>& issues, const QSqlDatabase& database )
{
    Query query( database );

    query.setQuery( "DELETE FROM comments WHERE comment_id IN ( SELECT change_id FROM changes WHERE issue_id = ? )" );

    foreach ( int issueId, issues ) {
        if ( !query.exec( issueId ) )
            return false;
    }

    query.setQuery( "DELETE FROM files WHERE file_id IN ( SELECT change_id FROM changes WHERE issue_id = ? )" );

    foreach ( int issueId, issues ) {
        if ( !query.exec( issueId ) )
            return false;
    }

    query.setQuery( "DELETE FROM changes WHERE issue_id = ?" );

    foreach ( int issueId, issues ) {
        if ( !query.exec( issueId ) )
            return false;
    }

    query.setQuery( "DELETE FROM issue_locks WHERE issue_id = ?" );

    foreach ( int issueId, issues ) {
        if ( !query.exec( issueId ) )
            return false;
    }

    query.setQuery( "DELETE FROM issues_cache WHERE issue_id = ?" );

    foreach ( int issueId, issues ) {
        if ( !query.exec( issueId ) )
            return false;
    }

    return true;
}

QString DataManager::findFilePath( int fileId ) const
{
    return m_fileCache->findFilePath( fileId );
}

QString DataManager::generateFilePath( const QString& name ) const
{
    return m_fileCache->generateFilePath( name );
}

void DataManager::allocFileSpace( int size )
{
    m_fileCache->allocFileSpace( size );
}

void DataManager::commitFile( int fileId, const QString& path, int size )
{
    m_fileCache->commitFile( fileId, path, size );
}

void DataManager::settingsChanged()
{
    flushIssueDetails();

    m_fileCache->flush();
}

bool DataManager::recalculateAllAlerts( const QSqlDatabase& database )
{
    Query query( database );

    if ( !query.execQuery( "DELETE FROM alerts_cache" ) )
        return false;

    if ( !query.execQuery( "SELECT a.alert_id, f.folder_id, a.view_id FROM alerts AS a JOIN folders AS f ON f.folder_id = a.folder_id" ) )
        return false;

    while ( query.next() ) {
        if ( !recalculateAlert( query.value( 0 ).toInt(), query.value( 1 ).toInt(), query.value( 2 ).toInt(), database ) )
            return false;
    }

    return true;
}

bool DataManager::recalculateAlerts( int folderId, const QSqlDatabase& database )
{
    Query query( database );

    if ( !query.execQuery( "DELETE FROM alerts_cache WHERE alert_id IN ( SELECT alert_id FROM alerts WHERE folder_id = ? )", folderId ) )
        return false;

    if ( !query.execQuery( "SELECT a.alert_id, f.folder_id, a.view_id FROM alerts AS a JOIN folders AS f ON f.folder_id = a.folder_id WHERE f.folder_id = ?", folderId ) )
        return false;

    while ( query.next() ) {
        if ( !recalculateAlert( query.value( 0 ).toInt(), query.value( 1 ).toInt(), query.value( 2 ).toInt(), database ) )
            return false;
    }

    return true;
}

bool DataManager::recalculateAlert( int alertId, int folderId, int viewId, const QSqlDatabase& database )
{
    QueryGenerator generator( folderId, viewId );

    QString sql = generator.query( false );
    if ( sql.isEmpty() )
        return true;

    Query query( database );

    if ( !query.execQuery( sql, generator.arguments() ) )
        return false;

    int total = 0;
    int modified = 0;
    int unread = 0;

    while ( query.next() ) {
        int readId = query.value( 2 ).toInt();
        if ( readId == 0 ) {
            unread++;
        } else {
            int stampId = query.value( 1 ).toInt();
            if ( readId < stampId )
                modified++;
        }
        total++;
    }

    if ( !query.execQuery( "INSERT INTO alerts_cache VALUES ( ?, ?, ?, ? )", alertId, total, modified, unread ) )
        return false;

    return true;
}
