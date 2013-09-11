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

#include "alertsbatch.h"
#include "command.h"

#include "data/datamanager.h"

AlertsBatch::AlertsBatch() : AbstractBatch( 0 ),
    m_update( false )
{
}

AlertsBatch::~AlertsBatch()
{
}

void AlertsBatch::addAlert( int folderId, int viewId, AlertEmail alertEmail, const QString& summaryDays, const QString& summaryHours, bool isPublic )
{
    Job job( &AlertsBatch::addAlertJob );
    job.addArg( folderId );
    job.addArg( viewId );
    job.addArg( (int)alertEmail );
    job.addArg( summaryDays );
    job.addArg( summaryHours );
    job.addArg( isPublic ? 1 : 0 );
    m_queue.addJob( job );
}

void AlertsBatch::addGlobalAlert( int typeId, int viewId, AlertEmail alertEmail, const QString& summaryDays, const QString& summaryHours, bool isPublic )
{
    Job job( &AlertsBatch::addGlobalAlertJob );
    job.addArg( typeId );
    job.addArg( viewId );
    job.addArg( (int)alertEmail );
    job.addArg( summaryDays );
    job.addArg( summaryHours );
    job.addArg( isPublic ? 1 : 0 );
    m_queue.addJob( job );
}

void AlertsBatch::modifyAlert( int alertId, AlertEmail alertEmail, const QString& summaryDays, const QString& summaryHours )
{
    Job job( &AlertsBatch::modifyAlertJob );
    job.addArg( alertId );
    job.addArg( (int)alertEmail );
    job.addArg( summaryDays );
    job.addArg( summaryHours );
    m_queue.addJob( job );
}

void AlertsBatch::deleteAlert( int alertId )
{
    Job job( &AlertsBatch::deleteAlertJob );
    job.addArg( alertId );
    m_queue.addJob( job );
}

Command* AlertsBatch::fetchNext()
{
    if ( m_queue.moreJobs() )
        return m_queue.callJob( this );

    if ( m_update ) {
        m_update = false;
        return dataManager->updateProjects();
    }

    return NULL;
}

Command* AlertsBatch::addAlertJob( const Job& job )
{
    Command* command = new Command();

    command->setKeyword( "ADD ALERT" );
    command->setArgs( job.args() );

    command->addRule( "ID i", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( setUpdate() ) );

    return command;
}

Command* AlertsBatch::addGlobalAlertJob( const Job& job )
{
    Command* command = new Command();

    command->setKeyword( "ADD GLOBAL ALERT" );
    command->setArgs( job.args() );

    command->addRule( "ID i", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( setUpdate() ) );

    return command;
}

Command* AlertsBatch::modifyAlertJob( const Job& job )
{
    Command* command = new Command();

    command->setKeyword( "MODIFY ALERT" );
    command->setArgs( job.args() );

    command->setAcceptNullReply( true );
    command->addRule( "OK", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( setUpdate() ) );

    return command;
}

Command* AlertsBatch::deleteAlertJob( const Job& job )
{
    Command* command = new Command();

    command->setKeyword( "DELETE ALERT" );
    command->setArgs( job.args() );

    command->addRule( "OK", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( setUpdate() ) );

    return command;
}

void AlertsBatch::setUpdate()
{
    m_update = true;
}
