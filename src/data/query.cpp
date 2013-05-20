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

#include "query.h"

Query::Query( const QSqlDatabase& database ) :
    m_prepared( false ),
    m_valid( false ),
    m_query( database )
{
    m_query.setForwardOnly( true );
}

Query::Query( const QString& query, const QSqlDatabase& database ) :
    m_queryText( query ),
    m_prepared( false ),
    m_valid( false ),
    m_query( database )
{
    m_query.setForwardOnly( true );
}

Query::~Query()
{
}

void Query::setQuery( const QString& query )
{
    m_queryText = query;
    m_prepared = false;
    m_valid = false;
}

bool Query::exec()
{
    if ( !ensurePrepared() )
        return false;

    return m_query.exec();
}

bool Query::exec( const QVariant& p1 )
{
    if ( !ensurePrepared() )
        return false;

    m_query.addBindValue( p1 );

    return m_query.exec();
}

bool Query::exec( const QVariant& p1, const QVariant& p2 )
{
    if ( !ensurePrepared() )
        return false;

    m_query.addBindValue( p1 );
    m_query.addBindValue( p2 );

    return m_query.exec();
}

bool Query::exec( const QVariant& p1, const QVariant& p2, const QVariant& p3 )
{
    if ( !ensurePrepared() )
        return false;

    m_query.addBindValue( p1 );
    m_query.addBindValue( p2 );
    m_query.addBindValue( p3 );

    return m_query.exec();
}

bool Query::exec( const QVariant& p1, const QVariant& p2, const QVariant& p3, const QVariant& p4 )
{
    if ( !ensurePrepared() )
        return false;

    m_query.addBindValue( p1 );
    m_query.addBindValue( p2 );
    m_query.addBindValue( p3 );
    m_query.addBindValue( p4 );

    return m_query.exec();
}

bool Query::exec( const QVariantList& params )
{
    if ( !ensurePrepared() )
        return false;

    foreach ( const QVariant& param, params )
        m_query.addBindValue( param );

    return m_query.exec();
}

bool Query::execQuery( const QString& query )
{
    setQuery( query );

    return exec();
}

bool Query::execQuery( const QString& query, const QVariant& p1 )
{
    setQuery( query );

    return exec( p1 );
}

bool Query::execQuery( const QString& query, const QVariant& p1, const QVariant& p2 )
{
    setQuery( query );

    return exec( p1, p2 );
}

bool Query::execQuery( const QString& query, const QVariant& p1, const QVariant& p2, const QVariant& p3 )
{
    setQuery( query );

    return exec( p1, p2, p3 );
}

bool Query::execQuery( const QString& query, const QVariant& p1, const QVariant& p2, const QVariant& p3, const QVariant& p4 )
{
    setQuery( query );

    return exec( p1, p2, p3, p4 );
}

bool Query::execQuery( const QString& query, const QVariantList& params )
{
    setQuery( query );

    return exec( params );
}

bool Query::ensurePrepared()
{
    if ( !m_prepared ) {
        m_valid = m_query.prepare( m_queryText );
        m_prepared = true;
    }

    return m_valid;
}

bool Query::next()
{
    return m_valid && m_query.next();
}

QVariant Query::value( int i ) const
{
    if ( m_valid )
        return m_query.value( i );
    return QVariant();
}

QVariant Query::readScalar()
{
    if ( m_valid && m_query.next() )
        return m_query.value( 0 );
    return QVariant();
}

int Query::numRowsAffected() const
{
    if ( m_valid )
        return m_query.numRowsAffected();
    return 0;
}
