/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2017 WebIssues Team
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

#ifndef UPDATEEVENT_H
#define UPDATEEVENT_H

#include <QEvent>

/**
* Event sent to data observers when data is updated.
*
* Use DataManager::addObserver() to receive update events and DataManager::removeObserver()
* to stop receiving them.
*/
class UpdateEvent : public QEvent
{
public:
    /**
    * Logical unit of data.
    */
    enum Unit
    {
        /** Update user's global access. */
        GlobalAccess,
        /** Update server settings. */
        Settings,
        /** Update users and their membership. */
        Users,
        /** Update types, attributes and views. */
        Types,
        /** Update projects, folders and alerts. */
        Projects,
        /** Update project summary. */
        Summary,
        /** Update states of all issues. */
        States,
        /** Update states of alerts. */
        AlertStates,
        /** Update issues and issue details in given folder. */
        Folder,
        /** Update issues in given folder. */
        IssueList,
        /** Update details of given issue. */
        Issue,
    };

    /**
    * Type of the event returned by QCustomEvent::type().
    */
    static const int Type = QEvent::User + 1;

public:
    /**
    * Constructor.
    * @param unit Unit of data which was updated.
    * @param id Identifier of the updated folder or issue.
    */
    UpdateEvent( Unit unit, int id );

    /**
    * Desctructor.
    */
    ~UpdateEvent();

public:
    /**
    * Return the unit of data which was updated.
    */
    Unit unit() const { return m_unit; }

    /**
    * Return the identifier of the updated folder or issue.
    */
    int id() const { return m_id; }

private:
    Unit m_unit;
    int m_id;
};

#endif
