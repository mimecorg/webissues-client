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

#include "definitioninfo.h"

#include <QStringList>
#include <QCache>

DefinitionInfo::DefinitionInfo()
{
}

DefinitionInfo::~DefinitionInfo()
{
}

void DefinitionInfo::setMetadata( const QString& key, const QVariant& value )
{
    if ( !value.isNull() )
        m_metadata.insert( key, value );
    else
        m_metadata.remove( key );
}

QVariant DefinitionInfo::metadata( const QString& key ) const
{
    return m_metadata.value( key );
}

QString DefinitionInfo::toString() const
{
    if ( isEmpty() )
        return QString();

    QString result = m_type;

    for ( QMap<QString, QVariant>::const_iterator it = m_metadata.constBegin(); it != m_metadata.constEnd(); ++it ) {
        QString value;
        if ( it.value().type() == QVariant::StringList ) {
            QStringList list = it.value().toStringList();
            value = "{";
            for ( int i = 0; i < list.count(); i++ ) {
                if ( i > 0 )
                    value += QLatin1String( "," );
                value += quoteString( list.at( i ) );
            }
            value += QLatin1String( "}" );
        } else if ( it.value().type() == QVariant::String ) {
            value = quoteString( it.value().toString() );
        } else {
            value = it.value().toString();
        }
        result += QString( " %1=%2" ).arg( it.key(), value );
    }

    return result;
}

QString DefinitionInfo::quoteString( const QString& string )
{
    QString result = "\"";
    int length = string.length();
    for ( int i = 0; i < length; i++ ) {
        QChar ch = string[ i ];
        if  ( ch == QLatin1Char( '\\' ) || ch == QLatin1Char( '\"' ) || ch == QLatin1Char( '\n' ) || ch == QLatin1Char( '\t' ) ) {
            result += QLatin1Char( '\\' );
            if ( ch == QLatin1Char( '\n' ) )
                ch = QLatin1Char( 'n' );
            else if ( ch == QLatin1Char( '\t' ) )
                ch = QLatin1Char( 't' );
        }
        result += ch;
    }
    result += QLatin1Char( '\"' );
    return result;
}

DefinitionInfo DefinitionInfo::fromString( const QString& text )
{
    if ( text.isEmpty() )
        return DefinitionInfo();

    static QCache<QString, DefinitionInfo> definitionsCache( 10000 );

    DefinitionInfo* info = definitionsCache.object( text );
    if ( info )
        return *info;

    info = new DefinitionInfo();
    definitionsCache.insert( text, info, text.length() );

    QString patternNumber = "-?\\d+";
    QString patternString = "\"(?:\\\\[\"\\\\nt]|[^\"\\\\])*\"";
    QString patternArray = QString( "\\{(?:%1(?:,%2)*)?\\}" ).arg( patternString, patternString );
    QString patternKey = "[a-z0-9]+(?:-[a-z0-9]+)*";
    QString patternKeyAndValue = QString( "(%1)=(%2|%3|%4)" ).arg( patternKey, patternNumber, patternString, patternArray );

    QRegExp definitionRegExp( QString( "([A-Z]+)(( %1)*)" ).arg( patternKeyAndValue ) );

    if ( !definitionRegExp.exactMatch( text ) )
        return *info;

    info->setType( definitionRegExp.cap( 1 ) );

    QString attributes = definitionRegExp.cap( 2 );

    QRegExp metadataRegExp( patternKeyAndValue );
    QRegExp stringRegExp( patternString );

    int i = 0;
    while ( ( i = metadataRegExp.indexIn( attributes, i ) ) != -1 ) {
        QString key = metadataRegExp.cap( 1 );
        QString value = metadataRegExp.cap( 2 );
        if ( value[ 0 ] == QLatin1Char( '\"' ) ) {
            info->setMetadata( key, unquoteString( value ) );
        } else if ( value[ 0 ] == QLatin1Char( '{' ) ) {
            QStringList list;
            int j = 0;
            while ( ( j = stringRegExp.indexIn( value, j ) ) != -1 ) {
                list.append( unquoteString( stringRegExp.cap( 0 ) ) );
                j += stringRegExp.matchedLength();
            }
            info->setMetadata( key, list );
        } else {
            info->setMetadata( key, value.toInt() );
        }
        i += metadataRegExp.matchedLength();
    }

    return *info;
}

QString DefinitionInfo::unquoteString( const QString& string )
{
    QString result = "";
    int length = string.length();
    for ( int i = 1; i < length - 1; i++ ) {
        QChar ch = string[ i ];
        if ( ch == QLatin1Char( '\\' ) ) {
            ch = string[ ++i ];
            if ( ch == QLatin1Char( 'n' ) )
                ch = QLatin1Char( '\n' );
            else if ( ch == QLatin1Char( 't' ) )
                ch = QLatin1Char( '\t' );
        }
        result += ch;
    }
    return result;
}
