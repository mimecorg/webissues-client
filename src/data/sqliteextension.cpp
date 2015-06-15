/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2015 WebIssues Team
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

#include "sqliteextension.h"

#include <QString>
#include <QRegExp>

#ifndef SQLITE_HAS_CODEC
#define SQLITE_HAS_CODEC
#endif

#include <sqlite3.h>

Q_DECLARE_OPAQUE_POINTER( sqlite3* )
Q_DECLARE_METATYPE( sqlite3* )

static int localeCompare( void* /*arg*/, int len1, const void* data1, int len2, const void* data2 )
{
    QString string1 = QString::fromRawData( reinterpret_cast<const QChar*>( data1 ), len1 / sizeof( QChar ) );
    QString string2 = QString::fromRawData( reinterpret_cast<const QChar*>( data2 ), len2 / sizeof( QChar ) );

    return QString::localeAwareCompare( string1, string2 );
}

static int nocaseCompare( void* /*arg*/, int len1, const void* data1, int len2, const void* data2 )
{
    QString string1 = QString::fromRawData( reinterpret_cast<const QChar*>( data1 ), len1 / sizeof( QChar ) );
    QString string2 = QString::fromRawData( reinterpret_cast<const QChar*>( data2 ), len2 / sizeof( QChar ) );

    return QString::compare( string1, string2, Qt::CaseInsensitive );
}

static void regexpFunction( sqlite3_context* context, int /*argc*/, sqlite3_value** argv )
{
    int len1 = sqlite3_value_bytes16( argv[ 0 ] );
    const void* data1 = sqlite3_value_text16( argv[ 0 ] );
    int len2 = sqlite3_value_bytes16( argv[ 1 ] );
    const void* data2 = sqlite3_value_text16( argv[ 1 ] );

    if ( !data1 || !data2 )
        return;

    // do not use fromRawData for pattern string because it may be cached internally by the regexp engine
    QString string1( reinterpret_cast<const QChar*>( data1 ), len1 / sizeof( QChar ) );
    QString string2 = QString::fromRawData( reinterpret_cast<const QChar*>( data2 ), len2 / sizeof( QChar ) );

    QRegExp pattern( string1, Qt::CaseInsensitive );

    bool match = pattern.exactMatch( string2 );

    sqlite3_result_int( context, match ? 1 : 0 );
}

void installSQLiteExtension( const QSqlDatabase& database )
{
    database.exec( "PRAGMA encoding = \"UTF-16\"" );

    sqlite3* db = qvariant_cast<sqlite3*>( database.driver()->handle() );

    if ( !db )
        return;

    sqlite3_create_collation( db, "LOCALE", SQLITE_UTF16, NULL, &localeCompare );
    sqlite3_create_collation( db, "NOCASE", SQLITE_UTF16, NULL, &nocaseCompare );

    sqlite3_create_function( db, "regexp", 2, SQLITE_UTF16, NULL, &regexpFunction, NULL, NULL );
}
