/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2014 WebIssues Team
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

#ifndef ISSUETYPECACHE_H
#define ISSUETYPECACHE_H

#include "utils/definitioninfo.h"

#include <QObject>
#include <QPair>
#include <QHash>

/**
* Cache for information related to an issue type.
*/
class IssueTypeCache : public QObject
{
public:
    /**
    * Constructor.
    * @param typeId Identifier of the issue type.
    * @param parent The parent object.
    */
    IssueTypeCache( int typeId, QObject* parent );

    /**
    * Destructor.
    */
    ~IssueTypeCache();

public:
    /**
    * Return the list of attributes in the default order.
    */
    const QList<int>& attributes() { return m_attributes; }

    /**
    * Return the list of available columns for the issue type.
    */
    QList<int> availableColumns( bool withLocation = false ) const;

    /**
    * Return the name of the given attribute.
    */
    QString attributeName( int attributeId ) const;

    /**
    * Return the definition of the given attribute.
    */
    DefinitionInfo attributeDefinition( int attributeId ) const;

    /**
    * Return the definition of the default view.
    */
    const DefinitionInfo& defaultView() const { return m_defaultView; }

    /**
    * Return the list of columns for given view.
    */
    QList<int> viewColumns( const DefinitionInfo& info, bool withLocation = false ) const;

    /**
    * Return the sort order for given view.
    */
    QPair<int, Qt::SortOrder> viewSortOrder( const DefinitionInfo& info ) const;

    /**
    * Return the filters for given view.
    */
    QList<DefinitionInfo> viewFilters( const DefinitionInfo& info ) const;

    /**
    * Return the definition of a filter value for given column.
    */
    DefinitionInfo filterValueInfo( int column ) const;

    /**
    * Return available filter operators for given column.
    */
    QStringList availableOperators( int column ) const;

    /**
    * Return default column widths.
    */
    QMap<int, int> defaultWidths() const;

    /**
    * Return the identifier of the initial view.
    */
    int initialViewId() const { return m_initialViewId; }

private:
    QList<int> m_attributes;

    QHash<int, QString> m_attributeNames;
    QHash<int, DefinitionInfo> m_attributeDefinitions;

    DefinitionInfo m_defaultView;

    int m_initialViewId;
};

#endif
