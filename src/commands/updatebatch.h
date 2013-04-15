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

#ifndef UPDATEBATCH_H
#define UPDATEBATCH_H

#include "commands/abstractbatch.h"
#include "commands/batchjob.h"

/**
* Batch for retrieving data from the server.
*/
class UpdateBatch : public AbstractBatch
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param priority Priority of the batch.
    */
    UpdateBatch( int priority = -5 );

    /**
    * Destructor.
    */
    ~UpdateBatch();

public:
    /**
    * Add a <tt>GET SETTINGS</tt> command to the batch.
    */
    void updateSettings();

    /**
    * Add a <tt>LIST USERS</tt> command to the batch.
    */
    void updateUsers();

    /**
    * Add a <tt>LIST TYPES</tt> command to the batch.
    */
    void updateTypes();

    /**
    * Add a <tt>LIST PROJECTS</tt> command to the batch.
    */
    void updateProjects();

    /**
    * Add a <tt>LIST STATES</tt> command to the batch.
    */
    void updateStates();

    /**
    * Add a <tt>GET SUMMARY</tt> command to the batch.
    */
    void updateSummary( int projectId );

    /**
    * Add a <tt>LIST ISSUES</tt> command to the batch.
    * @param folderId Identifier of the folder to update.
    */
    void updateFolder( int folderId );

    /**
    * Add a <tt>GET DETAILS</tt> command to the batch.
    * @param issueId Identifier of the issue to update.
    * @param markAsRead If @c true, the issue is marked as read.
    */
    void updateIssue( int issueId, bool markAsRead );

    /**
    * If set to @c true, only update folders/issues when they are out of date.
    */
    void setIfNeeded( bool value );

    /**
    * Return @c true if folders/issues are updated only when they are out of date.
    */
    bool ifNeeded() const { return m_ifNeeded; }

public: // overrides
    Command* fetchNext();

private:
    typedef BatchJob<UpdateBatch> Job;
    typedef BatchJobQueue<UpdateBatch> JobQueue;

private:
    Command* updateSettingsJob( const Job& job );
    Command* updateUsersJob( const Job& job );
    Command* updateTypesJob( const Job& job );
    Command* updateProjectsJob( const Job& job );
    Command* updateStatesJob( const Job& job );
    Command* updateSummaryJob( const Job& job );
    Command* updateFolderJob( const Job& job );
    Command* updateIssueJob( const Job& job );

private:
    JobQueue m_queue;

    bool m_ifNeeded;
};

#endif
