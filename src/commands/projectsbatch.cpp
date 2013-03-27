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

#include "projectsbatch.h"

#include "commands/command.h"

ProjectsBatch::ProjectsBatch() : AbstractBatch( 0 ),
    m_update( false )
{
}

ProjectsBatch::~ProjectsBatch()
{
}

void ProjectsBatch::addProject( const QString& name )
{
    Job job( &ProjectsBatch::addProjectJob );
    job.addArg( name );
    m_queue.addJob( job );
}

void ProjectsBatch::renameProject( int projectId, const QString& newName )
{
    Job job( &ProjectsBatch::renameProjectJob );
    job.addArg( projectId );
    job.addArg( newName );
    m_queue.addJob( job );
}

void ProjectsBatch::deleteProject( int projectId, bool force )
{
    Job job( &ProjectsBatch::deleteProjectJob );
    job.addArg( projectId );
    job.addArg( force ? 1 : 0 );
    m_queue.addJob( job );
}

void ProjectsBatch::addFolder( int projectId, int typeId, const QString& name )
{
    Job job( &ProjectsBatch::addFolderJob );
    job.addArg( projectId );
    job.addArg( typeId );
    job.addArg( name );
    m_queue.addJob( job );
}

void ProjectsBatch::renameFolder( int folderId, const QString& newName )
{
    Job job( &ProjectsBatch::renameFolderJob );
    job.addArg( folderId );
    job.addArg( newName );
    m_queue.addJob( job );
}

void ProjectsBatch::deleteFolder( int folderId, bool force )
{
    Job job( &ProjectsBatch::deleteFolderJob );
    job.addArg( folderId );
    job.addArg( force ? 1 : 0 );
    m_queue.addJob( job );
}

void ProjectsBatch::moveFolder( int folderId, int projectId )
{
    Job job( &ProjectsBatch::moveFolderJob );
    job.addArg( folderId );
    job.addArg( projectId );
    m_queue.addJob( job );
}

void ProjectsBatch::addProjectDescription( int projectId, const QString& text, TextFormat format )
{
    Job job( &ProjectsBatch::addProjectDescriptionJob );
    job.addArg( projectId );
    job.addArg( text );
    job.addArg( format );
    m_queue.addJob( job );
}

void ProjectsBatch::editProjectDescription( int projectId, const QString& newText, TextFormat newFormat )
{
    Job job( &ProjectsBatch::editProjectDescriptionJob );
    job.addArg( projectId );
    job.addArg( newText );
    job.addArg( newFormat );
    m_queue.addJob( job );
}

void ProjectsBatch::deleteProjectDescription( int projectId )
{
    Job job( &ProjectsBatch::deleteProjectDescriptionJob );
    job.addArg( projectId );
    m_queue.addJob( job );
}

Command* ProjectsBatch::fetchNext()
{
    if ( m_queue.moreJobs() )
        return m_queue.callJob( this );

    if ( m_update ) {
        m_update = false;
        return dataManager->updateProjects();
    }

    return NULL;
}

Command* ProjectsBatch::addProjectJob( const Job& job )
{
    Command* command = new Command();

    command->setKeyword( "ADD PROJECT" );
    command->setArgs( job.args() );

    command->addRule( "ID i", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( setUpdate() ) );

    return command;
}

Command* ProjectsBatch::renameProjectJob( const Job& job )
{
    Command* command = new Command();

    command->setKeyword( "RENAME PROJECT" );
    command->setArgs( job.args() );

    command->setAcceptNullReply( true );
    command->addRule( "OK", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( setUpdate() ) );

    return command;
}

Command* ProjectsBatch::deleteProjectJob( const Job& job )
{
    Command* command = new Command();

    command->setKeyword( "DELETE PROJECT" );
    command->setArgs( job.args() );

    command->addRule( "OK", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( setUpdate() ) );

    return command;
}

Command* ProjectsBatch::addFolderJob( const Job& job )
{
    Command* command = new Command();

    command->setKeyword( "ADD FOLDER" );
    command->setArgs( job.args() );

    command->addRule( "ID i", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( setUpdate() ) );

    return command;
}

Command* ProjectsBatch::renameFolderJob( const Job& job )
{
    Command* command = new Command();

    command->setKeyword( "RENAME FOLDER" );
    command->setArgs( job.args() );

    command->setAcceptNullReply( true );
    command->addRule( "OK", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( setUpdate() ) );

    return command;
}

Command* ProjectsBatch::deleteFolderJob( const Job& job )
{
    Command* command = new Command();

    command->setKeyword( "DELETE FOLDER" );
    command->setArgs( job.args() );

    command->addRule( "OK", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( setUpdate() ) );

    return command;
}

Command* ProjectsBatch::moveFolderJob( const Job& job )
{
    Command* command = new Command();

    command->setKeyword( "MOVE FOLDER" );
    command->setArgs( job.args() );

    command->setAcceptNullReply( true );
    command->addRule( "OK", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( setUpdate() ) );

    return command;
}

Command* ProjectsBatch::addProjectDescriptionJob( const Job& job )
{
    Command* command = new Command();

    command->setKeyword( "ADD PROJECT DESCRIPTION" );
    command->setArgs( job.args() );

    command->addRule( "ID i", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( setUpdate() ) );

    return command;
}

Command* ProjectsBatch::editProjectDescriptionJob( const Job& job )
{
    Command* command = new Command();

    command->setKeyword( "EDIT PROJECT DESCRIPTION" );
    command->setArgs( job.args() );

    command->setAcceptNullReply( true );
    command->addRule( "ID i", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( setUpdate() ) );

    return command;
}

Command* ProjectsBatch::deleteProjectDescriptionJob( const Job& job )
{
    Command* command = new Command();

    command->setKeyword( "DELETE PROJECT DESCRIPTION" );
    command->setArgs( job.args() );

    command->addRule( "ID i", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( setUpdate() ) );

    return command;
}

void ProjectsBatch::setUpdate()
{
    m_update = true;
}
