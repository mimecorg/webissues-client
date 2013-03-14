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

#include "issuebatch.h"

#include "commands/command.h"
#include "data/datamanager.h"
#include "data/entities.h"

#include <QFile>

IssueBatch::IssueBatch( int issueId ) : AbstractBatch( 0 ),
    m_issueId( issueId ),
    m_folderId( 0 ),
    m_update( false ),
    m_updateFolder( false ),
    m_file( NULL ),
    m_fileError( QFile::NoError )
{
}

IssueBatch::IssueBatch( int folderId, const QString& name ) : AbstractBatch( 0 ),
    m_issueId( 0 ),
    m_folderId( folderId ),
    m_update( false ),
    m_updateFolder( false ),
    m_file( NULL ),
    m_fileError( QFile::NoError )
{
    Job job( &IssueBatch::addIssueJob );
    job.addArg( folderId );
    job.addArg( name );
    m_queue.addJob( job );
}

IssueBatch::~IssueBatch()
{
    if ( m_file && m_file->openMode() == QIODevice::WriteOnly )
        m_file->remove();
    delete m_file;
}

void IssueBatch::renameIssue( const QString& newName )
{
    Job job( &IssueBatch::renameIssueJob );
    job.addArg( newName );
    m_queue.addJob( job );
}

void IssueBatch::setValue( int attributeId, const QString& newValue )
{
    Job job( &IssueBatch::setValueJob );
    job.addArg( attributeId );
    job.addArg( newValue );
    m_queue.addJob( job );
}

void IssueBatch::moveIssue( int folderId )
{
    Job job( &IssueBatch::moveIssueJob );
    job.addArg( folderId );
    m_queue.addJob( job );
}

void IssueBatch::deleteIssue()
{
    Job job( &IssueBatch::deleteIssueJob );
    m_queue.addJob( job );

    setUpdateFolder( true );
}

void IssueBatch::addComment( const QString& text, TextFormat format )
{
    Job job( &IssueBatch::addCommentJob );
    job.addArg( text );
    job.addArg( format );
    m_queue.addJob( job );
}

void IssueBatch::editComment( int commentId, const QString& newText, TextFormat newFormat )
{
    Job job( &IssueBatch::editCommentJob );
    job.addArg( commentId );
    job.addArg( newText );
    job.addArg( newFormat );
    m_queue.addJob( job );
}

void IssueBatch::deleteComment( int commentId )
{
    Job job( &IssueBatch::deleteCommentJob );
    job.addArg( commentId );
    m_queue.addJob( job );
}

void IssueBatch::addAttachment( const QString& name, const QString& description, const QString& path )
{
    Job job( &IssueBatch::addAttachmentJob );
    job.addArg( name );
    job.addArg( description );
    job.addArg( path );
    m_queue.addJob( job );
}

void IssueBatch::getAttachment( int fileId, const QString& path )
{
    Job job( &IssueBatch::getAttachmentJob );
    job.addArg( fileId );
    job.addArg( path );
    m_queue.addJob( job );
}

void IssueBatch::editAttachment( int fileId, const QString& newName, const QString& newDescription )
{
    Job job( &IssueBatch::editAttachmentJob );
    job.addArg( fileId );
    job.addArg( newName );
    job.addArg( newDescription );
    m_queue.addJob( job );
}

void IssueBatch::deleteAttachment( int fileId )
{
    Job job( &IssueBatch::deleteAttachmentJob );
    job.addArg( fileId );
    m_queue.addJob( job );
}

void IssueBatch::setUpdateFolder( bool update )
{
    if ( update ) {
        IssueEntity issue = IssueEntity::find( m_issueId );
        m_folderId = issue.folderId();
    } else {
        m_folderId = 0;
    }
}

Command* IssueBatch::fetchNext()
{
    if ( m_file != NULL ) {
        m_fileError = m_file->error();
        if ( m_fileError != QFile::NoError )
            return NULL;
    }

    delete m_file;
    m_file = NULL;

    if ( m_queue.moreJobs() )
        return m_queue.callJob( this );

    if ( m_updateFolder ) {
        m_updateFolder = false;
        return dataManager->updateFolder( m_folderId );
    }

    if ( m_update ) {
        m_update = false;
        return dataManager->updateIssue( m_issueId, true );
    }

    return NULL;
}

Command* IssueBatch::addIssueJob( const Job& job )
{
    Command* command = new Command();

    command->setKeyword( "ADD ISSUE" );
    command->addArg( job.argInt( 0 ) );
    command->addArg( job.argString( 1 ) );

    command->addRule( "ID i", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( addIssueReply( const Reply& ) ) );

    return command;
}

Command* IssueBatch::renameIssueJob( const Job& job )
{
    Command* command = new Command();

    command->setKeyword( "RENAME ISSUE" );
    command->addArg( m_issueId );
    command->addArg( job.argString( 0 ) );

    command->setAcceptNullReply( true );
    command->addRule( "ID i", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( setUpdate() ) );

    return command;
}

Command* IssueBatch::setValueJob( const Job& job )
{
    Command* command = new Command();

    command->setKeyword( "SET VALUE" );
    command->addArg( m_issueId );
    command->addArg( job.argInt( 0 ) );
    command->addArg( job.argString( 1 ) );

    command->setAcceptNullReply( true );
    command->addRule( "ID i", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( setUpdate() ) );

    return command;
}

Command* IssueBatch::moveIssueJob( const Job& job )
{
    Command* command = new Command();

    command->setKeyword( "MOVE ISSUE" );
    command->addArg( m_issueId );
    command->addArg( job.argInt( 0 ) );

    command->setAcceptNullReply( true );
    command->addRule( "ID i", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( setUpdate() ) );

    return command;
}

Command* IssueBatch::deleteIssueJob( const Job& /*job*/ )
{
    Command* command = new Command();

    command->setKeyword( "DELETE ISSUE" );
    command->addArg( m_issueId );

    command->addRule( "ID i", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( setUpdate() ) );

    return command;
}

Command* IssueBatch::addCommentJob( const Job& job )
{
    Command* command = new Command();

    command->setKeyword( "ADD COMMENT" );
    command->addArg( m_issueId );
    command->addArg( job.argString( 0 ) );
    command->addArg( job.argInt( 1 ) );

    command->addRule( "ID i", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( setUpdate() ) );

    return command;
}

Command* IssueBatch::editCommentJob( const Job& job )
{
    Command* command = new Command();

    command->setKeyword( "EDIT COMMENT" );
    command->setArgs( job.args() );

    command->setAcceptNullReply( true );
    command->addRule( "ID i", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( setUpdate() ) );

    return command;
}

Command* IssueBatch::deleteCommentJob( const Job& job )
{
    Command* command = new Command();

    command->setKeyword( "DELETE COMMENT" );
    command->setArgs( job.args() );

    command->addRule( "ID i", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( setUpdate() ) );

    return command;
}

Command* IssueBatch::addAttachmentJob( const Job& job )
{
    QString path = job.argString( 2 );

    m_file = new QFile( path );

    if ( !m_file->open( QIODevice::ReadOnly ) ) {
        m_fileError = m_file->error();
        return NULL;
    }

    Command* command = new Command();

    command->setKeyword( "ADD ATTACHMENT" );
    command->addArg( m_issueId );
    command->addArg( job.argString( 0 ) );
    command->addArg( job.argString( 1 ) );

    command->setAttachmentInput( m_file );

    command->addRule( "ID i", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( setUpdate() ) );
    connect( command, SIGNAL( uploadProgress( qint64, qint64 ) ), this, SLOT( uploadProgress( qint64, qint64 ) ) );

    return command;
}

Command* IssueBatch::getAttachmentJob( const Job& job )
{
    QString path = job.argString( 1 );

    m_file = new QFile( path );

    if ( !m_file->open( QIODevice::WriteOnly ) ) {
        m_fileError = m_file->error();
        return NULL;
    }

    Command* command = new Command();

    command->setKeyword( "GET ATTACHMENT" );
    command->addArg( job.argInt( 0 ) );

    command->setBinaryResponseOutput( m_file );

    connect( command, SIGNAL( downloadProgress( qint64, qint64 ) ), this, SLOT( downloadProgress( qint64, qint64 ) ) );

    return command;
}

Command* IssueBatch::editAttachmentJob( const Job& job )
{
    Command* command = new Command();

    command->setKeyword( "EDIT ATTACHMENT" );
    command->setArgs( job.args() );

    command->setAcceptNullReply( true );
    command->addRule( "ID i", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( setUpdate() ) );

    return command;
}

Command* IssueBatch::deleteAttachmentJob( const Job& job )
{
    Command* command = new Command();

    command->setKeyword( "DELETE ATTACHMENT" );
    command->setArgs( job.args() );

    command->addRule( "ID i", ReplyRule::One );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( setUpdate() ) );

    return command;
}

void IssueBatch::addIssueReply( const Reply& reply )
{
    ReplyLine line = reply.lines().at( 0 );
    m_issueId = line.argInt( 0 );

    m_updateFolder = true;
}

void IssueBatch::uploadProgress( qint64 /*done*/, qint64 /*total*/ )
{
    emit uploadProgress( (int)m_file->pos() );
}

void IssueBatch::downloadProgress( qint64 done, qint64 /*total*/ )
{
    emit downloadProgress( (int)done );
}

void IssueBatch::setUpdate()
{
    if ( m_folderId != 0 )
        m_updateFolder = true;
    else
        m_update = true;
}
