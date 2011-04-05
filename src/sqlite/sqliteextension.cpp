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

#include "sqliteextension.h"

#include <QString>
#include <QVariant>

#include "sqlite3.h"

static int localeCompare( void* /*arg*/, int len1, const void* data1, int len2, const void* data2 )
{
    QString string1 = QString::fromRawData( reinterpret_cast<const QChar*>( data1 ), len1 / sizeof( QChar ) );
    QString string2 = QString::fromRawData( reinterpret_cast<const QChar*>( data2 ), len2 / sizeof( QChar ) );
    return QString::localeAwareCompare( string1, string2 );
}

void installSQLiteExtension( const QVariant& handle )
{
    sqlite3* ptr = *static_cast<sqlite3* const*>( handle.data() );

    sqlite3_create_collation( ptr, "LOCALE", SQLITE_UTF16, NULL, &localeCompare );
}
