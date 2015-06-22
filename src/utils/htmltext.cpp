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

#include "htmltext.h"

#include <QRegExp>
#include <QTextDocument>

HtmlText::HtmlText( Flags flags ) :
    m_flags( flags )
{
}

HtmlText::HtmlText( const QString& text ) :
    m_flags( 0 ),
    m_html( text.toHtmlEscaped() )
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
    m_html += text.toHtmlEscaped();
}

void HtmlText::appendLink( const QString& text, const QString& url )
{
    m_html += QString( "<a href=\"%1\">%2</a>" ).arg( url.toHtmlEscaped(), text.toHtmlEscaped() );
}

void HtmlText::appendParsed( const QString& text )
{
    QRegExp linkExp( "\\b(?:mailto:)?[\\w.%+-]+@[\\w.-]+\\.[a-z]{2,}\\b"
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
        QString url = convertUrl( link, m_flags );

        appendLink( link, url );

        pos += linkExp.matchedLength();
    }
}

HtmlText HtmlText::parse( const QString& text, Flags flags )
{
    HtmlText result( flags );
    result.appendParsed( text );
    return result;
}

void HtmlText::appendImage( const QString& image, const QString& text )
{
    m_html += QString( "<img src=\"qrc:/icons/%1-16.png\" alt=\"%2\" title=\"%2\" width=\"16\" height=\"16\" class=\"icon\">" ).arg( image, text.toHtmlEscaped() );
}

void HtmlText::appendImageAndText( const QString& image, const QString& text )
{
    appendImage( image, text );
    m_html += QLatin1Char( ' ' );
    appendText( text );
}

void HtmlText::appendImageAndTextLink( const QString& image, const QString& text, const QString& url )
{
    m_html += QString( "<a href=\"%1\">" ).arg( url.toHtmlEscaped() );
    appendImage( image, text );
    m_html += QLatin1String( "</a>\n" );
    appendLink( text, url );
}

void HtmlText::createAnchor( const QString& name )
{
    m_html += QString( "<a class=\"anchor\" name=\"%1\">" ).arg( name.toHtmlEscaped() );
}

void HtmlText::endAnchor()
{
    m_html += QLatin1String( "</a>\n" );;
}

QString HtmlText::convertUrl( const QString& url, Flags flags )
{
    if ( url.at( 0 ) == QLatin1Char( '#' ) )
        return ( ( flags & NoInternalLinks ) ? "#item" : "id:" ) + url.mid( 1 );
    else if ( url.startsWith( QLatin1String( "www." ), Qt::CaseInsensitive ) )
        return "http://" + url;
    else if ( url.startsWith( QLatin1String( "ftp." ), Qt::CaseInsensitive ) )
        return "ftp://" + url;
    else if ( url.startsWith( QLatin1String( "\\\\" ) ) )
        return "file:///" + url;
    else if ( !url.contains( QLatin1Char( ':' ) ) )
        return "mailto:" + url;
    else
        return url;
}

QString HtmlText::convertTabsToSpaces( const QString& text )
{
    int column = 0;
    return convertTabsToSpaces( text, column );
}

QString HtmlText::convertTabsToSpaces( const QString& text, int& column )
{
    QString result;
    int last = 0;

    for ( int i = 0; i < text.length(); i++ ) {
        QChar ch = text.at( i );

        if ( ch == QLatin1Char( '\n' ) ) {
            column = 0;
        } else if ( ch == QLatin1Char( '\t' ) ) {
            if ( i > last )
                result += text.midRef( last, i - last );
            int count = 8 - ( column % 8 );
            for ( int j = 0; j < count; j++ )
                result += QLatin1Char( ' ' );
            column += count;
            last = i + 1;
        } else {
            column++;
        }
    }

    if ( last == 0 )
        return text;

    if ( last < text.length() )
        result += text.midRef( last );

    return result;
}
