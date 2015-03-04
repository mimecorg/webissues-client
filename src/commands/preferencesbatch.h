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

#ifndef PREFERENCESBATCH_H
#define PREFERENCESBATCH_H

#include "commands/abstractbatch.h"
#include "commands/batchjob.h"

class Reply;

/**
* Batch for setting preferences of a user.
*
* This batch sends the <tt>SET PREFERENCE</tt> commands with new preferences
* for the given user. If current user's preferences are changed, they are
* automatically updated.
*/
class SetPreferencesBatch : public AbstractBatch
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param userId Identifier of the user.
    */
    SetPreferencesBatch( int userId );

    /**
    * Destructor.
    */
    ~SetPreferencesBatch();

public:
    /**
    * Set preference of the user.
    * @param key Key of the preference.
    * @param value Value of the preference.
    */
    void setPreference( const QString& key, const QString& value );

public: // overrides
    Command* fetchNext();

private:
    typedef BatchJob<SetPreferencesBatch> Job;
    typedef BatchJobQueue<SetPreferencesBatch> JobQueue;

private:
    Command* setPreferenceJob( const Job& job );

private slots:
    void setUpdate();

private:
    int m_userId;

    JobQueue m_queue;

    bool m_update;
};

#endif
