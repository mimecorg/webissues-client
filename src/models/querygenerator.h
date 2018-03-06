/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2013 WebIssues Team
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

#ifndef QUERYGENERATOR_H
#define QUERYGENERATOR_H

#include <QObject>

class DefinitionInfo;

class QDateTime;

/**
* Generator for SQL query retrieving the list of issues.
*/
class QueryGenerator : public QObject
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param folderId Identifier of the folder.
    * @param viewId Identifier of the view or 0 to use the default view.
    */
    QueryGenerator( int folderId, int viewId );

    /**
    * Destructor.
    */
    ~QueryGenerator();

public:
    /**
    * Set the quick search text for the list.
    */
    void setSearchText( int column, const QString& text );

    /**
    * Return the identifier of the type of issues.
    */
    int typeId() const { return m_typeId; }

    /**
    * Override the list of columns for the view.
    */
    void setColumns( const QList<int>& columns );

    /**
    * Return the list of column types for the view.
    */
    const QList<int>& columns() const { return m_columns; }

    /**
    * Generate the SQL query.
    * @param allColumns If @c true, all columns are retrieved, otherwise only
    * the issue identifier and state is retrieved.
    */
    QString query( bool allColumns );

    /**
    * Return the bind arguments for the generated query.
    */
    const QList<QVariant>& arguments() const { return m_arguments; }

    /**
    * Return the list of sort expressions for each column.
    */
    QStringList sortColumns() const;

    /**
    * Return the column mapping for the view.
    */
    QList<int> columnMapping() const;

    /**
    * Return the index of the default sort column.
    */
    int sortColumn() const { return m_sortColumn; }

    /**
    * Return the default sort order.
    */
    Qt::SortOrder sortOrder() const { return m_sortOrder; }

private:
    QString generateSelect( bool allColumns );
    QString generateJoins( bool allColumns );
    QString generateConditions();

    QString convertUserValue( const QString& value ) const;
    QDateTime convertDateTimeValue( const QString& value, bool local ) const;

    QString makeNullCondition( const QString& expression, const QString& type );
    QString makeStringCondition( const QString& expression, const QString& type, const QString& value );
    QString makeNumericCondition( const QString& expression, const QString& type, const QVariant& value );
    QString makeDateCondition( const QString& expression, const QString& type, const QDateTime& value );

private:
    int m_folderId;
    int m_typeId;

    QList<int> m_columns;

    QList<DefinitionInfo> m_filters;

    int m_searchColumn;
    QString m_searchText;

    int m_sortColumn;
    Qt::SortOrder m_sortOrder;

    bool m_valid;
    QList<QVariant> m_arguments;
};

#endif
