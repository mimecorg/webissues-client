/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2014 WebIssues Team
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

#include "finditembatch.h"
#include "command.h"

#include "data/datamanager.h"

FindItemBatch::FindItemBatch( int itemId ) : AbstractBatch( 0 ),
    m_issueId( 0 ),
    m_update( false )
{
    Job job( &FindItemBatch::findItemJob );
    job.addArg( itemId );
    m_queue.addJob( job );
}

FindItemBatch::~FindItemBatch()
{
}

Command* FindItemBatch::fetchNext()
{
    if ( m_queue.moreJobs() )
        return m_queue.callJob( this );

    if ( m_update ) {
        m_update = false;
        if ( dataManager->issueUpdateNeeded( m_issueId ) )
            return dataManager->updateIssue( m_issueId, true );
    }

    return NULL;
}

Command* FindItemBatch::findItemJob( const Job& job )
{
    Command* command = new Command();

    command->setKeyword( "FIND ITEM" );
    command->addArg( job.argInt( 0 ) );

    command->addRule( "ID i", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( findItemReply( const Reply& ) ) );

    return command;
}

void FindItemBatch::findItemReply( const Reply& reply )
{
    ReplyLine line = reply.lines().at( 0 );

    m_issueId = line.argInt( 0 );
    m_update = true;
}
