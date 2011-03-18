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

#include "datarows.h"

ProjectRow::ProjectRow( int projectId, const QString& name ) :
    m_projectId( projectId ),
    m_name( name )
{
}

ProjectRow::~ProjectRow()
{
}

FolderRow::FolderRow( int folderId, int projectId, const QString& name, int typeId, int stamp ) :
    m_folderId( folderId ),
    m_projectId( projectId ),
    m_name( name ),
    m_typeId( typeId ),
    m_stamp( stamp )
{
}

FolderRow::~FolderRow()
{
}

UserRow::UserRow( int userId, const QString& login, const QString& name, Access access ) :
    m_userId( userId ),
    m_login( login ),
    m_name( name ),
    m_access( access )
{
}

UserRow::~UserRow()
{
}

MemberRow::MemberRow( int userId, int projectId, Access access ) :
    m_userId( userId ),
    m_projectId( projectId ),
    m_access( access )
{
}

MemberRow::~MemberRow()
{
}

TypeRow::TypeRow( int typeId, const QString& name ) :
    m_typeId( typeId ),
    m_name( name )
{
}

TypeRow::~TypeRow()
{
}

AttributeRow::AttributeRow( int attributeId, int typeId, const QString& name, const QString& definition ) :
    m_attributeId( attributeId ),
    m_typeId( typeId ),
    m_name( name ),
    m_definition( definition )
{
}

AttributeRow::~AttributeRow()
{
}

IssueRow::IssueRow( int issueId, int folderId, const QString& name, int stamp, const QDateTime& createdDate,
        int createdUser, const QDateTime& modifiedDate, int modifiedUser ) :
    m_issueId( issueId ),
    m_folderId( folderId ),
    m_name( name ),
    m_stamp( stamp ),
    m_createdDate( createdDate ),
    m_createdUser( createdUser ),
    m_modifiedDate( modifiedDate ),
    m_modifiedUser( modifiedUser )
{
}

IssueRow::~IssueRow()
{
}

ValueRow::ValueRow( int attributeId, int issueId, const QString& value ) :
    m_attributeId( attributeId ),
    m_issueId( issueId ),
    m_value( value )
{
}

ValueRow::~ValueRow()
{
}

ChangeRow::ChangeRow( int changeId, int issueId, ChangeType changeType, int stamp, const QDateTime& createdDate, int createdUser,
        const QDateTime& modifiedDate, int modifiedUser, int attributeId, const QString& oldValue, const QString& newValue,
        int fromFolder, int toFolder ) :
    m_changeId( changeId ),
    m_issueId( issueId ),
    m_changeType( changeType ),
    m_stamp( stamp ),
    m_createdDate( createdDate ),
    m_createdUser( createdUser ),
    m_modifiedDate( modifiedDate ),
    m_modifiedUser( modifiedUser ),
    m_attributeId( attributeId ),
    m_oldValue( oldValue ),
    m_newValue( newValue ),
    m_fromFolder( fromFolder ),
    m_toFolder( toFolder )
{
}

ChangeRow::~ChangeRow()
{
}

CommentRow::CommentRow( int commentId, const QString& text ) :
    m_commentId( commentId ),
    m_text( text )
{
}

CommentRow::~CommentRow()
{
}

FileRow::FileRow( int fileId, const QString& name, int size, const QString& description ) :
    m_fileId( fileId ),
    m_name( name ),
    m_size( size ),
    m_description( description )
{
}

FileRow::~FileRow()
{
}

ViewRow::ViewRow( int viewId, int typeId, const QString& name, const QString& definition, bool isPublic ) :
    m_viewId( viewId ),
    m_typeId( typeId ),
    m_name( name ),
    m_definition( definition ),
    m_isPublic( isPublic )
{
}

ViewRow::~ViewRow()
{
}

AlertRow::AlertRow( int alertId, int folderId, int viewId, AlertEmail alertEmail ) :
    m_alertId( alertId ),
    m_folderId( folderId ),
    m_viewId( viewId ),
    m_alertEmail( alertEmail )
{
}

AlertRow::~AlertRow()
{
}
