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

#include "viewsbatch.h"
#include "command.h"

#include "data/datamanager.h"

ViewsBatch::ViewsBatch() : AbstractBatch( 0 ),
    m_update( false ),
    m_viewId( 0 )
{
}

ViewsBatch::~ViewsBatch()
{
}

void ViewsBatch::addView( int typeId, const QString& name, const QString& definition, bool isPublic )
{
    Job job( &ViewsBatch::addViewJob );
    job.addArg( typeId );
    job.addArg( name );
    job.addArg( definition );
    job.addArg( isPublic ? 1 : 0 );
    m_queue.addJob( job );
}

void ViewsBatch::renameView( int viewId, const QString& newName )
{
    Job job( &ViewsBatch::renameViewJob );
    job.addArg( viewId );
    job.addArg( newName );
    m_queue.addJob( job );
}

void ViewsBatch::modifyView( int viewId, const QString& definition )
{
    Job job( &ViewsBatch::modifyViewJob );
    job.addArg( viewId );
    job.addArg( definition );
    m_queue.addJob( job );
}

void ViewsBatch::publishView( int viewId, bool isPublic )
{
    Job job( &ViewsBatch::publishViewJob );
    job.addArg( viewId );
    job.addArg( isPublic ? 1 : 0 );
    m_queue.addJob( job );
}

void ViewsBatch::deleteView( int viewId )
{
    Job job( &ViewsBatch::deleteViewJob );
    job.addArg( viewId );
    m_queue.addJob( job );
}

void ViewsBatch::setViewSetting( int typeId, const QString& key, const QString& value )
{
    Job job( &ViewsBatch::setViewSettingJob );
    job.addArg( typeId );
    job.addArg( key );
    job.addArg( value );
    m_queue.addJob( job );
}

Command* ViewsBatch::fetchNext()
{
    if ( m_queue.moreJobs() )
        return m_queue.callJob( this );

    if ( m_update ) {
        m_update = false;
        return dataManager->updateTypes();
    }

    return NULL;
}

Command* ViewsBatch::addViewJob( const Job& job )
{
    Command* command = new Command();

    command->setKeyword( "ADD VIEW" );
    command->setArgs( job.args() );

    command->addRule( "ID i", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( addViewReply( const Reply& ) ) );

    return command;
}

Command* ViewsBatch::renameViewJob( const Job& job )
{
    Command* command = new Command();

    command->setKeyword( "RENAME VIEW" );
    command->setArgs( job.args() );

    command->setAcceptNullReply( true );
    command->addRule( "OK", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( setUpdate() ) );

    return command;
}

Command* ViewsBatch::modifyViewJob( const Job& job )
{
    Command* command = new Command();

    command->setKeyword( "MODIFY VIEW" );
    command->setArgs( job.args() );

    command->setAcceptNullReply( true );
    command->addRule( "OK", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( setUpdate() ) );

    return command;
}

Command* ViewsBatch::publishViewJob( const Job& job )
{
    Command* command = new Command();

    command->setKeyword( "PUBLISH VIEW" );
    command->setArgs( job.args() );

    command->setAcceptNullReply( true );
    command->addRule( "OK", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( setUpdate() ) );

    return command;
}

Command* ViewsBatch::deleteViewJob( const Job& job )
{
    Command* command = new Command();

    command->setKeyword( "DELETE VIEW" );
    command->setArgs( job.args() );

    command->addRule( "OK", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( setUpdate() ) );

    return command;
}

Command* ViewsBatch::setViewSettingJob( const Job& job )
{
    Command* command = new Command();

    command->setKeyword( "SET VIEW SETTING" );
    command->setArgs( job.args() );

    command->setAcceptNullReply( true );
    command->addRule( "OK", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( setUpdate() ) );

    return command;
}

void ViewsBatch::addViewReply( const Reply& reply )
{
    ReplyLine line = reply.lines().at( 0 );
    m_viewId = line.argInt( 0 );

    m_update = true;
}

void ViewsBatch::setUpdate()
{
    m_update = true;
}
