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

#ifndef PREFERENCESBATCH_H
#define PREFERENCESBATCH_H

#include "abstractbatch.h"
#include "batchjob.h"

class Reply;

/**
* Batch for loading preferences of a user.
*
* This batch sends a <tt>LIST PREFERENCES</tt> command for the given user.
*/
class LoadPreferencesBatch : public AbstractBatch
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param userId Identifier of the user.
    */
    LoadPreferencesBatch( int userId );

    /**
    * Destructor.
    */
    ~LoadPreferencesBatch();

public:
    /**
    * Return the preferences data.
    */
    const QMap<QString, QString>& preferences() const { return m_preferences; }

public: // overrides
    Command* fetchNext();

private:
    typedef BatchJob<LoadPreferencesBatch> Job;
    typedef BatchJobQueue<LoadPreferencesBatch> JobQueue;

private:
    Command* listPreferencesJob( const Job& job );

private slots:
    void listPreferencesReply( const Reply& reply );

private:
    JobQueue m_queue;

    QMap<QString, QString> m_preferences;
};

/**
* Batch for setting preferences of a user.
*
* This batch sends the <tt>SET PREFERENCE</tt> commands with new preferences
* for the given user. If current user's preferences are changed, they are
* automatically updated.
*/
class SavePreferencesBatch : public AbstractBatch
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param userId Identifier of the user.
    * @param data The preferences to set.
    */
    SavePreferencesBatch( int userId, const QMap<QString, QString>& preferences );

    /**
    * Destructor.
    */
    ~SavePreferencesBatch();

public: // overrides
    Command* fetchNext();

private:
    Command* createSetCommand();

private slots:
    void setUpdate();

private:
    int m_userId;
    QMap<QString, QString> m_preferences;

    QMapIterator<QString, QString> m_iterator;

    bool m_update;
};

#endif
