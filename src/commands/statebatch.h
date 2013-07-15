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

#ifndef STATEBATCH_H
#define STATEBATCH_H

#include "abstractbatch.h"
#include "batchjob.h"

class Reply;

/**
* Batch for changing the state of issues.
*
* This batch executes a series of commands changing the state of issues.
* After making the modifications, the batch updates the issue states.
*/
class StateBatch : public AbstractBatch
{
    Q_OBJECT
public:
    /**
    * Constructor.
    */
    StateBatch();

    /**
    * Desctructor.
    */
    ~StateBatch();

public:
    /**
    * Add the <tt>SET ISSUE READ</tt> command to the batch.
    * @param issueId Identifier of the issue.
    * @param readId The new read stamp of the issue.
    */
    void setIssueRead( int issueId, int readId );

    /**
    * Add the <tt>SET FOLDER READ</tt> command to the batch.
    * @param folderId Identifier of the folder.
    * @param readId The new read stamp of the folder.
    */
    void setFolderRead( int folderId, int readId );

    /**
    * Add the <tt>ADD SUBSCRIPTION</tt> command to the batch.
    * @param issueId Identifier of the issue.
    */
    void addSubscription( int issueId );

    /**
    * Add the <tt>DELETE SUBSCRIPTION</tt> command to the batch.
    * @param issueId Identifier of the issue.
    * @param subscriptionId Identifier of the subscription.
    */
    void deleteSubscription( int issueId, int subscriptionId );

public: // overrides
    Command* fetchNext();

private:
    typedef BatchJob<StateBatch> Job;
    typedef BatchJobQueue<StateBatch> JobQueue;

private:
    Command* setIssueReadJob( const Job& job );
    Command* setFolderReadJob( const Job& job );
    Command* addSubscriptionJob( const Job& job );
    Command* deleteSubscriptionJob( const Job& job );

private slots:
    void setUpdate();

private:
    JobQueue m_queue;

    bool m_update;
};

#endif
