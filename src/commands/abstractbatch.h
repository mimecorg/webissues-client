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

#ifndef ABSTRACTBATCH_H
#define ABSTRACTBATCH_H

#include <QObject>

class Command;

/**
* Abstract batch providing commands to execute.
*
* Inherited classes must implement the fetchNext() method.
*/
class AbstractBatch : public QObject
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param priority Priority of this batch in the CommandManager queue.
    */
    AbstractBatch( int priority );

    /**
    * Destructor.
    */
    ~AbstractBatch();

public:
    /**
    * Create next command to execute.
    * This method is called by the CommandManager when a command can be executed.
    * Only one command is created at a time. The CommandManager becomes the owner
    * of the returned object.
    * This method must be implemented in inherited classes.
    * @return The command to execute or @c NULL if there are no more commands.
    */
    virtual Command* fetchNext() = 0;

public:
    /**
    * Return the priority of this batch.
    */
    int priority() const { return m_priority; }

signals:
    /**
    * Signal emitted when the batch execution is completed.
    * @note The batch is deleted directly after this signal is emitted.
    * @param successful Flag indicating if the batch was executed successfully or an error occured.
    */
    void completed( bool successful );

private:
    int m_priority;
};

#endif
