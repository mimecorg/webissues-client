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

#include "statedialogs.h"

#include "commands/statebatch.h"
#include "data/datamanager.h"
#include "data/entities.h"
#include "utils/iconloader.h"

#include <QLayout>
#include <QLabel>

IssueStateDialog::IssueStateDialog( int issueId, bool isRead, QWidget* parent ) : CommandDialog( parent ),
    m_issueId( issueId ),
    m_isRead( isRead )
{
    IssueEntity issue = IssueEntity::find( issueId );

    if ( isRead )
        setWindowTitle( tr( "Mark As Read" ) );
    else
        setWindowTitle( tr( "Mark As Unread" ) );
    setPrompt( tr( "Updating the state of issue <b>%1</b>." ).arg( issue.name() ) );
    setPromptPixmap( IconLoader::pixmap( isRead ? "issue" : "issue-unread", 22 ) );

    setContentLayout( NULL, true );
}

IssueStateDialog::~IssueStateDialog()
{
}

void IssueStateDialog::accept()
{
    StateBatch* batch = new StateBatch();
    batch->setIssueRead( m_issueId, m_isRead );

    executeBatch( batch );
}

FolderStateDialog::FolderStateDialog( int folderId, bool isRead, QWidget* parent ) : CommandDialog( parent ),
    m_folderId( folderId ),
    m_isRead( isRead )
{
    FolderEntity folder = FolderEntity::find( folderId );

    if ( isRead ) {
        setWindowTitle( tr( "Mark All As Read" ) );
        setPrompt( tr( "Are you sure you want to mark all issues in folder <b>%1</b> as read?" ).arg( folder.name() ) );
        setPromptPixmap( IconLoader::pixmap( "folder-read", 22 ) );
    } else {
        setWindowTitle( tr( "Mark All As Unread" ) );
        setPrompt( tr( "Are you sure you want to mark all issues in folder <b>%1</b> as unread?" ).arg( folder.name() ) );
        setPromptPixmap( IconLoader::pixmap( "folder-unread", 22 ) );
    }

    setContentLayout( NULL, true );
}

FolderStateDialog::~FolderStateDialog()
{
}

void FolderStateDialog::accept()
{
    StateBatch* batch = new StateBatch();
    batch->setFolderRead( m_folderId, m_isRead );

    executeBatch( batch );
}
