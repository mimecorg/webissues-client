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

#include "htmltext.h"

#include <QRegExp>

HtmlText::HtmlText( Flags flags /*= 0*/ ) :
    m_flags( flags )
{
}

HtmlText::HtmlText( const QString& text ) :
    m_flags( 0 ),
    m_html( Qt::escape( text ) )
{
}

HtmlText::~HtmlText()
{
}

void HtmlText::clear()
{
    m_html.clear();
}

void HtmlText::appendText( const QString& text )
{
    m_html += Qt::escape( text );
}

void HtmlText::appendLink( const QString& text, const QString& url )
{
    m_html += QString( "<a href=\"%1\">%2</a>" ).arg( Qt::escape( url ), Qt::escape( text ) );
}

void HtmlText::appendParsed( const QString& text )
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
        if ( link.at( 0 ) == QLatin1Char( '#' ) )
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

HtmlText HtmlText::parse( const QString& text, Flags flags /*= 0*/ )
{
    HtmlText result( flags );
    result.appendParsed( text );
    return result;
}

void HtmlText::appendImage( const QString& image, const QString& text )
{
    m_html += QString( "<img src=\"qrc:/icons/%1-16.png\" alt=\"%2\" title=\"%2\" width=\"16\" height=\"16\" class=\"icon\" />" ).arg( image, Qt::escape( text ) );
}

void HtmlText::appendImageAndText( const QString& image, const QString& text )
{
    appendImage( image, text );
    m_html += QLatin1Char( ' ' );
    appendText( text );
}

void HtmlText::appendImageAndTextLink( const QString& image, const QString& text, const QString& url )
{
    m_html += QString( "<a href=\"%1\">" ).arg( Qt::escape( url ) );
    appendImage( image, text );
    m_html += QLatin1String( "</a>\n" );
    appendLink( text, url );
}

void HtmlText::createAnchor( const QString& name )
{
    m_html += QString( "<a class=\"anchor\" name=\"%1\">" ).arg( Qt::escape( name ) );
}

void HtmlText::endAnchor()
{
    m_html += QLatin1String( "</a>\n" );;
}