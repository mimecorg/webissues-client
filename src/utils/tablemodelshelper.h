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

#ifndef TABLEMODELSHELPER_H
#define TABLEMODELSHELPER_H

#include <QString>

class IssueRow;

/**
* Functions for formatting various values.
*
* This functions are mostly used by the table models and other
* UI elements which display information in a standarized way.
*/
class TableModelsHelper
{
public:
    /**
    * Format an item identifier.
    * The identifier is prepended with a # like in links to items.
    */
    static QString formatId( int id );

    /**
    * Format the size of a file.
    * The size is formatted using bytes, kilobytes or megabytes,
    * whichever is most appropriate.
    */
    static QString formatSize( int size );

    /**
    * Return the name of the user with given identifier.
    */
    static QString userName( int userId );

    /**
    * Return the name of the issue type with given identifier.
    */
    static QString typeName( int typeId );

    /**
    * Return the name of the attribute with given identifier.
    */
    static QString attributeName( int attributeId );

    /**
    * Return the name of the view with given identifier.
    */
    static QString viewName( int viewId );

    /**
    * Return the name of the given column.
    */
    static QString columnName( int column );

    /**
    * Return the formatted value of the given column.
    */
    static QString extractValue( const IssueRow* row, int column );

    /**
    * Return @c true if user is administrator of given issue.
    */
    static bool isIssueAdmin( int issueId );

    /**
    * Return @c true if user is administrator of given folder.
    */
    static bool isFolderAdmin( int folderId );

    /**
    * Return @c true if user is administrator of given project.
    */
    static bool isProjectAdmin( int projectId );

private:
    static QString tr( const char* text );
};

#endif
