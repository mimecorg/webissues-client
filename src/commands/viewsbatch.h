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

#ifndef VIEWSBATCH_H
#define VIEWSBATCH_H

#include "abstractbatch.h"
#include "batchjob.h"

class Reply;

/**
* Batch for operations on views and view settings.
*
* This batch executes a series of commands creating, modifying and deleting
* views and changing view settings. After making the modifications, the batch
* updates the views and view settings.
*/
class ViewsBatch : public AbstractBatch
{
    Q_OBJECT
public:
    /**
    * Constructor.
    */
    ViewsBatch();

    /**
    * Desctructor.
    */
    ~ViewsBatch();

public:
    /**
    * Add the <tt>ADD VIEW</tt> command to the batch.
    * @param typeId Identifier of the issue type containing the view.
    * @param name Name of the view to create.
    * @param definition Definition of the view to create.
    * @param isPublic @c true if the view is public, @c false otherwise.
    */
    void addView( int typeId, const QString& name, const QString& definition, bool isPublic );

    /**
    * Add the <tt>RENAME VIEW</tt> command to the batch. 
    * @param viewId Identifier of the view to rename.
    * @param newName The new name of the view.
    */
    void renameView( int viewId, const QString& newName );

    /**
    * Add the <tt>MODIFY VIEW</tt> command to the batch. 
    * @param viewId Identifier of the view to modify.
    * @param definition The new definition of the view.
    */
    void modifyView( int viewId, const QString& definition );

    /**
    * Add the <tt>PUBLISH VIEW</tt> command to the batch. 
    * @param viewId Identifier of the view to publish or unpublish.
    * @param isPublic @c true if the view is published, @c false otherwise.
    */
    void publishView( int viewId, bool isPublic );

    /**
    * Add the <tt>DELETE VIEW</tt> command to the batch.
    * @param viewId Identifier of the view to delete.
    */
    void deleteView( int viewId );

    /**
    * Add the <tt>SET VIEW SETTING</tt> command to the batch.
    * @param typeId Issue type associated with the setting.
    * @param key Name of the setting.
    * @param value The new value of the setting.
    */
    void setViewSetting( int typeId, const QString& key, const QString& value );

    /**
    * Return the identifier of the created view.
    */
    int viewId() const { return m_viewId; }

public: // overrides
    Command* fetchNext();

private:
    typedef BatchJob<ViewsBatch> Job;
    typedef BatchJobQueue<ViewsBatch> JobQueue;

private:
    Command* addViewJob( const Job& job );
    Command* renameViewJob( const Job& job );
    Command* modifyViewJob( const Job& job );
    Command* publishViewJob( const Job& job );
    Command* deleteViewJob( const Job& job );
    Command* setViewSettingJob( const Job& job );

private slots:
    void addViewReply( const Reply& reply );

    void setUpdate();

private:
    JobQueue m_queue;

    bool m_update;

    int m_viewId;
};

#endif
