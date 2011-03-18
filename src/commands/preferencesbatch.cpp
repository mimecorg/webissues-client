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

#include "preferencesbatch.h"
#include "command.h"
#include "commandmanager.h"

#include "data/datamanager.h"
#include "data/localecache.h"

LoadPreferencesBatch::LoadPreferencesBatch( int userId ) : AbstractBatch( 0 )
{
    Job job( &LoadPreferencesBatch::listPreferencesJob );
    job.addArg( userId );
    m_queue.addJob( job );
}

LoadPreferencesBatch::~LoadPreferencesBatch()
{
}

Command* LoadPreferencesBatch::fetchNext()
{
    if ( !dataManager->localeCache()->isPopulated() )
        return dataManager->localeCache()->updateLocale();

    if ( m_queue.moreJobs() )
        return m_queue.callJob( this );

    return NULL;
}

Command* LoadPreferencesBatch::listPreferencesJob( const Job& job )
{
    Command* command = new Command();

    command->setKeyword( "LIST PREFERENCES" );
    command->addArg( job.argInt( 0 ) );

    command->setAcceptNullReply( true );
    command->addRule( "P ss", ReplyRule::ZeroOrMore );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( listPreferencesReply( const Reply& ) ) );

    return command;
}

void LoadPreferencesBatch::listPreferencesReply( const Reply& reply )
{
    for ( int i = 0; i < reply.lines().count(); i++ ) {
        ReplyLine line = reply.lines().at( i );
        m_preferences.insert( line.argString( 0 ), line.argString( 1 ) );
    }
}

SavePreferencesBatch::SavePreferencesBatch( int userId, const QMap<QString, QString>& preferences ) : AbstractBatch( 0 ),
    m_userId( userId ),
    m_preferences( preferences ),
    m_iterator( m_preferences ),
    m_update( false )
{
}

SavePreferencesBatch::~SavePreferencesBatch()
{
}

Command* SavePreferencesBatch::fetchNext()
{
    if ( m_iterator.hasNext() ) {
        m_iterator.next();
        return createSetCommand();
    }

    if ( m_update ) {
        m_update = false;
        return dataManager->updateSettings();
    }

    return NULL;
}

Command* SavePreferencesBatch::createSetCommand()
{
    Command* command = new Command();

    command->setKeyword( "SET PREFERENCE" );
    command->addArg( m_userId );
    command->addArg( m_iterator.key() );
    command->addArg( m_iterator.value() );

    command->setAcceptNullReply( true );
    command->addRule( "OK", ReplyRule::One );

    if ( m_userId == dataManager->currentUserId() )
        connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( setUpdate() ) );

    return command;
}

void SavePreferencesBatch::setUpdate()
{
    m_update = true;
}
