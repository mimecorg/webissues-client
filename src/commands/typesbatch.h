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

#ifndef TYPESBATCH_H
#define TYPESBATCH_H

#include "abstractbatch.h"
#include "batchjob.h"

/**
* Batch for operations on types and attributes.
*
* This batch executes a series of commands creating, modifying and deleting
* types and attributes. After making the modifications, the batch updates
* the types and attributes data.
*/
class TypesBatch : public AbstractBatch
{
    Q_OBJECT
public:
    /**
    * Constructor.
    */
    TypesBatch();

    /**
    * Desctructor.
    */
    ~TypesBatch();

public:
    /**
    * Add the <tt>ADD TYPE</tt> command to the batch.
    * @param name Name of the type to create.
    */
    void addType( const QString& name );

    /**
    * Add the <tt>RENAME TYPE</tt> command to the batch. 
    * @param typeId Identifier of the type to rename.
    * @param newName The new name of the type.
    */
    void renameType( int typeId, const QString& newName );

    /**
    * Add the <tt>DELETE TYPE</tt> command to the batch.
    * @param typeId Identifier of the type to delete.
    * @param force Force deleting used type.
    */
    void deleteType( int typeId, bool force );

    /**
    * Add the <tt>ADD ATTRIBUTE</tt> command to the batch.
    * @param typeId Identifier of the issue type containing the attribute.
    * @param name Name of the attribute to create.
    * @param definition Definition of the attribute to create.
    */
    void addAttribute( int typeId, const QString& name, const QString& definition );

    /**
    * Add the <tt>RENAME ATTRIBUTE</tt> command to the batch. 
    * @param attributeId Identifier of the attribute to rename.
    * @param newName The new name of the attribute.
    */
    void renameAttribute( int attributeId, const QString& newName );

    /**
    * Add the <tt>MODIFY ATTRIBUTE</tt> command to the batch. 
    * @param attributeId Identifier of the attribute to modify.
    * @param definition The new definition of the attribute.
    */
    void modifyAttribute( int attributeId, const QString& definition );

    /**
    * Add the <tt>DELETE ATTRIBUTE</tt> command to the batch.
    * @param attributeId Identifier of the attribute to delete.
    * @param force Force deleting used attribute.
    */
    void deleteAttribute( int attributeId, bool force );

public: // overrides
    Command* fetchNext();

private:
    typedef BatchJob<TypesBatch> Job;
    typedef BatchJobQueue<TypesBatch> JobQueue;

private:
    Command* addTypeJob( const Job& job );
    Command* renameTypeJob( const Job& job );
    Command* deleteTypeJob( const Job& job );

    Command* addAttributeJob( const Job& job );
    Command* renameAttributeJob( const Job& job );
    Command* modifyAttributeJob( const Job& job );
    Command* deleteAttributeJob( const Job& job );

private slots:
    void setUpdate();

private:
    JobQueue m_queue;

    bool m_update;
};

#endif
