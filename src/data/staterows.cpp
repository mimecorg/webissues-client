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

#include "staterows.h"

FolderState::FolderState( int folderId ) :
    m_folderId( folderId ),
    m_listStamp( 0 ),
    m_cached( false )
{
}

FolderState::~FolderState()
{
}

IssueState::IssueState( int issueId ) :
    m_issueId( issueId ),
    m_detailsStamp( 0 ),
    m_readStamp( 0 ),
    m_lockCount( 0 )
{
}

IssueState::~IssueState()
{
}

AlertState::AlertState( int alertId ) :
    m_alertId( alertId ),
    m_unreadCount( 0 ),
    m_modifiedCount( 0 ),
    m_totalCount( 0 )
{
}

AlertState::~AlertState()
{
}
