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

#ifndef ALERTSBATCH_H
#define ALERTSBATCH_H

#include "abstractbatch.h"
#include "batchjob.h"

#include "data/datarows.h"

class Reply;

/**
* Batch for operations on alerts.
*
* This batch executes a series of commands creating, modifying and deleting
* alerts. After making the modifications, the batch updates projects, folders
* and alerts.
*/
class AlertsBatch : public AbstractBatch
{
    Q_OBJECT
public:
    /**
    * Constructor.
    */
    AlertsBatch();

    /**
    * Desctructor.
    */
    ~AlertsBatch();

public:
    /**
    * Add the <tt>ADD ALERT</tt> command to the batch.
    * @param typeId Identifier of the folder containing the alert.
    * @param viewId Optional identifier of the view associated with the alert.
    * @param alertEmail Email type for the alert.
    */
    void addAlert( int folderId, int viewId, AlertEmail alertEmail );

    /**
    * Add the <tt>MODIFY ALERT</tt> command to the batch. 
    * @param alertId Identifier of the alert to modify.
    * @param alertEmail Email type for the alert.
    */
    void modifyAlert( int alertId, AlertEmail alertEmail );

    /**
    * Add the <tt>DELETE ALERT</tt> command to the batch.
    * @param alertId Identifier of the alert to delete.
    */
    void deleteAlert( int alertId );

public: // overrides
    Command* fetchNext();

private:
    typedef BatchJob<AlertsBatch> Job;
    typedef BatchJobQueue<AlertsBatch> JobQueue;

private:
    Command* addAlertJob( const Job& job );
    Command* modifyAlertJob( const Job& job );
    Command* deleteAlertJob( const Job& job );

private slots:
    void setUpdate();

private:
    JobQueue m_queue;

    bool m_update;
};

#endif
