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

#ifndef LOGINBATCH_H
#define LOGINBATCH_H

#include "abstractbatch.h"
#include "batchjob.h"

class Reply;

/**
* Batch for logging in to the server.
*
* This batch sends a <tt>HELLO</tt> command for identifying the server
* and a <tt>LOGIN</tt> command for logging in as the given user.
*/
class LoginBatch : public AbstractBatch
{
    Q_OBJECT
public:
    /**
    * Constructor.
    */
    LoginBatch();

    /**
    * Destructor.
    */
    ~LoginBatch();

public:
    /**
    * Add HELLO command to the batch.
    */
    void hello();

    /**
    * Add LOGIN command to the batch.
    * @param login The user's login.
    * @param password The user's password.
    */
    void login( const QString& login, const QString& password );

    /**
    * Add LOGIN NEW command to the batch.
    * @param login The user's login.
    * @param password The user's password.
    * @param newPassword The new password to set.
    */
    void loginNew( const QString& login, const QString& password, const QString& newPassword );

    /**
    * Send LOGIN command only if server's unique ID matches the given value.
    */
    void setExpectedUuid( const QString& uuid );

public: // overrides
    Command* fetchNext();

private:
    typedef BatchJob<LoginBatch> Job;
    typedef BatchJobQueue<LoginBatch> JobQueue;

private:
    Command* helloJob( const Job& job );
    Command* loginJob( const Job& job );
    Command* loginNewJob( const Job& job );

private:
    JobQueue m_queue;

    QString m_uuid;
};

#endif
