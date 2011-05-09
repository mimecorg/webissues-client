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
#include "models/querygenerator.h"
#include "sqlite/sqlitedriver.h"
#include "sqlite/sqliteextension.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QFileInfo>
#include <QFile>

DataManager* dataManager = NULL;

DataManager::DataManager() :
    m_valid( false ),
    m_currentUserId( 0 ),
    m_currentUserAccess( NoAccess ),
    m_connectionSettings( NULL ),
    m_localeUpdated( false )
{
}

DataManager::~DataManager()
{
    if ( m_valid )
        closeDatabase();
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

class AutoSqlQuery : public QSqlQuery
{
public:
    AutoSqlQuery( const QString& sql, const QSqlDatabase& database ) : QSqlQuery( database ),
        m_sql( sql ),
        m_prepared( false )
    {
    }

public: // overrides
    void addBindValue( const QVariant& value, QSql::ParamType paramType = QSql::In )
    {
        ensurePrepared();
        QSqlQuery::addBindValue( value, paramType );
    }

    bool exec()
    {
        ensurePrepared();
        return QSqlQuery::exec();
    }

private:
    void ensurePrepared()
    {
        if ( !m_prepared ) {
            prepare( m_sql );
            m_prepared = true;
        }
    }

private:
    QString m_sql;
    bool m_prepared;
};

static bool execReply( const QString& sql, const char* keyword, const Reply& reply, const QSqlDatabase& database, int& i )
{
    AutoSqlQuery query( sql, database );

    for ( ; i < reply.lines().count(); i++ ) {
        const ReplyLine& line = reply.lines().at( i );

        if ( line.keyword() != QLatin1String( keyword ) )
            break;

        foreach ( const QVariant& arg, line.args() )
            query.addBindValue( arg );

        if ( !query.exec() )
            return false;
    }

    return true;
}

static QVariant execScalar( const QString& sql, const QSqlDatabase& database, const QVariant& arg = QVariant() )
{
    QSqlQuery query( database );
    query.prepare( sql );
    if ( arg.isValid() )
        query.addBindValue( arg );
    query.exec();

    if ( query.next() )
        return query.value( 0 );

    return QVariant();
}

bool DataManager::openDatabase()
{
    SQLiteDriver* driver = new SQLiteDriver();
    QSqlDatabase database = QSqlDatabase::addDatabase( driver );

    database.setDatabaseName( locateCacheFile( "cache.db" ) );

    if ( !database.open() )
        return false;

    installSQLiteExtension( driver->handle() );

    database.transaction();

    bool ok = installSchema( database );
    if ( ok )
        ok = clearIssueLocks( database );
    if ( ok )
        ok = flushFileCache( 0, database );
    if ( ok )
        ok = database.commit();

    if ( !ok ) {
        database.rollback();
        database.close();
    }

    return ok;
}

bool DataManager::installSchema( const QSqlDatabase& database )
{
    const int schemaVersion = 1;

    int currentVersion = execScalar( "PRAGMA user_version", database ).toInt();

    if ( currentVersion == schemaVersion )
        return true;

    if ( currentVersion > schemaVersion )
        return false;

    QSqlQuery query( database );

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
            "CREATE TABLE files_cache ( file_id integer UNIQUE, file_path text, file_size integer, last_access integer )",
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
            if ( !query.exec( schema[ i ] ) )
                return false;
        }
    }

    QString sql = QString( "PRAGMA user_version = %1" ).arg( schemaVersion );

    if ( !query.exec( sql ) )
        return false;

    return true;
}

void DataManager::closeDatabase()
{
    QSqlDatabase database = QSqlDatabase::database();
    database.transaction();

    bool ok = clearIssueLocks( database );
    if ( ok )
        ok = flushFileCache( 0, database );
    if ( ok )
        ok = database.commit();

    if ( !ok )
        database.rollback();

    database.close();
}

bool DataManager::localeUpdateNeeded() const
{
    return !m_localeUpdated;
}

bool DataManager::folderUpdateNeeded( int folderId ) const
{
    QSqlDatabase database = QSqlDatabase::database();

    int stampId = execScalar( "SELECT stamp_id FROM folders WHERE folder_id = ?", database, folderId ).toInt();
    int lastStampId = execScalar( "SELECT list_id FROM folders_cache WHERE folder_id = ?", database, folderId ).toInt();

    return ( stampId == 0 || stampId > lastStampId );
}

bool DataManager::issueUpdateNeeded( int issueId ) const
{
    QSqlDatabase database = QSqlDatabase::database();

    int stampId = execScalar( "SELECT stamp_id FROM issues WHERE issue_id = ?", database, issueId ).toInt();
    int lastStampId = execScalar( "SELECT details_id FROM issues_cache WHERE issue_id = ?", database, issueId ).toInt();

    return ( stampId == 0 || stampId > lastStampId );
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
    ReplyLine line = reply.lines().at( 0 );

    m_serverName = line.argString( 0 );
    m_serverUuid = line.argString( 1 );
    m_serverVersion = line.argString( 2 );

    m_connectionSettings = new LocalSettings( locateDataFile( "connection.dat" ), this );

    m_valid = openDatabase();
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
    ReplyLine line = reply.lines().at( 0 );

    m_currentUserId = line.argInt( 0 );
    m_currentUserName = line.argString( 1 );
    m_currentUserAccess = (Access)line.argInt( 2 );
}

Command* DataManager::updateSettings()
{
    Command* command = new Command();

    command->setKeyword( "GET SETTINGS" );

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
    QSqlQuery query( database );
    if ( !query.exec( "DELETE FROM settings" ) )
        return false;

    int i = 0;
    if ( !execReply( "INSERT INTO settings VALUES ( ?, ? )", "S", reply, database, i ) )
        return false;

    m_settings.clear();

    for ( i = 0; i < reply.lines().count(); i++ ) {
        const ReplyLine& line = reply.lines().at( i );
        m_settings.insert( line.argString( 0 ), line.argString( 1 ) );
    }

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
    QSqlQuery query( database );
    if ( !query.exec( "DELETE FROM languages" ) )
        return false;

    int i = 0;
    if ( !execReply( "INSERT INTO languages VALUES ( ?, ? )", "L", reply, database, i ) )
        return false;

    if ( !query.exec( "DELETE FROM formats" ) )
        return false;

    if ( !execReply( "INSERT INTO formats VALUES ( ?, ?, ? )", "F", reply, database, i ) )
        return false;

    if ( !query.exec( "DELETE FROM time_zones" ) )
        return false;

    if ( !execReply( "INSERT INTO time_zones VALUES ( ?, ? )", "Z", reply, database, i ) )
        return false;

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
    int userId = command->args().at( 0 ).toInt();

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
    QSqlQuery query( database );

    query.prepare( "DELETE FROM preferences WHERE user_id = ?" );
    query.addBindValue( userId );
    if ( !query.exec() )
        return false;

    AutoSqlQuery insertPreferencesQuery( "INSERT INTO preferences VALUES ( ?, ?, ? )", database );

    for ( int i = 0; i < reply.lines().count(); i++ ) {
        const ReplyLine& line = reply.lines().at( i );

        query.addBindValue( userId );
        foreach ( const QVariant& arg, line.args() )
            query.addBindValue( arg );

        if ( !query.exec() )
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
    QSqlQuery query( database );
    if ( !query.exec( "DELETE FROM users" ) )
        return false;

    int i = 0;
    if ( !execReply( "INSERT INTO users VALUES ( ?, ?, ?, ? )", "U", reply, database, i ) )
        return false;

    if ( !query.exec( "DELETE FROM rights" ) )
        return false;

    if ( !execReply( "INSERT INTO rights VALUES ( ?, ?, ? )", "M", reply, database, i ) )
        return false;

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
    QSqlQuery query( database );
    if ( !query.exec( "DELETE FROM issue_types" ) )
        return false;

    int i = 0;
    if ( !execReply( "INSERT INTO issue_types VALUES ( ?, ? )", "T", reply, database, i ) )
        return false;

    if ( !query.exec( "DELETE FROM attr_types" ) )
        return false;

    if ( !execReply( "INSERT INTO attr_types VALUES ( ?, ?, ?, ? )", "A", reply, database, i ) )
        return false;

    if ( !query.exec( "DELETE FROM views" ) )
        return false;

    if ( !execReply( "INSERT INTO views VALUES ( ?, ?, ?, ?, ? )", "V", reply, database, i ) )
        return false;

    if ( !query.exec( "DELETE FROM view_settings" ) )
        return false;

    if ( !execReply( "INSERT INTO view_settings VALUES ( ?, ?, ? )", "S", reply, database, i ) )
        return false;

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
    QSqlQuery query( database );
    if ( !query.exec( "DELETE FROM projects" ) )
        return false;

    int i = 0;
    if ( !execReply( "INSERT INTO projects VALUES ( ?, ? )", "P", reply, database, i ) )
        return false;

    if ( !query.exec( "DELETE FROM folders" ) )
        return false;

    if ( !execReply( "INSERT INTO folders VALUES ( ?, ?, ?, ?, ? )", "F", reply, database, i ) )
        return false;

    if ( !query.exec( "DELETE FROM alerts" ) )
        return false;

    if ( !execReply( "INSERT INTO alerts VALUES ( ?, ?, ?, ? )", "A", reply, database, i ) )
        return false;

    if ( !recalculateAllAlerts( database ) )
        return false;

    return true;
}

Command* DataManager::updateStates()
{
    int lastStateId = execScalar( "SELECT state_id FROM users_cache WHERE user_id = ?", QSqlDatabase::database(), m_currentUserId ).toInt();

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
    int lastStateId = command->args().at( 0 ).toInt();

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
    AutoSqlQuery insertStateQuery( "INSERT OR REPLACE INTO issue_states VALUES ( ?, ?, ? )", database );

    for ( int i = 0; i < reply.lines().count(); i++ ) {
        const ReplyLine& line = reply.lines().at( i );

        int stateId = line.argInt( 0 );
        if ( stateId > lastStateId )
            lastStateId = stateId;

        insertStateQuery.addBindValue( m_currentUserId );
        insertStateQuery.addBindValue( line.argInt( 1 ) );
        insertStateQuery.addBindValue( line.argInt( 2 ) );
        if ( !insertStateQuery.exec() )
            return false;
    }

    QSqlQuery query( database );
    query.prepare( "INSERT OR REPLACE INTO users_cache VALUES ( ?, ? )" );
    query.addBindValue( m_currentUserId );
    query.addBindValue( lastStateId );
    if ( !query.exec() )
        return false;

    if ( !recalculateAllAlerts( database ) )
        return false;

    return true;
}

Command* DataManager::updateFolder( int folderId )
{
    int lastStampId = execScalar( "SELECT list_id FROM folders_cache WHERE folder_id = ?", QSqlDatabase::database(), folderId ).toInt();

    Command* command = new Command();

    command->setKeyword( "LIST ISSUES" );
    command->addArg( folderId );
    command->addArg( lastStampId );

    command->setAcceptNullReply( true );
    command->addRule( "F iisii", ReplyRule::One );
    command->addRule( "I iisiiiii", ReplyRule::ZeroOrMore );
    command->addRule( "V iis", ReplyRule::ZeroOrMore );
    command->addRule( "X ii", ReplyRule::ZeroOrMore );

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
    const ReplyLine& folderLine = reply.lines().at( 0 );
    int folderId = folderLine.argInt( 0 );
    int lastStampId = folderLine.argInt( 4 );

    updatedFolders.append( folderId );

    QSqlQuery query( database );

    query.prepare( "INSERT OR REPLACE INTO folders VALUES ( ?, ?, ?, ?, ? )" );
    foreach ( const QVariant& arg, folderLine.args() )
        query.addBindValue( arg );
    if ( !query.exec() )
        return false;

    query.prepare( "INSERT OR REPLACE INTO folders_cache VALUES ( ?, ? )" );
    query.addBindValue( folderId );
    query.addBindValue( lastStampId );
    if ( !query.exec() )
        return false;

    AutoSqlQuery oldIssueQuery( "SELECT folder_id FROM issues WHERE issue_id = ?", database );
    AutoSqlQuery deleteValuesQuery( "DELETE FROM attr_values WHERE issue_id = ?", database );
    AutoSqlQuery insertIssueQuery( "INSERT OR REPLACE INTO issues VALUES ( ?, ?, ?, ?, ?, ?, ?, ? )", database );

    int i = 1;
    for ( ; i < reply.lines().count(); i++ ) {
        const ReplyLine& line = reply.lines().at( i );

        if ( line.keyword() != QLatin1String( "I" ) )
            break;

        int issueId = line.argInt( 0 );

        oldIssueQuery.addBindValue( issueId );
        if ( !oldIssueQuery.exec() )
            return false;

        if ( oldIssueQuery.next() ) {
            int oldFolderId = oldIssueQuery.value( 0 ).toInt();

            if ( !updatedFolders.contains( oldFolderId ) )
                updatedFolders.append( oldFolderId );

            deleteValuesQuery.addBindValue( issueId );
            if ( !deleteValuesQuery.exec() )
                return false;
        }

        foreach ( const QVariant& arg, line.args() )
            insertIssueQuery.addBindValue( arg );
        if ( !insertIssueQuery.exec() )
            return false;
    }

    if ( !execReply( "INSERT INTO attr_values VALUES ( ?, ?, ? )", "V", reply, database, i ) )
        return false;

    AutoSqlQuery moveIssueQuery( "UPDATE issues SET folder_id = ? WHERE issue_id = ?", database );
    AutoSqlQuery deleteIssueQuery( "DELETE FROM issues WHERE issue_id = ?", database );

    QList<int> deletedIssues;

    for ( ; i < reply.lines().count(); i++ ) {
        const ReplyLine& line = reply.lines().at( i );

        if ( line.keyword() != QLatin1String( "X" ) )
            break;

        int issueId = line.argInt( 0 );
        int toFolderId = line.argInt( 1 );

        if ( toFolderId != 0 ) {
            oldIssueQuery.addBindValue( issueId );
            if ( !oldIssueQuery.exec() )
                return false;

            if ( oldIssueQuery.next() ) {
                int oldFolderId = oldIssueQuery.value( 0 ).toInt();

                if ( oldFolderId != toFolderId ) {
                    if ( !updatedFolders.contains( toFolderId ) )
                        updatedFolders.append( toFolderId );

                    moveIssueQuery.addBindValue( toFolderId );
                    moveIssueQuery.addBindValue( issueId );
                    if ( !moveIssueQuery.exec() )
                        return false;
                }
            }
        } else {
            deletedIssues.append( issueId );

            deleteIssueQuery.addBindValue( issueId );
            if ( !deleteIssueQuery.exec() )
                return false;

            deleteValuesQuery.addBindValue( issueId );
            if ( !deleteValuesQuery.exec() )
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

Command* DataManager::updateIssue( int issueId )
{
    int lastStampId = execScalar( "SELECT details_id FROM issues_cache WHERE issue_id = ?", QSqlDatabase::database(), issueId ).toInt();

    Command* command = new Command();

    command->setKeyword( "GET DETAILS" );
    command->addArg( issueId );
    command->addArg( lastStampId );

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
    const ReplyLine& issueLine = reply.lines().at( 0 );
    issueId = issueLine.argInt( 0 );
    int folderId = issueLine.argInt( 1 );
    int lastStampId = issueLine.argInt( 3 );

    updatedFolders.append( folderId );

    QSqlQuery oldIssueQuery( database );
    oldIssueQuery.prepare( "SELECT folder_id FROM issues WHERE issue_id = ?" );
    oldIssueQuery.addBindValue( issueId );
    if ( !oldIssueQuery.exec() )
        return false;

    if ( oldIssueQuery.next() ) {
        int oldFolderId = oldIssueQuery.value( 0 ).toInt();
        if ( !updatedFolders.contains( oldFolderId ) )
            updatedFolders.append( oldFolderId );

        QSqlQuery deleteValuesQuery( database );
        deleteValuesQuery.prepare( "DELETE FROM attr_values WHERE issue_id = ?" );
        deleteValuesQuery.addBindValue( issueId );
        if ( !deleteValuesQuery.exec() )
            return false;
    }

    QSqlQuery query( database );

    query.prepare( "INSERT OR REPLACE INTO issues VALUES ( ?, ?, ?, ?, ?, ?, ?, ? )" );
    foreach ( const QVariant& arg, issueLine.args() )
        query.addBindValue( arg );
    if ( !query.exec() )
        return false;

    query.prepare( "INSERT OR REPLACE INTO issues_cache VALUES ( ?, ? )" );
    query.addBindValue( issueId );
    query.addBindValue( lastStampId );
    if ( !query.exec() )
        return false;

    query.prepare( "INSERT OR REPLACE INTO issue_states VALUES ( ?, ?, ? )" );
    query.addBindValue( m_currentUserId );
    query.addBindValue( issueId );
    query.addBindValue( lastStampId );
    if ( !query.exec() )
        return false;

    int i = 1;
    if ( !execReply( "INSERT INTO attr_values VALUES ( ?, ?, ? )", "V", reply, database, i ) )
        return false;

    if ( !execReply( "INSERT OR REPLACE INTO changes VALUES ( ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ? )", "H", reply, database, i ) )
        return false;

    if ( !execReply( "INSERT OR REPLACE INTO comments VALUES ( ?, ? )", "C", reply, database, i ) )
        return false;

    if ( !execReply( "INSERT OR REPLACE INTO files VALUES ( ?, ?, ?, ? )", "A", reply, database, i ) )
        return false;

    QSqlQuery oldChangeQuery( "SELECT change_type FROM changes WHERE change_id = ?", database );
    QSqlQuery deleteCommentQuery( "DELETE FROM comments WHERE comment_id = ?", database );
    QSqlQuery deleteFileQuery( "DELETE FROM files WHERE file_id = ?", database );
    QSqlQuery deleteChangeQuery( "DELETE FROM changes WHERE change_id = ?", database );

    for ( ; i < reply.lines().count(); i++ ) {
        const ReplyLine& line = reply.lines().at( i );

        if ( line.keyword() != QLatin1String( "X" ) )
            break;

        int changeId = line.argInt( 0 );

        oldChangeQuery.addBindValue( changeId );
        if ( !oldChangeQuery.exec() )
            return false;

        if ( oldChangeQuery.next() ) {
            int changeType = oldChangeQuery.value( 0 ).toInt();

            if ( changeType == CommentAdded ) {
                deleteCommentQuery.addBindValue( changeId );
                if ( !deleteCommentQuery.exec() )
                    return false;
            } else if ( changeType == FileAdded ) {
                deleteFileQuery.addBindValue( changeId );
                if ( !deleteFileQuery.exec() )
                    return false;
            }

            deleteChangeQuery.addBindValue( changeId );
            if ( !deleteChangeQuery.exec() )
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
    QSqlQuery query( database );
    query.prepare( "UPDATE issue_locks SET lock_count = lock_count + 1 WHERE issue_id = ?" );
    query.addBindValue( issueId );
    if ( !query.exec() )
        return false;

    if ( query.numRowsAffected() == 0 ) {
        query.prepare( "INSERT INTO issue_locks VALUES ( ?, 1, strftime( '%s', 'now' ) )" );
        query.addBindValue( issueId );
        if ( !query.exec() )
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
    QSqlQuery query( database );
    query.prepare( "UPDATE issue_locks SET lock_count = lock_count - 1, last_access = strftime( '%s', 'now' ) WHERE issue_id = ?" );
    query.addBindValue( issueId );
    if ( !query.exec() )
        return false;

    if ( !flushIssueDetails( database ) )
        return false;

    return true;
}

bool DataManager::clearIssueLocks( const QSqlDatabase& database )
{
    QSqlQuery query( database );
    query.prepare( "UPDATE issue_locks SET lock_count = 0" );
    if ( !query.exec() )
        return false;

    if ( !flushIssueDetails( database ) )
        return false;

    return true;
}

bool DataManager::flushIssueDetails( const QSqlDatabase& database )
{
    int count = execScalar( "SELECT COUNT(*) FROM issue_locks", database ).toInt();

    if ( count > 100 ) {
        QSqlQuery query( database );
        query.prepare( "SELECT issue_id FROM issue_locks WHERE lock_count = 0 ORDER BY last_access LIMIT ?" );
        query.addBindValue( count - 100 );
        if ( !query.exec() )
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
    QSqlQuery query( database );

    query.prepare( "DELETE FROM changes WHERE issue_id = ?" );
    foreach ( int issueId, issues ) {
        query.addBindValue( issueId );
        if ( !query.exec() )
            return false;
    }

    query.prepare( "DELETE FROM comments WHERE issue_id = ?" );
    foreach ( int issueId, issues ) {
        query.addBindValue( issueId );
        if ( !query.exec() )
            return false;
    }

    query.prepare( "DELETE FROM files WHERE issue_id = ?" );
    foreach ( int issueId, issues ) {
        query.addBindValue( issueId );
        if ( !query.exec() )
            return false;
    }

    query.prepare( "DELETE FROM issue_locks WHERE issue_id = ?" );
    foreach ( int issueId, issues ) {
        query.addBindValue( issueId );
        if ( !query.exec() )
            return false;
    }

    return true;
}

QString DataManager::findFilePath( int fileId ) const
{
    QSqlDatabase database = QSqlDatabase::database();

    QString path = execScalar( "SELECT file_path FROM files_cache WHERE file_id = ?", database, fileId ).toString();

    if ( !path.isEmpty() ) {
        QSqlQuery query( database );
        query.prepare( "UPDATE files_cache SET last_access = strftime( '%s', 'now' ) WHERE file_id = ?" );
        query.addBindValue( fileId );
        query.exec();
    }

    return path;
}

QString DataManager::generateFilePath( const QString& name ) const
{
    QString path = application->locateTempFile( name );

    QFileInfo info( path );

    if ( !info.exists() )
        return path;

    QString baseName = info.baseName();
    QString suffix = info.completeSuffix();
    if ( !suffix.isEmpty() )
        suffix.prepend( '.' );

    for ( int number = 2; ; number++ ) {
        QString generatedName = QString( "%1(%2)%3" ).arg( baseName ).arg( number ).arg( suffix );
        path = application->locateTempFile( generatedName );
        if ( !QFile::exists( path ) )
            return path;
    }
}

void DataManager::allocFileSpace( int size )
{
    QSqlDatabase database = QSqlDatabase::database();
    database.transaction();

    bool ok = flushFileCache( size, database );
    if ( ok )
        ok = database.commit();

    if ( !ok )
        database.rollback();
}

void DataManager::commitFile( int fileId, const QString& path, int size )
{
    QSqlDatabase database = QSqlDatabase::database();

    QSqlQuery query( database );
    query.prepare( "INSERT INTO files_cache VALUES ( ?, ?, ?, strftime( '%s', 'now' ) )" );
    query.addBindValue( fileId );
    query.addBindValue( path );
    query.addBindValue( size );
    query.exec();
}

bool DataManager::flushFileCache( int allocatedSize, const QSqlDatabase& database )
{
    LocalSettings* settings = application->applicationSettings();
    int limit = settings->value( "AttachmentsCacheSize" ).toInt() * 1024 * 1024;

    int occupied = ( allocatedSize + 4095 ) & ~4096;
    occupied += execScalar( "SELECT SUM( ( file_size + 4095 ) & ~4096 ) FROM files_cache", database ).toInt();

    if ( occupied > limit ) {
        QList<int> deletedFiles;

        QSqlQuery query( database );
        if ( !query.exec( "SELECT file_id, file_path, file_size FROM files_cache ORDER BY last_access" ) )
            return false;

        while ( query.next() && occupied > limit ) {
            QString path = query.value( 1 ).toString();
            if ( !QFile::exists( path ) || QFile::remove( path ) ) {
                occupied -= ( query.value( 2 ).toInt() + 4095 ) & ~4096;
                deletedFiles.append( query.value( 0 ).toInt() );
            }
        }

        AutoSqlQuery deleteFileQuery( "DELETE FROM files_cache WHERE file_id = ?", database );

        foreach ( int fileId, deletedFiles ) {
            deleteFileQuery.addBindValue( fileId );
            if ( !deleteFileQuery.exec() )
                return false;
        }
    }

    return true;
}

bool DataManager::recalculateAllAlerts( const QSqlDatabase& database )
{
    QSqlQuery query( database );
    if ( !query.exec( "DELETE FROM alerts_cache" ) )
        return false;

    if ( !query.exec( "SELECT a.alert_id, f.folder_id, a.view_id FROM alerts AS a JOIN folders AS f ON f.folder_id = a.folder_id" ) )
        return false;

    while ( query.next() ) {
        if ( !recalculateAlert( query.value( 0 ).toInt(), query.value( 1 ).toInt(), query.value( 2 ).toInt(), database ) )
            return false;
    }

    return true;
}

bool DataManager::recalculateAlerts( int folderId, const QSqlDatabase& database )
{
    QSqlQuery query( database );
    query.prepare( "DELETE FROM alerts_cache WHERE alert_id IN ( SELECT alert_id FROM alerts WHERE folder_id = ? )" );
    query.addBindValue( folderId );
    if ( !query.exec() )
        return false;

    query.prepare( "SELECT a.alert_id, f.folder_id, a.view_id FROM alerts AS a JOIN folders AS f ON f.folder_id = a.folder_id WHERE f.folder_id = ?" );
    query.addBindValue( folderId );
    if ( !query.exec() )
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

    QSqlQuery sqlQuery( database );
    sqlQuery.prepare( sql );

    foreach ( const QVariant& arg, generator.arguments() )
        sqlQuery.addBindValue( arg );

    if ( !sqlQuery.exec() )
        return false;

    int total = 0;
    int modified = 0;
    int unread = 0;

    while ( sqlQuery.next() ) {
        int readId = sqlQuery.value( 2 ).toInt();
        if ( readId == 0 ) {
            unread++;
        } else {
            int stampId = sqlQuery.value( 1 ).toInt();
            if ( readId < stampId )
                modified++;
        }
        total++;
    }

    sqlQuery.prepare( "INSERT INTO alerts_cache VALUES ( ?, ?, ?, ? )" );
    sqlQuery.addBindValue( alertId );
    sqlQuery.addBindValue( total );
    sqlQuery.addBindValue( modified );
    sqlQuery.addBindValue( unread );
    if ( !sqlQuery.exec() )
        return false;

    return true;
}
