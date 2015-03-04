/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2015 WebIssues Team
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

#include "typesbatch.h"
#include "command.h"

#include "data/datamanager.h"

TypesBatch::TypesBatch() : AbstractBatch( 0 ),
    m_update( false )
{
}

TypesBatch::~TypesBatch()
{
}

void TypesBatch::addType( const QString& name )
{
    Job job( &TypesBatch::addTypeJob );
    job.addArg( name );
    m_queue.addJob( job );
}

void TypesBatch::renameType( int typeId, const QString& newName )
{
    Job job( &TypesBatch::renameTypeJob );
    job.addArg( typeId );
    job.addArg( newName );
    m_queue.addJob( job );
}

void TypesBatch::deleteType( int typeId, bool force )
{
    Job job( &TypesBatch::deleteTypeJob );
    job.addArg( typeId );
    job.addArg( force ? 1 : 0 );
    m_queue.addJob( job );
}

void TypesBatch::addAttribute( int typeId, const QString& name, const QString& definition )
{
    Job job( &TypesBatch::addAttributeJob );
    job.addArg( typeId );
    job.addArg( name );
    job.addArg( definition );
    m_queue.addJob( job );
}

void TypesBatch::renameAttribute( int attributeId, const QString& newName )
{
    Job job( &TypesBatch::renameAttributeJob );
    job.addArg( attributeId );
    job.addArg( newName );
    m_queue.addJob( job );
}

void TypesBatch::modifyAttribute( int attributeId, const QString& definition )
{
    Job job( &TypesBatch::modifyAttributeJob );
    job.addArg( attributeId );
    job.addArg( definition );
    m_queue.addJob( job );
}

void TypesBatch::deleteAttribute( int attributeId, bool force )
{
    Job job( &TypesBatch::deleteAttributeJob );
    job.addArg( attributeId );
    job.addArg( force ? 1 : 0 );
    m_queue.addJob( job );
}

Command* TypesBatch::fetchNext()
{
    if ( m_queue.moreJobs() )
        return m_queue.callJob( this );

    if ( m_update ) {
        m_update = false;
        return dataManager->updateTypes();
    }

    return NULL;
}

Command* TypesBatch::addTypeJob( const Job& job )
{
    Command* command = new Command();

    command->setKeyword( "ADD TYPE" );
    command->setArgs( job.args() );

    command->addRule( "ID i", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( setUpdate() ) );

    return command;
}

Command* TypesBatch::renameTypeJob( const Job& job )
{
    Command* command = new Command();

    command->setKeyword( "RENAME TYPE" );
    command->setArgs( job.args() );

    command->setAcceptNullReply( true );
    command->addRule( "OK", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( setUpdate() ) );

    return command;
}

Command* TypesBatch::deleteTypeJob( const Job& job )
{
    Command* command = new Command();

    command->setKeyword( "DELETE TYPE" );
    command->setArgs( job.args() );

    command->addRule( "OK", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( setUpdate() ) );

    return command;
}

Command* TypesBatch::addAttributeJob( const Job& job )
{
    Command* command = new Command();

    command->setKeyword( "ADD ATTRIBUTE" );
    command->setArgs( job.args() );

    command->addRule( "ID i", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( setUpdate() ) );

    return command;
}

Command* TypesBatch::renameAttributeJob( const Job& job )
{
    Command* command = new Command();

    command->setKeyword( "RENAME ATTRIBUTE" );
    command->setArgs( job.args() );

    command->setAcceptNullReply( true );
    command->addRule( "OK", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( setUpdate() ) );

    return command;
}

Command* TypesBatch::modifyAttributeJob( const Job& job )
{
    Command* command = new Command();

    command->setKeyword( "MODIFY ATTRIBUTE" );
    command->setArgs( job.args() );

    command->setAcceptNullReply( true );
    command->addRule( "OK", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( setUpdate() ) );

    return command;
}

Command* TypesBatch::deleteAttributeJob( const Job& job )
{
    Command* command = new Command();

    command->setKeyword( "DELETE ATTRIBUTE" );
    command->setArgs( job.args() );

    command->addRule( "OK", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( setUpdate() ) );

    return command;
}

void TypesBatch::setUpdate()
{
    m_update = true;
}
