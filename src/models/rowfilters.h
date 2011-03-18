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

#ifndef ROWFILTERS_H
#define ROWFILTERS_H

#include "rdb/abstractrowfilter.h"
#include "utils/definitioninfo.h"

class IssueRow;

/**
* Filter for issue rows using a list of conditions.
*
* Issue rows can be filtered using a list of conditions for column values
* and a quick search string. Either of them can be empty.
*/
class IssueRowFilter : public RDB::AbstractRowFilter
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent The parent object.
    */
    IssueRowFilter( QObject* parent );

    /**
    * Destructor.
    */
    ~IssueRowFilter();

public:
    /**
    * Set the string to search in given column.
    */
    void setQuickSearch( int column, const QString& text );

    /**
    * Return the quick search column.
    */
    int searchColumn() const { return m_searchColumn; }

    /**
    * Return the quick search text.
    */
    const QString& searchText() const { return m_searchText; }

    /**
    * Set the filter conditions.
    */
    void setFilters( const QList<DefinitionInfo> filters );

    /**
    * Return the filter conditions.
    */
    const QList<DefinitionInfo> filters() const { return m_filters; }

public: // overrides
    bool filterRow( int id );

private:
    bool matchValue( const IssueRow* row, const DefinitionInfo& filter );

    bool matchTextValue( const QString& type, const QString& value1, const QString& value2 );
    bool matchNumericValue( const QString& type, double value1, double value2 );
    bool matchDateTimeValue( const QString& type, const QDateTime& value1, const QDate& value2, bool local );

    QString convertUserValue( const QString& value );
    QString convertDateTimeValue( const QString& value );

private:
    int m_searchColumn;
    QString m_searchText;

    QList<DefinitionInfo> m_filters;
};

/**
* Filter for public or personal view rows.
*/
class ViewRowFilter : public RDB::AbstractRowFilter
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param isPersonal The type of views to filter.
    * @param parent The parent object.
    */
    ViewRowFilter( bool isPublic, QObject* parent );

    /**
    * Destructor.
    */
    ~ViewRowFilter();

public: // overrides
    bool filterRow( int id );

private:
    bool m_isPublic;
};

#endif
