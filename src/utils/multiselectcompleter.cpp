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

#include "multiselectcompleter.h"

#include <QLineEdit>

MultiSelectCompleter::MultiSelectCompleter( const QStringList& items, QObject* parent ) : QCompleter( items, parent ),
    m_multiSelect( true )
{
}

MultiSelectCompleter::~MultiSelectCompleter()
{
}

void MultiSelectCompleter::setMultiSelect( bool multi )
{
    m_multiSelect = multi;
}

QString MultiSelectCompleter::pathFromIndex( const QModelIndex& index ) const
{
    QString path = QCompleter::pathFromIndex( index );

    if ( m_multiSelect ) {
        QString text = static_cast<QLineEdit*>( widget() )->text();

        int pos = text.lastIndexOf( ',' );
        if ( pos >= 0 )
            path = text.left( pos ) + ", " + path;
    }

    return path;
}

QStringList MultiSelectCompleter::splitPath( const QString& path ) const
{
    if ( m_multiSelect ) {
        int pos = path.lastIndexOf( ',' ) + 1;

        while ( pos < path.length() && path.at( pos ) == QLatin1Char( ' ' ) )
            pos++;

        return QStringList( path.mid( pos ) );
    } else {
        return QCompleter::splitPath( path );
    }
}
