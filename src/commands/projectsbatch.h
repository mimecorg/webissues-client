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

#ifndef PROJECTSBATCH_H
#define PROJECTSBATCH_H

#include "abstractbatch.h"
#include "batchjob.h"

/**
* Batch for operations on projects and folders.
*
* This batch executes a series of commands creating, modifying and deleting
* projects and folders. After making the modifications, the batch updates
* the projects and folders data.
*/
class ProjectsBatch : public AbstractBatch
{
    Q_OBJECT
public:
    /**
    * Constructor.
    */
    ProjectsBatch();

    /**
    * Destructor.
    */
    ~ProjectsBatch();

public:
    /**
    * Add the <tt>ADD PROJECT</tt> command to the batch.
    * @param name Name of the project to create.
    */
    void addProject( const QString& name );

    /**
    * Add the <tt>RENAME PROJECT</tt> command to the batch. 
    * @param projectId Identifier of the project to rename.
    * @param newName The new name of the project.
    */
    void renameProject( int projectId, const QString& newName );

    /**
    * Add the <tt>DELETE PROJECT</tt> command to the batch. 
    * @param projectId Identifier of the project to delete.
    * @param force Force deleting non empty project.
    */
    void deleteProject( int projectId, bool force );

    /**
    * Add the <tt>ADD FOLDER</tt> command to the batch. 
    * @param projectId Identifier of the project containing the folder.
    * @param typeId Identifier of the type of issues in the folder.
    * @param name Name of the folder to create.
    */
    void addFolder( int projectId, int typeId, const QString& name );

    /**
    * Add the <tt>RENAME FOLDER</tt> command to the batch. 
    * @param folderId Identifier of the folder to rename.
    * @param newName The new name of the folder.
    */
    void renameFolder( int folderId, const QString& newName );

    /**
    * Add the <tt>DELETE FOLDER</tt> command to the batch. 
    * @param folderId Identifier of the folder to delete.
    * @param force Force deleting non empty folder.
    */
    void deleteFolder( int folderId, bool force );

    /**
    * Add the <tt>MOVE FOLDER</tt> command to the batch. 
    * @param folderId Identifier of the folder to delete.
    * @param projectId Identifier of the target project.
    */
    void moveFolder( int folderId, int projectId );

public: // overrides
    Command* fetchNext();

private:
    typedef BatchJob<ProjectsBatch> Job;
    typedef BatchJobQueue<ProjectsBatch> JobQueue;

private:
    Command* addProjectJob( const Job& job );
    Command* renameProjectJob( const Job& job );
    Command* deleteProjectJob( const Job& job );

    Command* addFolderJob( const Job& job );
    Command* renameFolderJob( const Job& job );
    Command* deleteFolderJob( const Job& job );
    Command* moveFolderJob( const Job& job );

private slots:
    void setUpdate();

private:
    JobQueue m_queue;

    bool m_update;
};

#endif
