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

#include "loginbatch.h"
#include "command.h"
#include "commandmanager.h"

#include "data/datamanager.h"

LoginBatch::LoginBatch() : AbstractBatch( 127 )
{
}

LoginBatch::~LoginBatch()
{
}

void LoginBatch::hello()
{
    Job job( &LoginBatch::helloJob );
    m_queue.addJob( job );
}

void LoginBatch::login( const QString& login, const QString& password )
{
    Job job( &LoginBatch::loginJob );
    job.addArg( login );
    job.addArg( password );
    m_queue.addJob( job );
}

void LoginBatch::loginNew( const QString& login, const QString& password, const QString& newPassword )
{
    Job job( &LoginBatch::loginNewJob );
    job.addArg( login );
    job.addArg( password );
    job.addArg( newPassword );
    m_queue.addJob( job );
}

void LoginBatch::setExpectedUuid( const QString& uuid )
{
    m_uuid = uuid;
}

Command* LoginBatch::fetchNext()
{
    if ( m_queue.moreJobs() )
        return m_queue.callJob( this );

    return NULL;
}

Command* LoginBatch::helloJob( const Job& /*job*/ )
{
    return dataManager->hello();
}

Command* LoginBatch::loginJob( const Job& job )
{
    if ( dataManager->isValid() && ( m_uuid.isEmpty() || m_uuid == dataManager->serverUuid() ) )
        return dataManager->login( job.argString( 0 ), job.argString( 1 ) );
    return NULL;
}

Command* LoginBatch::loginNewJob( const Job& job )
{
    return dataManager->loginNew( job.argString( 0 ), job.argString( 1 ), job.argString( 2 ) );
}
