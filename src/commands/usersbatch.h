/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2014 WebIssues Team
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

#ifndef USERSBATCH_H
#define USERSBATCH_H

#include "commands/abstractbatch.h"
#include "commands/batchjob.h"
#include "data/datamanager.h"

/**
* Batch for operations on users.
*
* This batch executes a series of commands creating and modifying users
* and their rights. After making the modifications, the batch updates
* the users data.
*/
class UsersBatch : public AbstractBatch
{
    Q_OBJECT
public:
    /**
    * Constructor.
    */
    UsersBatch();

    /**
    * Destructor.
    */
    ~UsersBatch();

public:
    /**
    * Add the <tt>ADD USER</tt> command to the batch.
    * @param login Login of the user to create.
    * @param name Name of the user.
    * @param password Password for the user.
    * @param isTemp If @true user must change password at next logon.
    */
    void addUser( const QString& login, const QString& name, const QString& password, bool isTemp );

    /**
    * Add the <tt>SET PASSWORD</tt> command to the batch.
    * @param userId Identifier of the user.
    * @param newPassword The new password for the user.
    * @param isTemp If @true user must change password at next logon.
    */
    void setPassword( int userId, const QString& newPassword, bool isTemp );

    /**
    * Add the <tt>CHANGE PASSWORD</tt> command to the batch.
    * @param password The current password of the user.
    * @param newPassword The new password for the user.
    */
    void changePassword( const QString& password, const QString& newPassword );

    /**
    * Add the <tt>GRANT USER</tt> command to the batch.
    * @param userId Identifier of the user.
    * @param access The new access level of the user.
    */
    void grantUser( int userId, Access access );

    /**
    * Add the <tt>RENAME USER</tt> command to the batch.
    * @param userId Identifier of the user.
    * @param newName The new name of the user.
    */
    void renameUser( int userId, const QString& newName );

    /**
    * Add the <tt>GRANT MEMBER</tt> command to the batch.
    * @param userId Identifier of the user.
    * @param projectId Identifier of the project.
    * @param access The new access level to the project.
    */
    void grantMember( int userId, int projectId, Access access );

public: // overrides
    Command* fetchNext();

private:
    typedef BatchJob<UsersBatch> Job;
    typedef BatchJobQueue<UsersBatch> JobQueue;

private:
    Command* addUserJob( const Job& job );
    Command* setPasswordJob( const Job& job );
    Command* changePasswordJob( const Job& job );
    Command* grantUserJob( const Job& job );
    Command* renameUserJob( const Job& job );
    Command* grantMemberJob( const Job& job );

private slots:
    void setUpdate();

private:
    JobQueue m_queue;

    bool m_update;
};

#endif
