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

#ifndef STATEROWS_H
#define STATEROWS_H

#include <QDateTime>

/**
* A row of a folder state table.
*
* This table is used by the DataManager for tracking the state of folders.
*/
class FolderState
{
public:
    /**
    * Constructor.
    */
    FolderState( int folderId );

    /**
    * Destructor.
    */
    ~FolderState();

public:
    /**
    * Return the identifier of the folder.
    */
    int folderId() const { return m_folderId; }

    /**
    * Set the stamp of the last <tt>LIST ISSUES</tt> command.
    */
    void setListStamp( int stamp ) { m_listStamp = stamp; }

    /**
    * Return the stamp of the last <tt>LIST ISSUES</tt> command.
    * This property is used by the DataManager for the incremental update of folders.
    */
    int listStamp() const { return m_listStamp; }

    /**
    * Set the cached state of the folder.
    */
    void setCached( bool cached ) { m_cached = cached; }

    /**
    * Return the cached state of the folder.
    * This property is used by the DataManager for cashing folders in data files.
    */
    bool cached() const { return m_cached; }

private:
    int m_folderId;
    int m_listStamp;
    bool m_cached;
};

/**
* A row of an issue state table.
*
* This table is used by the DataManager for tracking the state of issues.
*/
class IssueState
{
public:
    /**
    * Constructor.
    */
    IssueState( int issueId );

    /**
    * Destructor.
    */
    ~IssueState();

public:
    /**
    * Return the identifier of the issue.
    */
    int issueId() const { return m_issueId; }

    /**
    * Set the stamp of the last <tt>GET DETAILS</tt> command.
    */
    void setDetailsStamp( int stamp ) { m_detailsStamp = stamp; }

    /**
    * Return the stamp of the last <tt>GET DETAILS</tt> command.
    * This property is used by the DataManager for the incremental update of issues.
    */
    int detailsStamp() const { return m_detailsStamp; }

    /**
    * Set the stamp of the issue when it was last read.
    */
    void setReadStamp( int stamp ) { m_readStamp = stamp; }

    /**
    * Return the stamp of the issue when it was last read.
    */
    int readStamp() const { return m_readStamp; }

    /**
    * Set the lock count of the issue details.
    */
    void setLockCount( int count ) { m_lockCount = count; }

    /**
    * Return the lock count of the issue details.
    * This property is used by the DataManager for locking issue details.
    */
    int lockCount() const { return m_lockCount; }

    /**
    * Set the time when the issue was last used.
    */
    void setTimeUsed( const QDateTime& time ) { m_usedTime = time; }

    /**
    * Return the time when the issue was last used.
    * This property is used by the DataManager for removing least recently used issue details
    * from memory.
    */
    const QDateTime& timeUsed() const { return m_usedTime; }

private:
    int m_issueId;
    int m_detailsStamp;
    int m_readStamp;
    int m_lockCount;
    QDateTime m_usedTime;
};

/**
* A row of an alert state table.
*
* This table is used by the DataManager for tracking statistics of alerts.
*/
class AlertState
{
public:
    /**
    * Constructor.
    */
    AlertState( int alertId );

    /**
    * Destructor.
    */
    ~AlertState();

public:
    /**
    * Return the identifier of the alert.
    */
    int alertId() const { return m_alertId; }

    /**
    * Set the number of unread issues for this alert.
    */
    void setUnreadCount( int count ) { m_unreadCount = count; }

    /**
    * Return the number of unread issues for this alert.
    */
    int unreadCount() const { return m_unreadCount; }

    /**
    * Set the number of modified issues for this alert.
    */
    void setModifiedCount( int count ) { m_modifiedCount = count; }

    /**
    * Return the number of modified issues for this alert.
    */
    int modifiedCount() const { return m_modifiedCount; }

    /**
    * Set the total number of issues for this alert.
    */
    void setTotalCount( int count ) { m_totalCount = count; }

    /**
    * Return the total number of issues for this alert.
    */
    int totalCount() const { return m_totalCount; }

private:
    int m_alertId;
    int m_unreadCount;
    int m_modifiedCount;
    int m_totalCount;
};

#endif
