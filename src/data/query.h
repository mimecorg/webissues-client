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

#ifndef QUERY_H
#define QUERY_H

#include <QSqlQuery>
#include <QVariant>

/**
* Wrapper for the QSqlQuery.
*/
class Query
{
public:
    /**
    * Constructor.
    */
    Query( const QSqlDatabase& database = QSqlDatabase() );

    /**
    * Constructor.
    */
    Query( const QString& query, const QSqlDatabase& database = QSqlDatabase() );

    /**
    * Destructor.
    */
    ~Query();

public:
    /**
    * Set the text of the query.
    */
    void setQuery( const QString& query );

    /**
    * Execute the current query.
    */
    bool exec();

    /**
    * Execute the current query with a bound parameter.
    */
    bool exec( const QVariant& p1 );

    /**
    * Execute the current query with two bound parameters.
    */
    bool exec( const QVariant& p1, const QVariant& p2 );

    /**
    * Execute the current query with three bound parameters.
    */
    bool exec( const QVariant& p1, const QVariant& p2, const QVariant& p3 );

    /**
    * Execute the current query with four bound parameters.
    */
    bool exec( const QVariant& p1, const QVariant& p2, const QVariant& p3, const QVariant& p4 );

    /**
    * Execute the current query with any number of parameters.
    */
    bool exec( const QVariantList& params );

    /**
    * Execute the given query.
    */
    bool execQuery( const QString& query );

    /**
    * Execute the given query with a bound parameter.
    */
    bool execQuery( const QString& query, const QVariant& p1 );

    /**
    * Execute the given query with two bound parameters.
    */
    bool execQuery( const QString& query, const QVariant& p1, const QVariant& p2 );

    /**
    * Execute the given query with three bound parameters.
    */
    bool execQuery( const QString& query, const QVariant& p1, const QVariant& p2, const QVariant& p3 );

    /**
    * Execute the given query with four bound parameters.
    */
    bool execQuery( const QString& query, const QVariant& p1, const QVariant& p2, const QVariant& p3, const QVariant& p4 );

    /**
    * Execute the given query with any number of parameters.
    */
    bool execQuery( const QString& query, const QVariantList& params );

    /**
    * Move to the next record.
    */
    bool next();

    /**
    * Retrieve the value of the given cell in current record.
    */
    QVariant value( int i ) const;

    /**
    * Read a scalar value from the next record.
    */
    QVariant readScalar();

    /**
    * Return the number of rows affected by the query.
    */
    int numRowsAffected() const;

private:
    bool ensurePrepared();

private:
    QString m_queryText;

    bool m_prepared;
    bool m_valid;

    QSqlQuery m_query;
};

#endif
