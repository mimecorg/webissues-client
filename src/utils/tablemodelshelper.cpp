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

#include "tablemodelshelper.h"
#include "formatter.h"

#include "data/datamanager.h"
#include "data/datarows.h"
#include "models/tablemodels.h"

#include <QApplication>

QString TableModelsHelper::formatId( int id )
{
    return QString( "#%1" ).arg( id );
}

QString TableModelsHelper::formatSize( int size )
{
    Formatter formatter;

    if ( size >= 1048576 ) {
        double mb = (double)size / 1048576.0;
        return tr( "%1 MB" ).arg( formatter.formatNumber( mb, 1, true ) );
    }
    if ( size >= 1024 ) {
        double kb = (double)size / 1024.0;
        return tr( "%1 kB" ).arg( formatter.formatNumber( kb, 1, true ) );
    }
    return tr( "%1 bytes" ).arg( formatter.formatNumber( size, 0, false ) );
}

QString TableModelsHelper::userName( int userId )
{
    const UserRow* row = dataManager->users()->find( userId );
    if ( !row )
        return QString();
    return row->name();
}

QString TableModelsHelper::typeName( int typeId )
{
    const TypeRow* row = dataManager->types()->find( typeId );
    if ( !row )
        return QString();
    return row->name();
}

QString TableModelsHelper::attributeName( int attributeId )
{
    const AttributeRow* row = dataManager->attributes()->find( attributeId );
    if ( !row )
        return QString();
    return row->name();
}

QString TableModelsHelper::viewName( int viewId )
{
    if ( viewId == 0 )
        return tr( "All Issues" );

    const ViewRow* row = dataManager->views()->find( viewId );
    if ( !row )
        return QString();
    return row->name();
}

QString TableModelsHelper::columnName( int column )
{
    switch ( column ) {
        case Column_Name:
            return tr( "Name" );
        case Column_ID:
            return tr( "ID" );
        case Column_CreatedDate:
            return tr( "Created Date" );
        case Column_CreatedBy:
            return tr( "Created By" );
        case Column_ModifiedDate:
            return tr( "Modified Date" );
        case Column_ModifiedBy:
            return tr( "Modified By" );
        case Column_Login:
            return tr( "Login" );
        case Column_Size:
            return tr( "Size" );
        case Column_Description:
            return tr( "Description" );
        case Column_Access:
            return tr( "Access Level" );
        case Column_Required:
            return tr( "Required" );
        case Column_Details:
            return tr( "Details" );
        case Column_DefaultValue:
            return tr( "Default Value" );
        case Column_OldValue:
            return tr( "Previous Value" );
        case Column_NewValue:
            return tr( "New Value" );
        case Column_Type:
            return tr( "Type" );
        case Column_Columns:
            return tr( "Columns" );
        case Column_SortBy:
            return tr( "Sort By" );
        case Column_Filter:
            return tr( "Filter" );
        case Column_Status:
            return tr( "Status" );
        case Column_EmailType:
            return tr( "Email Type" );
        default:
            if ( column > Column_UserDefined ) {
                const AttributeRow* attribute = dataManager->attributes()->find( column - Column_UserDefined );
                if ( attribute )
                    return attribute->name();
            }
            return QString();
    }
}

QString TableModelsHelper::extractValue( const IssueRow* row, int column )
{
    if ( column > Column_UserDefined ) {
        int attributeId = column - Column_UserDefined;
        const ValueRow* value = dataManager->values()->find( attributeId, row->issueId() );
        if ( !value )
            return QString();
        const AttributeRow* attribute = dataManager->attributes()->find( attributeId );
        if ( !attribute )
            return QString();
        Formatter formatter;
        return formatter.convertAttributeValue( attribute->definition(), value->value(), false );
    }

    switch ( column ) {
        case Column_Name:
            return row->name();
        case Column_ID:
            return TableModelsHelper::formatId( row->issueId() );
        case Column_CreatedDate: {
            Formatter formatter;
            return formatter.formatDateTime( row->createdDate(), true );
        }
        case Column_CreatedBy:
            return TableModelsHelper::userName( row->createdUser() );
        case Column_ModifiedDate: {
            Formatter formatter;
            return formatter.formatDateTime( row->modifiedDate(), true );
        }
        case Column_ModifiedBy:
            return TableModelsHelper::userName( row->modifiedUser() );
        default:
            return QString();
    }
}

bool TableModelsHelper::isIssueAdmin( int issueId )
{
    if ( dataManager->currentUserAccess() == AdminAccess )
        return true;

    const IssueRow* issue = dataManager->issues()->find( issueId );
    if ( issue )
        return isFolderAdmin( issue->folderId() );

    return false;
}

bool TableModelsHelper::isFolderAdmin( int folderId )
{
    if ( dataManager->currentUserAccess() == AdminAccess )
        return true;

    const FolderRow* folder = dataManager->folders()->find( folderId );
    if ( folder )
        return isProjectAdmin( folder->projectId() );

    return false;
}

bool TableModelsHelper::isProjectAdmin( int projectId )
{
    if ( dataManager->currentUserAccess() == AdminAccess )
        return true;

    const MemberRow* member = dataManager->members()->find( dataManager->currentUserId(), projectId );
    if ( member && member->access() == AdminAccess )
        return true;

    return false;
}

QString TableModelsHelper::tr( const char* text )
{
    return qApp->translate( "TableModelsHelper", text );
}
