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

#include "usersbatch.h"
#include "command.h"

#include "data/datamanager.h"

UsersBatch::UsersBatch() : AbstractBatch( 0 ),
    m_update( false )
{
}

UsersBatch::~UsersBatch()
{
}

void UsersBatch::addUser( const QString& login, const QString& name, const QString& newPassword, bool isTemp )
{
    Job job( &UsersBatch::addUserJob );
    job.addArg( login );
    job.addArg( name );
    job.addArg( newPassword );
    job.addArg( isTemp ? 1 : 0 );
    m_queue.addJob( job );
}

void UsersBatch::setPassword( int userId, const QString& newPassword, bool isTemp )
{
    Job job( &UsersBatch::setPasswordJob );
    job.addArg( userId );
    job.addArg( newPassword );
    job.addArg( isTemp ? 1 : 0 );
    m_queue.addJob( job );
}

void UsersBatch::changePassword( const QString& password, const QString& newPassword )
{
    Job job( &UsersBatch::changePasswordJob );
    job.addArg( password );
    job.addArg( newPassword );
    m_queue.addJob( job );
}

void UsersBatch::grantUser( int userId, Access access )
{
    Job job( &UsersBatch::grantUserJob );
    job.addArg( userId );
    job.addArg( access );
    m_queue.addJob( job );
}

void UsersBatch::renameUser( int userId, const QString& newName )
{
    Job job( &UsersBatch::renameUserJob );
    job.addArg( userId );
    job.addArg( newName );
    m_queue.addJob( job );
}

void UsersBatch::grantMember( int userId, int projectId, Access access )
{
    Job job( &UsersBatch::grantMemberJob );
    job.addArg( userId );
    job.addArg( projectId );
    job.addArg( access );
    m_queue.addJob( job );
}

Command* UsersBatch::fetchNext()
{
    if ( m_queue.moreJobs() )
        return m_queue.callJob( this );

    if ( m_update ) {
        m_update = false;
        return dataManager->updateUsers();
    }

    return NULL;
}

Command* UsersBatch::addUserJob( const Job& job )
{
    Command* command = new Command();

    command->setKeyword( "ADD USER" );
    command->setArgs( job.args() );

    command->addRule( "ID i", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( setUpdate() ) );

    return command;
}

Command* UsersBatch::setPasswordJob( const Job& job )
{
    Command* command = new Command();

    command->setKeyword( "SET PASSWORD" );
    command->setArgs( job.args() );

    command->addRule( "OK", ReplyRule::One );

    return command;
}

Command* UsersBatch::changePasswordJob( const Job& job )
{
    Command* command = new Command();

    command->setKeyword( "CHANGE PASSWORD" );
    command->setArgs( job.args() );

    command->addRule( "OK", ReplyRule::One );

    return command;
}

Command* UsersBatch::grantUserJob( const Job& job )
{
    Command* command = new Command();

    command->setKeyword( "GRANT USER" );
    command->setArgs( job.args() );

    command->setAcceptNullReply( true );
    command->addRule( "OK", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( setUpdate() ) );

    return command;
}

Command* UsersBatch::renameUserJob( const Job& job )
{
    Command* command = new Command();

    command->setKeyword( "RENAME USER" );
    command->setArgs( job.args() );

    command->setAcceptNullReply( true );
    command->addRule( "OK", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( setUpdate() ) );

    return command;
}

Command* UsersBatch::grantMemberJob( const Job& job )
{
    Command* command = new Command();

    command->setKeyword( "GRANT MEMBER" );
    command->setArgs( job.args() );

    command->setAcceptNullReply( true );
    command->addRule( "OK", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( setUpdate() ) );

    return command;
}

void UsersBatch::setUpdate()
{
    m_update = true;
}
