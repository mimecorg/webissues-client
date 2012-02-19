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

#ifndef ISSUEBATCH_H
#define ISSUEBATCH_H

#include "abstractbatch.h"
#include "batchjob.h"

class Reply;

class QFile;

/**
* Batch for executing commands creating and modifying an issue.
*
* This batch can execute a series of commands operating on an existing issue
* or a newly created issue. If any modifications are made, the batch updates
* the issue data.
*/
class IssueBatch : public AbstractBatch
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * Construct a batch operating on an existing issue.
    * @param issueId Identifier of the issue to modify.
    */
    IssueBatch( int issueId );

    /**
    * Constructor.
    * Construct a batch creating a new issue and operating on it.
    * @param folderId The identifier of the folder containing the issue.
    * @param name Name of the issue to create.
    */
    IssueBatch( int folderId, const QString& name );

    /**
    * Destructor.
    */
    ~IssueBatch();

public:
    /**
    * Add the <tt>RENAME ISSUE</tt> command to the batch.
    * @param newName The new name of the issue.
    */
    void renameIssue( const QString& newName );

    /**
    * Add the <tt>SET VALUE</tt> command to the batch.
    * @param attributeId The identifier of the attribute to change.
    * @param newValue The new value of the attribute.
    */
    void setValue( int attributeId, const QString& newValue );

    /**
    * Add the <tt>MOVE ISSUE</tt> command to the batch.
    * @param folderId The target folder to move the issue to.
    */
    void moveIssue( int folderId );

    /**
    * Add the <tt>DELETE ISSUE</tt> command to the batch.
    */
    void deleteIssue();

    /**
    * Add the <tt>ADD COMMENT</tt> command to the batch.
    * @param text Text of the comment.
    */
    void addComment( const QString& text );

    /**
    * Add the <tt>EDIT COMMENT</tt> command to the batch.
    * @param commentId Identifier of the comment.
    * @param newText New text of the comment.
    */
    void editComment( int commentId, const QString& newText );

    /**
    * Add the <tt>DELETE COMMENT</tt> command to the batch.
    * @param commentId Identifier of the comment.
    */
    void deleteComment( int commentId );

    /**
    * Add the <tt>ADD ATTACHMENT</tt> command to the batch.
    * @param name Name of the file.
    * @param description Optional description of the file.
    * @param path Path of the uploaded file.
    */
    void addAttachment( const QString& name, const QString& description, const QString& path );

    /**
    * Add the <tt>GET ATTACHMENT</tt> command to the batch.
    * @param fileId Identifier of the file to download.
    * @param path Path of the downloaded file.
    */
    void getAttachment( int fileId, const QString& path );

    /**
    * Add the <tt>EDIT ATTACHMENT</tt> command to the batch.
    * @param fileId Identifier of the file.
    * @param newName New name of the file.
    * @param newDescription Optional description of the file.
    */
    void editAttachment( int fileId, const QString& newName, const QString& newDescription );

    /**
    * Add the <tt>DELETE ATTACHMENT</tt> command to the batch.
    * @param fileId Identifier of the file.
    */
    void deleteAttachment( int fileId );

    /**
    * If @c true, update folder instead of issue details after making the changes.
    */
    void setUpdateFolder( bool update );

    /**
    * Return the identifier of the issue.
    */
    int issueId() const { return m_issueId; }

    /**
    * Return the status of reading or writing the attachment file.
    * If the file to upload cannot be opened for reading, the <tt>ADD ATTACHMENT</tt> command is not executed
    * and the batch is finished with success. If an error occurs while writing the downloaded file,
    * the batch is finished with success after executing the <tt>GET ATTACHMENT</tt>command and the incomplete
    * file is removed.
    * Use this flag to check if the upload or download operation was really successful.
    * @see QFile::error()
    */
    int fileError() const { return m_fileError; }

signals:
    /**
    * Emitted while uploading an attachment.
    * @param done Number of uploaded bytes.
    */
    void uploadProgress( int done );

    /**
    * Emitted while downloading an attachment.
    * @param done Number of downloaded bytes.
    */
    void downloadProgress( int done );

public: // overrides
    Command* fetchNext();

private:
    typedef BatchJob<IssueBatch> Job;
    typedef BatchJobQueue<IssueBatch> JobQueue;

private:
    Command* addIssueJob( const Job& job );
    Command* renameIssueJob( const Job& job );
    Command* setValueJob( const Job& job );
    Command* moveIssueJob( const Job& job );
    Command* deleteIssueJob( const Job& job );

    Command* addCommentJob( const Job& job );
    Command* editCommentJob( const Job& job );
    Command* deleteCommentJob( const Job& job );

    Command* addAttachmentJob( const Job& job );
    Command* getAttachmentJob( const Job& job );
    Command* editAttachmentJob( const Job& job );
    Command* deleteAttachmentJob( const Job& job );

private slots:
    void addIssueReply( const Reply& reply );

    void uploadProgress( qint64 done, qint64 total );
    void downloadProgress( qint64 done, qint64 total );

    void setUpdate();

private:
    JobQueue m_queue;

    int m_issueId;
    int m_folderId;

    bool m_update;
    bool m_updateFolder;

    QFile* m_file;
    int m_fileError;
};

#endif
