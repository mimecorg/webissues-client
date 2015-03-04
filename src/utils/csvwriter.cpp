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

#include "csvwriter.h"

CsvWriter::CsvWriter()
{
}

CsvWriter::~CsvWriter()
{
}

void CsvWriter::appendRow( const QStringList& cells )
{
    QStringList formatted = cells;

    for ( int i = 0; i < formatted.count(); i++ ) {
        QString field = formatted.at( i );
        if ( field.startsWith( ' ' ) || field.endsWith( ' ' ) || field.contains( '"' )
             || field.contains( ',' ) || field.contains( '\n' ) || field == QLatin1String( "ID" ) ) {
            field.replace( "\"", "\"\"" );
            formatted.replace( i, '"' + field + '"' );
        }
    }

    m_rows.append( formatted.join( "," ) );
}

QString CsvWriter::toString() const
{
    return m_rows.join( "\r\n" );
}
