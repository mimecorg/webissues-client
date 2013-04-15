/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2012 WebIssues Team
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

#include "commands/command.h"
#include "commands/commandmanager.h"
#include "data/datamanager.h"

SetPreferencesBatch::SetPreferencesBatch( int userId ) : AbstractBatch( 0 ),
    m_userId( userId ),
    m_update( false )
{
}

SetPreferencesBatch::~SetPreferencesBatch()
{
}

void SetPreferencesBatch::setPreference( const QString& key, const QString& value )
{
    Job job( &SetPreferencesBatch::setPreferenceJob );
    job.addArg( key );
    job.addArg( value );
    m_queue.addJob( job );
}

Command* SetPreferencesBatch::fetchNext()
{
    while ( m_queue.moreJobs() )
        return m_queue.callJob( this );

    if ( m_update ) {
        m_update = false;
        return dataManager->updateUsers();
    }

    return NULL;
}

Command* SetPreferencesBatch::setPreferenceJob( const Job& job )
{
    Command* command = new Command();

    command->setKeyword( "SET PREFERENCE" );
    command->addArg( m_userId );
    command->addArg( job.argString( 0 ) );
    command->addArg( job.argString( 1 ) );

    command->setAcceptNullReply( true );
    command->addRule( "OK", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( setUpdate() ) );

    return command;
}

void SetPreferencesBatch::setUpdate()
{
    m_update = true;
}
