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

#ifndef FINDITEMBATCH_H
#define FINDITEMBATCH_H

#include "abstractbatch.h"
#include "batchjob.h"

class Reply;

/**
* Batch for executing the <tt>FIND ITEM</tt> command.
*/
class FindItemBatch : public AbstractBatch
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param itemId Identifier of the item to find.
    */
    FindItemBatch( int itemId );

    /**
    * Destructor.
    */
    ~FindItemBatch();

public:
    /**
    * Return the identifier of the issue containing the item or 0 if it wasn't found.
    */
    int issueId() const { return m_issueId; }

public: // overrides
    Command* fetchNext();

private:
    typedef BatchJob<FindItemBatch> Job;
    typedef BatchJobQueue<FindItemBatch> JobQueue;

private:
    Command* findItemJob( const Job& job );

private slots:
    void findItemReply( const Reply& reply );

private:
    JobQueue m_queue;

    int m_issueId;

    bool m_update;
};

#endif
