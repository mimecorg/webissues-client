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

#ifndef BATCHJOB_H
#define BATCHJOB_H

#include <QList>
#include <QVariant>

class Command;

/**
* A single job for a batch of commands.
*
* A job creates a single command for execution. A job is a method which creates the command
* and an array of arguments which can be read from the method.
*
* @see BatchJobQueue
*/
template<class BATCH>
class BatchJob
{
public:
    /**
    * Default constructor.
    */
    BatchJob() :
        m_method( NULL )
    {
    }

    /**
    * Constructor using a method pointer.
    * @param method Pointer to a job method.
    */
    BatchJob( Command* ( BATCH::*method )( const BatchJob<BATCH>& job ) ) :
        m_method( method )
    {
    }

    /**
    * Destructor.
    */
    ~BatchJob()
    {
    }

public:
    /**
    * Add an integer argument to the job.
    */
    void addArg( int number )
    {
        m_args.append( QVariant( number ) );
    }

    /**
    * Add a string argument to the job.
    */
    void addArg( const QString& string )
    {
        m_args.append( QVariant( string ) );
    }

    /**
    * Return all arguments of the job.
    */
    const QList<QVariant>& args() const { return m_args; }

    /**
    * Read an integer argument of the job.
    */
    int argInt( int index ) const
    {
        return m_args[ index ].toInt();
    }

    /**
    * Read a string argument of the job.
    */
    QString argString( int index ) const
    {
        return m_args[ index ].toString();
    }

    /**
    * Call the job method using the given object.
    * @param batch The object whose method will be executed.
    * @return The command created by the job method.
    */
    Command* call( BATCH* batch )
    {
        return ( batch->*m_method )( *this );
    }

private:
    Command* ( BATCH::*m_method )( const BatchJob<BATCH>& job );

    QList<QVariant> m_args;
};

/**
* A queue of jobs for a batch of commands.
*
* This template siplifies implementing a batch of commands by providing a queue
* of jobs. Each job in the queue creates a single command to execute by calling
* an appropriate method. Each job may store additional arguments.
*
* Example implementation of a batch:
*
* @code
* class MyBatch : public AbstractBatch
* {
* public:
*     void addJob1( int arg )
*     {
*         Job job( &MyBatch::job1 );
*         job.addArg( arg );
*         m_queue.addJob( job );
*     }
*
*     Command* fetchNext()
*     {
*         if ( m_queue.moreJobs() )
*             return m_queue.callJob( this );
*         return NULL;
*     }
*
* private:
*     typedef BatchJob<MyBatch> Job;
*     typedef BatchJobQueue<MyBatch> JobQueue;
*
*     Command* job1( const Job& job )
*     {
*         // create the command
*     }
*
*     JobQueue m_queue;
* };
* @endcode
*/
template<class BATCH>
class BatchJobQueue
{
public:
    /**
    * Constructor.
    * Create an empty queue.
    */
    BatchJobQueue() :
        m_next( 0 )
    {
    }

    /**
    * Destructor.
    */
    ~BatchJobQueue()
    {
    }

public:
    /**
    * Type of a single job in the queue.
    */
    typedef BatchJob<BATCH> Job;

public:
    /**
    * Add a job to the queue.
    */
    void addJob( const Job& job )
    {
        m_jobs.append( job );
    }

    /**
    * Return @c true if there are more jobs to execute.
    */
    bool moreJobs() const
    {
        return m_next < m_jobs.count();
    }

    /**
    * Call the current job and advance to the next one.
    * @param batch The object whose method will be executed.
    * @return The command created by the job method.
    */
    Command* callJob( BATCH* batch )
    {
        return m_jobs[ m_next++ ].call( batch );
    }

private:
    QList<Job> m_jobs;
    int m_next;
};

#endif
