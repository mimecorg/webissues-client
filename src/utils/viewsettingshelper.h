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

#ifndef VIEWSETTINGSHELPER_H
#define VIEWSETTINGSHELPER_H

#include <QStringList>
#include <QPair>

class DefinitionInfo;

/**
* Functions providing information about view settings.
*/
class ViewSettingsHelper
{
public:
    /**
    * Return the list of attributes in given order.
    */
    static QList<int> attributeOrder( int typeId, const QString& attributes );

    /**
    * Return names of attributes.
    */
    static QString attributeNames( const QList<int>& attributeIds );

    /**
    * Return the list of columns for given view.
    */
    static QList<int> viewColumns( int typeId, const DefinitionInfo& info );

    /**
    * Return names of view columns.
    */
    static QString columnNames( const QList<int>& columns );

    /**
    * Return the sort order for given view.
    */
    static QPair<int, Qt::SortOrder> viewSortOrder( int typeId, const DefinitionInfo& info );

    /**
    * Return information about sort order.
    */
    static QString sortOrderInfo(const QPair<int, Qt::SortOrder>& order );

    /**
    * Return the filters for given view.
    */
    static QList<DefinitionInfo> viewFilters( int typeId, const DefinitionInfo& info );

    /**
    * Return information about filter conditions.
    */
    static QString filtersInfo( const QList<DefinitionInfo>& filters );

    /**
    * Return the list of available columns for given issue type.
    */
    static QList<int> availableColumns( int typeId );

    /**
    * Return the definition of a filter value for given column.
    */
    static DefinitionInfo filterValueInfo( int column );

    static QStringList availableOperators( int column );

    static QString operatorName( const QString& type );

private:
    static QString tr( const char* text );
};

#endif
