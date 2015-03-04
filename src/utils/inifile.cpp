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

#include "inifile.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>

IniFile::IniFile()
{
}

IniFile::IniFile( const QString& path )
{
    load( path );
}

IniFile::~IniFile()
{
}

bool IniFile::load( const QString& path )
{
    QString group;

    QFile file( path );

    if ( !file.open( QIODevice::ReadOnly ) )
        return false;

    QTextStream stream( &file );
    stream.setCodec( "UTF-8" );

    int i = 0;

    QRegExp iniRegExp( "([^=]*[^= ]) *= *(.*)", Qt::CaseSensitive, QRegExp::RegExp2 );

    while ( !stream.atEnd() ) {
        QString line = stream.readLine();
        i++;

        if ( line.isEmpty() || line.at( 0 ) == QLatin1Char( '#' ) || line.at( 0 ) == QLatin1Char( ';' ) )
            continue;

        if ( line.at( 0 ) == QLatin1Char( '[' ) ) {
            if ( line.length() > 2 && line.at( line.length() - 1 ) == QLatin1Char( ']' ) ) {
                group = line.mid( 1, line.length() - 2 );
                if ( !m_groups.contains( group ) )
                    m_groups.insert( group, GroupData() );
            } else {
                qWarning() << "Syntax error in" << path << "at line" << i;
            }
        } else {
            if ( !group.isEmpty() && iniRegExp.exactMatch( line ) ) {
                QString key = iniRegExp.cap( 1 );
                QString value = iniRegExp.cap( 2 );
                m_groups[ group ].m_values.insert( key, value );
            } else {
                qWarning() << "Syntax error in" << path << "at line" << i;
            }
        }
    }

    return true;
}

QStringList IniFile::groups() const
{
    return m_groups.keys();
}

QStringList IniFile::keys( const QString& group ) const
{
    return m_groups.value( group ).m_values.keys();
}

QMap<QString, QString> IniFile::values( const QString& group ) const
{
    return m_groups.value( group ).m_values;
}

QString IniFile::value( const QString& group, const QString& key ) const
{
    return m_groups.value( group ).m_values.value( key );
}
