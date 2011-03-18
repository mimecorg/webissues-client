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

#ifndef STATEDIALOGS_H
#define STATEDIALOGS_H

#include "commanddialog.h"

/**
* Dialog for executing the <tt>SET ISSUE READ</tt> command.
*/
class IssueStateDialog : public CommandDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param issueId Identifier of the issue.
    * @param isRead @c true to set the issue to read, @c false to unread.
    * @param parent The parent widget.
    */
    IssueStateDialog( int issueId, bool isRead, QWidget* parent );

    /**
    * Destructor.
    */
    ~IssueStateDialog();

public: // overrides
    void accept();

private:
    int m_issueId;
    bool m_isRead;
};

/**
* Dialog for executing the <tt>SET FOLDER READ</tt> command.
*/
class FolderStateDialog : public CommandDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param folderId Identifier of the folder.
    * @param isRead @c true to set all issues to read, @c false to unread.
    * @param parent The parent widget.
    */
    FolderStateDialog( int folderId, bool isRead, QWidget* parent );

    /**
    * Destructor.
    */
    ~FolderStateDialog();

public: // overrides
    void accept();

private:
    int m_folderId;
    bool m_isRead;
};

#endif
