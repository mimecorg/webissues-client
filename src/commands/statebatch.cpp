/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2017 WebIssues Team
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

#include "statebatch.h"
#include "command.h"

#include "data/datamanager.h"

StateBatch::StateBatch() : AbstractBatch( 0 ),
    m_update( false )
{
}

StateBatch::~StateBatch()
{
}

void StateBatch::setIssueRead( int issueId, int readId )
{
    Job job( &StateBatch::setIssueReadJob );
    job.addArg( issueId );
    job.addArg( readId );
    m_queue.addJob( job );
}

void StateBatch::setFolderRead( int folderId, int readId )
{
    Job job( &StateBatch::setFolderReadJob );
    job.addArg( folderId );
    job.addArg( readId );
    m_queue.addJob( job );
}

void StateBatch::addSubscription( int issueId )
{
    Job job( &StateBatch::addSubscriptionJob );
    job.addArg( issueId );
    m_queue.addJob( job );
}

void StateBatch::deleteSubscription( int subscriptionId )
{
    Job job( &StateBatch::deleteSubscriptionJob );
    job.addArg( subscriptionId );
    m_queue.addJob( job );
}

Command* StateBatch::fetchNext()
{
    if ( m_queue.moreJobs() )
        return m_queue.callJob( this );

    if ( m_update ) {
        m_update = false;
        return dataManager->updateStates();
    }

    return NULL;
}

Command* StateBatch::setIssueReadJob( const Job& job )
{
    Command* command = new Command();

    command->setKeyword( "SET ISSUE READ" );
    command->setArgs( job.args() );

    command->setAcceptNullReply( true );
    command->addRule( "ID i", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( setUpdate() ) );

    return command;
}

Command* StateBatch::setFolderReadJob( const Job& job )
{
    Command* command = new Command();

    command->setKeyword( "SET FOLDER READ" );
    command->setArgs( job.args() );

    command->setAcceptNullReply( true );
    command->addRule( "OK", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( setUpdate() ) );

    return command;
}

Command* StateBatch::addSubscriptionJob( const Job& job )
{
    Command* command = new Command();

    command->setKeyword( "ADD SUBSCRIPTION" );
    command->setArgs( job.args() );

    command->addRule( "ID i", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( setUpdate() ) );

    return command;
}

Command* StateBatch::deleteSubscriptionJob( const Job& job )
{
    Command* command = new Command();

    command->setKeyword( "DELETE SUBSCRIPTION" );
    command->setArgs( job.args() );

    command->addRule( "OK", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( setUpdate() ) );

    return command;
}

void StateBatch::setUpdate()
{
    m_update = true;
}
