/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2012 WebIssues Team
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

#include "textwithlinks.h"

#include <QRegExp>

TextWithLinks::TextWithLinks( Flags flags /*= 0*/ ) :
    m_flags( flags )
{
}

TextWithLinks::TextWithLinks( const QString& text ) :
    m_flags( 0 )
{
    m_texts.append( text );
    m_urls.append( QString() );
}

TextWithLinks::~TextWithLinks()
{
}

void TextWithLinks::clear()
{
    m_texts.clear();
    m_urls.clear();
}

void TextWithLinks::appendText( const QString& text )
{
    if ( !m_texts.isEmpty() && m_urls.last().isEmpty() ) {
        m_texts.last().append( text );
    } else {
        m_texts.append( text );
        m_urls.append( QString() );
    }
}

void TextWithLinks::appendLink( const QString& text, const QString& url )
{
    m_texts.append( text );
    m_urls.append( url );
}

void TextWithLinks::appendParsed( const QString& text )
{
    QRegExp linkExp( "\\b(?:mailto:)?[\\w.%+-]+@[\\w.-]+\\.[a-z]{2,4}\\b"
        "|(?:\\b(?:(?:https?|ftp|file):\\/\\/|www\\.|ftp\\.)|\\\\\\\\)(?:\\([\\w+&@#\\/\\\\%=~|$?!:,.-]*\\)|[\\w+&@#\\/\\\\%=~|$?!:,.-])*(?:\\([\\w+&@#\\/\\\\%=~|$?!:,.-]*\\)|[\\w+&@#\\/\\\\%=~|$])"
        "|#\\d+\\b", Qt::CaseInsensitive );

    int pos = 0;
    for ( ; ; ) {
        int oldpos = pos;
        pos = linkExp.indexIn( text, pos );

        if ( pos < 0 ) {
            appendText( text.mid( oldpos ) );
            break;
        }

        if ( pos > oldpos )
            appendText( text.mid( oldpos, pos - oldpos ) );

        QString link = linkExp.cap( 0 );

        QString url;
        if ( link[ 0 ] == QLatin1Char( '#' ) )
            url = ( ( m_flags & NoInternalLinks ) ? "#id" : "id://" ) + link.mid( 1 );
        else if ( link.startsWith( QLatin1String( "www." ), Qt::CaseInsensitive ) )
            url = "http://" + link;
        else if ( link.startsWith( QLatin1String( "ftp." ), Qt::CaseInsensitive ) )
            url = "ftp://" + link;
        else if ( link.startsWith( QLatin1String( "\\\\" ) ) )
            url = "file:///" + link;
        else if ( !link.contains( QLatin1Char( ':' ) ) )
            url = "mailto:" + link;
        else
            url = link;

        appendLink( link, url );

        pos += linkExp.matchedLength();
    }
}

QString TextWithLinks::toHtml() const
{
    QString result;

    for ( int i = 0; i < m_texts.count(); i++ ) {
        if ( !m_urls.at( i ).isEmpty() )
            result += QString( "<a href=\"%1\">%2</a>" ).arg( Qt::escape( m_urls.at( i ) ), Qt::escape( m_texts.at( i ) ) );
        else
            result += Qt::escape( m_texts.at( i ) );
    }

    return result;
}

TextWithLinks TextWithLinks::parse( const QString& text, Flags flags /*= 0*/ )
{
    TextWithLinks result( flags );
    result.appendParsed( text );
    return result;
}
