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
#include "data/localecache.h"
#include "data/attachmentscache.h"
#include "data/localsettings.h"
#include "models/rowfilters.h"
#include "utils/dataserializer.h"
#include "utils/viewsettingshelper.h"

#include <QApplication>
#include <QFile>

DataManager* dataManager = NULL;

DataManager::DataManager() :
    m_currentUserId( 0 ),
    m_currentUserAccess( NoAccess ),
    m_stateCached( false ),
    m_lastStateId( 0 ),
    m_connectionSettings( NULL )
{
    m_users.initIndex( 251, &UserRow::userId );
    m_members.initFirstIndex( 251, &MemberRow::userId );
    m_members.initSecondIndex( 31, &MemberRow::projectId );
    m_types.initIndex( 31, &TypeRow::typeId );
    m_attributes.initIndex( 251, &AttributeRow::attributeId );
    m_attributes.initParentIndex( 31, &AttributeRow::typeId );
    m_projects.initIndex( 31, &ProjectRow::projectId );
    m_folders.initIndex( 127, &FolderRow::folderId );
    m_folders.initParentIndex( 31, &FolderRow::projectId );
    m_issues.initIndex( 8191, &IssueRow::issueId );
    m_issues.initParentIndex( 1021, &IssueRow::folderId );
    m_values.initFirstIndex( 251, &ValueRow::attributeId );
    m_values.initSecondIndex( 8191, &ValueRow::issueId );
    m_changes.initIndex( 1021, &ChangeRow::changeId );
    m_changes.initParentIndex( 127, &ChangeRow::issueId );
    m_comments.initIndex( 251, &CommentRow::commentId );
    m_files.initIndex( 251, &FileRow::fileId );
    m_views.initIndex( 251, &ViewRow::viewId );
    m_views.initParentIndex( 31, &ViewRow::typeId );
    m_alerts.initIndex( 251, &AlertRow::alertId );
    m_alerts.initParentIndex( 31, &AlertRow::folderId );

    m_folderStates.initIndex( 127, &FolderState::folderId );
    m_issueStates.initIndex( 8191, &IssueState::issueId );
    m_alertStates.initIndex( 251, &AlertState::alertId );

    m_localeCache = new LocaleCache( this );
}

DataManager::~DataManager()
{
    saveFolderCache();
    saveStateCache();
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

IssueState* DataManager::issueState( int issueId )
{
    IssueState* state = m_issueStates.find( issueId );
    if ( !state ) {
        state = new IssueState( issueId );
        m_issueStates.insert( state );
    }
    return state;
}

FolderState* DataManager::folderState( int folderId )
{
    FolderState* state = m_folderStates.find( folderId );
    if ( !state ) {
        state = new FolderState( folderId );
        m_folderStates.insert( state );
    }
    return state;
}

AlertState* DataManager::alertState( int alertId )
{
    AlertState* state = m_alertStates.find( alertId );
    if ( !state ) {
        state = new AlertState( alertId );
        m_alertStates.insert( state );
    }
    return state;
}

bool DataManager::folderUpdateNeeded( int folderId )
{
    updateFolderCache( folderId );

    int folderStamp = 0;
    int listStamp = 0;

    FolderRow* row = m_folders.find( folderId );
    if ( row )
        folderStamp = row->stamp();

    FolderState* state = m_folderStates.find( folderId );
    if ( state )
        listStamp = state->listStamp();

    return ( folderStamp == 0 || folderStamp > listStamp );
}

bool DataManager::issueUpdateNeeded( int issueId )
{
    int issueStamp = 0;
    int detailsStamp = 0;

    IssueRow* row = m_issues.find( issueId );
    if ( row )
        issueStamp = row->stamp();

    IssueState* state = m_issueStates.find( issueId );
    if ( state )
        detailsStamp = state->detailsStamp();

    return ( issueStamp == 0 || issueStamp > detailsStamp );
}

void DataManager::lockIssue( int issueId )
{
    IssueState* state = issueState( issueId );

    state->setLockCount( state->lockCount() + 1 );
}

void DataManager::unlockIssue( int issueId )
{
    IssueState* state = issueState( issueId );

    state->setLockCount( state->lockCount() - 1 );
    state->setTimeUsed( QDateTime::currentDateTime() );

    flushIssueCache();
}

void DataManager::flushIssueCache()
{
    for ( ; ; ) {
        int cacheSize = 0;

        int oldestIssueId = 0;
        QDateTime oldestTime = QDateTime::currentDateTime().addSecs( 1 );

        RDB::IndexIterator<IssueState> it( m_issueStates.index() );
        while ( it.next() ) {
            IssueState* state = it.get();
            if ( state->detailsStamp() != 0 ) {
                cacheSize++;
                if ( state->lockCount() == 0 ) {
                    QDateTime time = state->timeUsed();
                    if ( time.isNull() || time < oldestTime ) {
                        oldestIssueId = state->issueId();
                        oldestTime = time;
                    }
                }
            }
        }

        if ( cacheSize <= 20 || oldestIssueId == 0 )
            break;

        removeIssueDetails( oldestIssueId );
    }
}

void DataManager::removeIssueDetails( int issueId )
{
    RDB::ForeignConstIterator<ChangeRow> itc( m_changes.parentIndex(), issueId );
    while ( itc.next() ) {
        const ChangeRow* change = itc.get();
        if ( change->changeType() == CommentAdded )
            m_comments.remove( change->changeId() );
        else if ( change->changeType() == FileAdded )
            m_files.remove( change->changeId() );
    }

    m_changes.removeChildren( issueId );

    IssueState* state = issueState( issueId );
    state->setDetailsStamp( 0 );

    notifyObservers( UpdateEvent::Issue, issueId );
}

int DataManager::issueReadStamp( int issueId )
{
    return issueState( issueId )->readStamp();
}

QString DataManager::setting( const QString& key ) const
{
    return m_settings.value( key );
}

QString DataManager::viewSetting( int typeId, const QString& key ) const
{
    return m_viewSettings.value( qMakePair( typeId, key ) );
}

void DataManager::recalculateAllAlerts()
{
    RDB::IndexConstIterator<FolderRow> it( m_folders.index() );
    while ( it.next() )
        recalculateAlerts( it.key() );
}

void DataManager::recalculateAlerts( int folderId )
{
    const FolderRow* folder = m_folders.find( folderId );
    if ( !folder )
        return;

    int typeId = folder->typeId();

    RDB::ForeignConstIterator<AlertRow> ita( m_alerts.parentIndex(), folderId );
    while ( ita.next() ) {
        int alertId = ita.key( 0 );
        int viewId = ita.get()->viewId();

        IssueRowFilter filter( this );

        if ( viewId ) {
            const ViewRow* view = m_views.find( viewId );
            if ( view ) {
                DefinitionInfo info = DefinitionInfo::fromString( view->definition() );
                QList<DefinitionInfo> filters = ViewSettingsHelper::viewFilters( typeId, info );
                filter.setFilters( filters );
            }
        }

        int unread = 0;
        int modified = 0;
        int total = 0;

        RDB::ForeignConstIterator<IssueRow> iti( m_issues.parentIndex(), folderId );
        while ( iti.next() ) {
            if ( filter.filterRow( iti.key( 0 ) ) ) {
                int readStamp = issueState( iti.key( 0 ) )->readStamp();
                if ( readStamp == 0 )
                    unread++;
                else if ( readStamp < iti.get()->stamp() )
                    modified++;
                total++;
            }
        }

        AlertState* state = alertState( alertId );
        state->setUnreadCount( unread );
        state->setModifiedCount( modified );
        state->setTotalCount( total );
    }
}

void DataManager::getAlertIssuesCount( int alertId, int* unread, int* modified, int* total )
{
    AlertState* state = alertState( alertId );
    *unread = state->unreadCount();
    *modified = state->modifiedCount();
    *total = state->totalCount();
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
    m_attachmentsCache = new AttachmentsCache( locateCacheFile( "attachments.cache" ), this );
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
    m_settings.clear();

    for ( int i = 0; i < reply.lines().count(); i++ ) {
        ReplyLine line = reply.lines().at( i );
        m_settings.insert( line.argString( 0 ), line.argString( 1 ) );
    }

    notifyObservers( UpdateEvent::Settings );
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
    m_users.clear();
    m_members.clear();

    for ( int i = 0; i < reply.lines().count(); i++ ) {
        ReplyLine line = reply.lines().at( i );
        if ( line.keyword() == QLatin1String( "U" ) ) {
            UserRow* user = readUserRow( line );
            m_users.insert( user );
        } else { // "M"
            MemberRow* member = readMemberRow( line );
            m_members.insert( member );
        }
    }

    notifyObservers( UpdateEvent::Users );
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
    m_types.clear();
    m_attributes.clear();
    m_views.clear();
    m_viewSettings.clear();

    for ( int i = 0; i < reply.lines().count(); i++ ) {
        ReplyLine line = reply.lines().at( i );
        if ( line.keyword() == QLatin1String( "T" ) ) {
            TypeRow* type = readTypeRow( line );
            m_types.insert( type );
        } else if ( line.keyword() == QLatin1String( "A" ) ) {
            AttributeRow* attribute = readAttributeRow( line );
            m_attributes.insert( attribute );
        } else if ( line.keyword() == QLatin1String( "V" ) ) {
            ViewRow* view = readViewRow( line );
            m_views.insert( view );
        } else { // "S"
            m_viewSettings.insert( qMakePair( line.argInt( 0 ), line.argString( 1 ) ), line.argString( 2 ) );
        }
    }

    recalculateAllAlerts();

    notifyObservers( UpdateEvent::Types );
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
    m_projects.clear();
    m_folders.clear();
    m_alerts.clear();

    for ( int i = 0; i < reply.lines().count(); i++ ) {
        ReplyLine line = reply.lines().at( i );
        if ( line.keyword() == QLatin1String( "P" ) ) {
            ProjectRow* project = readProjectRow( line );
            m_projects.insert( project );
        } else if ( line.keyword() == QLatin1String( "F" ) ) {
            FolderRow* folder = readFolderRow( line );
            m_folders.insert( folder );
        } else { // "A"
            AlertRow* alert = readAlertRow( line );
            m_alerts.insert( alert );
        }
    }

    recalculateAllAlerts();

    notifyObservers( UpdateEvent::Projects );
}

Command* DataManager::updateStates()
{
    updateStateCache();

    Command* command = new Command();

    command->setKeyword( "LIST STATES" );
    command->addArg( m_lastStateId );

    command->setAcceptNullReply( true );
    command->addRule( "S iii", ReplyRule::ZeroOrMore );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( updateStatesReply( const Reply& ) ) );

    return command;
}

void DataManager::updateStatesReply( const Reply& reply )
{
    for ( int i = 0; i < reply.lines().count(); i++ ) {
        ReplyLine line = reply.lines().at( i );
        int stateId = line.argInt( 0 );
        int issueId = line.argInt( 1 );
        int stamp = line.argInt( 2 );

        if ( stateId > m_lastStateId )
            m_lastStateId = stateId;

        IssueState* state = issueState( issueId );
        state->setReadStamp( stamp );
    }

    recalculateAllAlerts();

    notifyObservers( UpdateEvent::States );
}

Command* DataManager::updateFolder( int folderId )
{
    updateFolderCache( folderId );

    FolderState* state = m_folderStates.find( folderId );
    int stamp = state ? state->listStamp() : 0;

    Command* command = new Command();

    command->setKeyword( "LIST ISSUES" );
    command->addArg( folderId );
    command->addArg( stamp );

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
    ReplyLine folderLine = reply.lines().at( 0 );

    FolderRow* folder = readFolderRow( folderLine );
    int folderId = folder->folderId();

    QList<int> folders;
    folders.append( folderId );

    m_folders.remove( folderId );
    m_folders.insert( folder );

    FolderState* state = folderState( folderId );
    state->setListStamp( folder->stamp() );

    for ( int i = 1; i < reply.lines().count(); i++ ) {
        ReplyLine line = reply.lines().at( i );
        if ( line.keyword() == QLatin1String( "I" ) ) {
            IssueRow* issue = readIssueRow( line );
            int issueId = issue->issueId();

            const IssueRow* oldIssue = m_issues.find( issueId );
            if ( oldIssue ) {
                if ( !folders.contains( oldIssue->folderId() ) )
                    folders.append( oldIssue->folderId() );

                m_issues.remove( issueId );
                m_values.removeSecond( issueId );
            }

            m_issues.insert( issue );
        } else if ( line.keyword() == QLatin1String( "V" ) ) {
            ValueRow* value = readValueRow( line );
            m_values.insert( value );
        } else { // "X"
            int issueId = line.argInt( 0 );
            int toFolderId = line.argInt( 1 );

            const IssueRow* issue = m_issues.find( issueId );
            if ( issue ) {
                if ( toFolderId != 0 ) {
                    if ( issue->folderId() != toFolderId ) {
                        IssueRow* newIssue = new IssueRow( issueId, toFolderId, issue->name(), issue->stamp(), issue->createdDate(), issue->createdUser(),
                            issue->modifiedDate(), issue->modifiedUser() );

                        if ( !folders.contains( toFolderId ) )
                            folders.append( toFolderId );

                        m_issues.remove( issueId );
                        m_issues.insert( newIssue );
                    }
                } else {
                    m_issues.remove( issueId );
                    m_values.removeSecond( issueId );

                    removeIssueDetails( issueId );
                }
            }
        }
    }

    foreach ( int folderId, folders )
        recalculateAlerts( folderId );

    foreach ( int folderId, folders )
        notifyObservers( UpdateEvent::Folder, folderId );

    notifyObservers( UpdateEvent::AlertStates );
}

Command* DataManager::updateIssue( int issueId )
{
    IssueState* state = m_issueStates.find( issueId );
    int stamp = state ? state->detailsStamp() : 0;

    Command* command = new Command();

    command->setKeyword( "GET DETAILS" );
    command->addArg( issueId );
    command->addArg( stamp );

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
    ReplyLine issueLine = reply.lines().at( 0 );

    IssueRow* issue = readIssueRow( issueLine );
    int issueId = issue->issueId();

    QList<int> folders;
    folders.append( issue->folderId() );

    const IssueRow* oldIssue = m_issues.find( issueId );
    if ( oldIssue ) {
        if ( !folders.contains( oldIssue->folderId() ) )
            folders.append( oldIssue->folderId() );

        m_issues.remove( issueId );
        m_values.removeSecond( issueId );
    }

    m_issues.insert( issue );

    IssueState* state = issueState( issueId );
    state->setDetailsStamp( issue->stamp() );
    state->setReadStamp( issue->stamp() );

    for ( int i = 1; i < reply.lines().count(); i++ ) {
        ReplyLine line = reply.lines().at( i );
        if ( line.keyword() == QLatin1String( "V" ) ) {
            ValueRow* value = readValueRow( line );
            m_values.insert( value );
        } else if ( line.keyword() == QLatin1String( "H" ) ) {
            ChangeRow* change = readChangeRow( line );
            m_changes.remove( change->changeId() );
            m_changes.insert( change );
        } else if ( line.keyword() == QLatin1String( "C" ) ) {
            CommentRow* comment = readCommentRow( line );
            m_comments.remove( comment->commentId() );
            m_comments.insert( comment );
        } else if ( line.keyword() == QLatin1String( "A" ) ) {
            FileRow* file = readFileRow( line );
            m_files.remove( file->fileId() );
            m_files.insert( file );
        } else { // "X"
            int changeId = line.argInt( 0 );
            const ChangeRow* change = m_changes.find( changeId );
            if ( change ) {
                if ( change->changeType() == CommentAdded )
                    m_comments.remove( changeId );
                else if ( change->changeType() == FileAdded )
                    m_files.remove( changeId );
                m_changes.remove( changeId );
            }
        }
    }

    foreach ( int folderId, folders )
        recalculateAlerts( folderId );

    foreach ( int folderId, folders )
        notifyObservers( UpdateEvent::IssueList, folderId );

    notifyObservers( UpdateEvent::Issue, issueId );

    notifyObservers( UpdateEvent::AlertStates );

    flushIssueCache();
}

UserRow* DataManager::readUserRow( const ReplyLine& line )
{
    int userId = line.argInt( 0 );
    QString login = line.argString( 1 );
    QString name = line.argString( 2 );
    Access access = (Access)line.argInt( 3 );

    return new UserRow( userId, login, name, access );
}

MemberRow* DataManager::readMemberRow( const ReplyLine& line )
{
    int userId = line.argInt( 0 );
    int projectId = line.argInt( 1 );
    Access access = (Access)line.argInt( 2 );

    return new MemberRow( userId, projectId, access );
}

TypeRow* DataManager::readTypeRow( const ReplyLine& line )
{
    int typeId = line.argInt( 0 );
    QString name = line.argString( 1 );

    return new TypeRow( typeId, name );
}

AttributeRow* DataManager::readAttributeRow( const ReplyLine& line )
{
    int attributeId = line.argInt( 0 );
    int typeId = line.argInt( 1 );
    QString name = line.argString( 2 );
    QString definition = line.argString( 3 );

    return new AttributeRow( attributeId, typeId, name, definition );
}

ProjectRow* DataManager::readProjectRow( const ReplyLine& line )
{
    int projectId = line.argInt( 0 );
    QString name = line.argString( 1 );

    return new ProjectRow( projectId, name );
}

FolderRow* DataManager::readFolderRow( const ReplyLine& line )
{
    int folderId = line.argInt( 0 );
    int projectId = line.argInt( 1 );
    QString name = line.argString( 2 );
    int typeId = line.argInt( 3 );
    int stamp = line.argInt( 4 );

    return new FolderRow( folderId, projectId, name, typeId, stamp );
}

IssueRow* DataManager::readIssueRow( const ReplyLine& line )
{
    int issueId = line.argInt( 0 );
    int folderId = line.argInt( 1 );
    QString name = line.argString( 2 );
    int stamp = line.argInt( 3 );
    QDateTime createdDate;
    createdDate.setTime_t( line.argInt( 4 ) );
    int createdUser = line.argInt( 5 );
    QDateTime modifiedDate;
    modifiedDate.setTime_t( line.argInt( 6 ) );
    int modifiedUser = line.argInt( 7 );

    return new IssueRow( issueId, folderId, name, stamp, createdDate, createdUser, modifiedDate, modifiedUser );
}

ValueRow* DataManager::readValueRow( const ReplyLine& line )
{
    int attributeId = line.argInt( 0 );
    int issueId = line.argInt( 1 );
    QString value = line.argString( 2 );

    return new ValueRow( attributeId, issueId, value );
}

ChangeRow* DataManager::readChangeRow( const ReplyLine& line )
{
    int changeId = line.argInt( 0 );
    int issueId = line.argInt( 1 );
    ChangeType changeType = (ChangeType)line.argInt( 2 );
    int stamp = line.argInt( 3 );
    QDateTime createdDate;
    createdDate.setTime_t( line.argInt( 4 ) );
    int createdUser = line.argInt( 5 );
    QDateTime modifiedDate;
    modifiedDate.setTime_t( line.argInt( 6 ) );
    int modifiedUser = line.argInt( 7 );
    int attributeId = line.argInt( 8 );
    QString oldValue = line.argString( 9 );
    QString newValue = line.argString( 10 );
    int fromFolder = line.argInt( 11 );
    int toFolder = line.argInt( 12 );

    return new ChangeRow( changeId, issueId, changeType, stamp, createdDate, createdUser, modifiedDate, modifiedUser,
        attributeId, oldValue, newValue, fromFolder, toFolder );
}

CommentRow* DataManager::readCommentRow( const ReplyLine& line )
{
    int commentId = line.argInt( 0 );
    QString text = line.argString( 1 );

    return new CommentRow( commentId, text );
}

FileRow* DataManager::readFileRow( const ReplyLine& line )
{
    int fileId = line.argInt( 0 );
    QString name = line.argString( 1 );
    int size = line.argInt( 2 );
    QString description = line.argString( 3 );

    return new FileRow( fileId, name, size, description );
}

ViewRow* DataManager::readViewRow( const ReplyLine& line )
{
    int viewId = line.argInt( 0 );
    int typeId = line.argInt( 1 );
    QString name = line.argString( 2 );
    QString definition = line.argString( 3 );
    bool isPublic = line.argInt( 4 ) != 0;

    return new ViewRow( viewId, typeId, name, definition, isPublic );
}

AlertRow* DataManager::readAlertRow( const ReplyLine& line )
{
    int alertId = line.argInt( 0 );
    int folderId = line.argInt( 1 );
    int viewId = line.argInt( 2 );
    AlertEmail alertEmail = (AlertEmail)line.argInt( 3 );

    return new AlertRow( alertId, folderId, viewId, alertEmail );
}

int DataManager::findItem( int itemId )
{
    const IssueRow* issue = m_issues.find( itemId );
    if ( issue != NULL )
        return itemId;

    const ChangeRow* change = m_changes.find( itemId );
    if ( change != NULL && ( change->changeType() == CommentAdded || change->changeType() == FileAdded ) )
        return change->issueId();

    return 0;
}

QDataStream& operator <<( QDataStream& stream, const IssueRow* row )
{
    return stream
        << (qint32)row->issueId()
        << (qint32)row->folderId()
        << row->name()
        << (qint32)row->stamp()
        << row->createdDate()
        << (qint32)row->createdUser()
        << row->modifiedDate()
        << (qint32)row->modifiedUser();
}

QDataStream& operator >>( QDataStream& stream, IssueRow*& row )
{
    qint32 issueId, folderId, stamp, createdUser, modifiedUser;
    QString name;
    QDateTime createdDate, modifiedDate;

    stream >> issueId >> folderId >> name >> stamp >> createdDate >> createdUser >> modifiedDate >> modifiedUser;

    if ( !dataManager->issues()->find( issueId ) )
        row = new IssueRow( issueId, folderId, name, stamp, createdDate, createdUser, modifiedDate, modifiedUser );
    else
        row = NULL;

    return stream;
}

QDataStream& operator <<( QDataStream& stream, const ValueRow* row )
{
    return stream
        << (qint32)row->attributeId()
        << (qint32)row->issueId()
        << row->value();
}

QDataStream& operator >>( QDataStream& stream, ValueRow*& row )
{
    qint32 attributeId, issueId;
    QString value;

    stream >> attributeId >> issueId >> value;

    if ( !dataManager->issues()->find( issueId ) )
        row = new ValueRow( attributeId, issueId, value );
    else
        row = NULL;

    return stream;
}

void DataManager::updateFolderCache( int folderId )
{
    FolderState* state = folderState( folderId );
    if ( !state->cached() ) {
        readFolderCache( folderId );
        recalculateAlerts( folderId );
        state->setCached( true );
    }
}

void DataManager::readFolderCache( int folderId )
{
    QString name = QString( "folder%1.cache" ).arg( folderId );
    DataSerializer serializer( locateCacheFile( name ) );

    if ( !serializer.openForReading() )
        return;

    qint32 listStamp;
    serializer.stream() >> listStamp;

    FolderState* state = folderState( folderId );
    state->setListStamp( listStamp );

    QList<IssueRow*> issues;
    QList<ValueRow*> values;

    serializer.stream() >> issues >> values;

    foreach ( IssueRow* issue, issues ) {
        if ( issue )
            m_issues.insert( issue );
    }

    foreach ( ValueRow* value, values ) {
        if ( value )
            m_values.insert( value );
    }

    notifyObservers( UpdateEvent::Folder, folderId );
}

void DataManager::saveFolderCache()
{
    RDB::IndexConstIterator<FolderRow> it( m_folders.index() );
    while ( it.next() )
        writeFolderCache( it.key() );
}

void DataManager::writeFolderCache( int folderId )
{
    FolderState* state = folderState( folderId );
    if ( state->listStamp() == 0 )
        return;

    QString name = QString( "folder%1.cache" ).arg( folderId );
    DataSerializer serializer( locateCacheFile( name ) );

    if ( !serializer.openForWriting() )
        return;

    serializer.stream() << (qint32)state->listStamp();

    QList<const IssueRow*> issues;
    QList<const ValueRow*> values;

    RDB::ForeignConstIterator<IssueRow> it( m_issues.parentIndex(), folderId );
    while ( it.next() ) {
        issues.append( it.get() );
        RDB::ForeignConstIterator<ValueRow> it2( m_values.index().second(), it.key( 0 ) );
        while ( it2.next() )
            values.append( it2.get() );
    }

    serializer.stream() << issues << values;
}

struct IssueStateRecord
{
    qint32 m_issueId;
    qint32 m_readStamp;
};

QDataStream& operator <<( QDataStream& stream, const IssueStateRecord& record )
{
    return stream
        << record.m_issueId
        << record.m_readStamp;
}

QDataStream& operator >>( QDataStream& stream, IssueStateRecord& record )
{
    return stream
        >> record.m_issueId
        >> record.m_readStamp;
}

void DataManager::updateStateCache()
{
    if ( m_stateCached )
        return;

    m_stateCached = true;

    QString name = QString( "state%1.cache" ).arg( m_currentUserId );
    DataSerializer serializer( locateCacheFile( name ) );

    if ( !serializer.openForReading() )
        return;

    qint32 lastStateId;
    serializer.stream() >> lastStateId;

    m_lastStateId = lastStateId;

    QList<IssueStateRecord> records;
    serializer.stream() >> records;

    for ( int i = 0; i < records.count(); i++ ) {
        int issueId = records.at( i ).m_issueId;
        int readStamp = records.at( i ).m_readStamp;

        IssueState* state = issueState( issueId );
        state->setReadStamp( readStamp );
    }

    recalculateAllAlerts();

    notifyObservers( UpdateEvent::States );
}

void DataManager::saveStateCache()
{
    QString name = QString( "state%1.cache" ).arg( m_currentUserId );
    DataSerializer serializer( locateCacheFile( name ) );

    if ( !serializer.openForWriting() )
        return;

    serializer.stream() << m_lastStateId;

    QList<IssueStateRecord> records;

    RDB::IndexConstIterator<IssueState> it( m_issueStates.index() );
    while ( it.next() ) {
        const IssueState* state = it.get();
        IssueStateRecord record;
        record.m_issueId = state->issueId();
        record.m_readStamp = state->readStamp();
        records.append( record );
    }

    serializer.stream() << records;
}
