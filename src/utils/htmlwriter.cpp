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

#include "htmlwriter.h"

#include "utils/htmltext.h"

#include <QStringList>
#include <QTextDocument>
#include <QFile>
#include <QTextStream>

HtmlWriter::HtmlWriter() :
    m_embedded( false )
{
}

HtmlWriter::~HtmlWriter()
{
}

void HtmlWriter::setTitle( const QString& title )
{
    m_title = title;
}

void HtmlWriter::setEmbedded( bool on )
{
    m_embedded = on;
}

void HtmlWriter::createLayout()
{
    pushTag( "div", "class=\"sub-pane-wrapper\"" );
    pushTag( "table", "class=\"sub-pane-layout\"" );
    pushTag( "tr" );
}

void HtmlWriter::appendLayoutRow()
{
    popTag( "td" );
    popTag( "tr" );
    pushTag( "tr" );
}

void HtmlWriter::beginCell( Pane pane, int mergeColumns )
{
    popTag( "td" );

    QStringList attributes;

    if ( mergeColumns > 1 )
        attributes.append( QString( "colspan=\"%1\"" ).arg( mergeColumns ) );

    switch ( pane ) {
        case TopPane:
            attributes.append( "class=\"top-sub-pane\"" );
            break;
        case BottomPane:
            attributes.append( "class=\"bottom-sub-pane\"" );
            break;
    }

    pushTag( "td", attributes.join( " " ) );
}

void HtmlWriter::endLayout()
{
    popTag( "td" );
    popTag( "tr" );
    popTag( "table" );
    popTag( "div" );
}

void HtmlWriter::beginHistoryItem()
{
    pushTag( "div", "class=\"history-item\"" );
}

void HtmlWriter::endHistoryItem()
{
    popTag( "div" );
}

void HtmlWriter::getTagAndAttributes( HtmlWriter::BlockStyle style, QString& tag, QString& attributes )
{
    switch ( style ) {
    case Header2Block:
        tag = "h2";
        break;
    case Header3Block:
        tag = "h3";
        break;
    case Header4Block:
        tag = "h4";
        break;
    case FloatBlock:
        tag = "div";
        attributes = "style=\"float: right\"";
        break;
    case HistoryLinksBlock:
        tag = "div";
        attributes = "class=\"history-links\"";
        break;
    case HistoryInfoBlock:
        tag = "div";
        attributes = "class=\"history-info\"";
        break;
    case CommentBlock:
        tag = "div";
        attributes = "class=\"comment-text\"";
        break;
    case AttachmentBlock:
        tag = "div";
        attributes = "class=\"attachment\"";
        break;
    case NoItemsBlock:
        tag = "div";
        attributes = "class=\"noitems\"";
        break;
    case EditedBlock:
        tag = "span";
        attributes = "class=\"edited\"";
        break;
    }
}

void HtmlWriter::writeBlock( const HtmlText& text, BlockStyle style )
{
    QString tag;
    QString attributes;
    getTagAndAttributes( style, tag, attributes );

    pushTag( tag, attributes );
    m_body += text.toString();
    popTag( tag );
}

void HtmlWriter::writeNestedBlock( const HtmlText& text, BlockStyle style, const HtmlText& nestedText, BlockStyle nestedStyle )
{
    QString tag;
    QString attributes;
    getTagAndAttributes( style, tag, attributes );

    QString nestedTag;
    QString nestedAttributes;
    getTagAndAttributes( nestedStyle, nestedTag, nestedAttributes );

    pushTag( tag, attributes );
    m_body += text.toString();
    m_body += QLatin1String( "&nbsp; " );
    pushTag( nestedTag, nestedAttributes );
    m_body += nestedText.toString();
    popTag( nestedTag );
    popTag( tag );
}

void HtmlWriter::writeBulletList( const QList<HtmlText>& items )
{
    pushTag( "ul" );

    foreach ( const HtmlText& item, items ) {
        pushTag( "li", "class=\"changes\"" );
        m_body += item.toString();
        popTag( "li" );
    }

    popTag( "ul" );
}

void HtmlWriter::writeInfoList( const QStringList& headers, const QList<HtmlText>& values, bool multiLine )
{
    pushTag( "table", "class=\"info-list\"" );

    for ( int i = 0; i < headers.count(); i++ ) {
        pushTag( "tr" );

        pushTag( "td" );
        m_body += headers.at( i ).toHtmlEscaped();
        popTag( "td" );

        pushTag( "td", multiLine ? "class=\"multi-line\"" : QString() );
        m_body += values.at( i ).toString();
        popTag( "td" );

        popTag( "tr" );
    }

    popTag( "table" );
}

void HtmlWriter::createTable( const QStringList& headers )
{
    pushTag( "table", "class=\"grid\"" );
    pushTag( "tr" );

    foreach ( const QString& header, headers ) {
        pushTag( "th" );
        m_body += header.toHtmlEscaped();
        popTag( "th" );
    }

    popTag( "tr" );
}

void HtmlWriter::appendTableRow( const QList<HtmlText>& cells )
{
    pushTag( "tr" );

    foreach ( const HtmlText& cell, cells ) {
        pushTag( "td" );
        m_body += cell.toString();
        popTag( "td" );
    }

    popTag( "tr" );
}

void HtmlWriter::endTable()
{
    popTag( "table" );
}

static QString readFile( const QString& path )
{
    QFile file( path );
    if ( !file.open( QFile::ReadOnly ) )
        return QString();
    QTextStream stream( &file );
    return stream.readAll();
}

QString HtmlWriter::toHtml()
{
    popAll();

    QString html;
    html += QLatin1String( "<!DOCTYPE html>\n" );
    html += QLatin1String( "<html>\n" );
    html += QLatin1String( "<head>\n" );
    html += QLatin1String( "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />\n" );
    html += QString( "<title>%1</title>\n" ).arg( m_title.toHtmlEscaped() );
    if ( m_embedded ) {
        html += QLatin1String( "<style type=\"text/css\">\n" );
        html += readFile( ":/resources/style.css" );
        html += QLatin1String( "</style>\n" );
    } else {
        html += QLatin1String( "<link rel=\"stylesheet\" href=\"qrc:/resources/style.css\" type=\"text/css\" />\n" );
        html += QLatin1String( "<script type=\"text/javascript\" src=\"qrc:/resources/prettify.js\"></script>\n" );
        html += QLatin1String( "<script type=\"text/javascript\">addEventListener( 'load', function () { prettyPrint(); }, false );</script>\n" );
    }
    html += QLatin1String( "</head>\n" );
    html += QLatin1String( "<body>\n" );
    html += m_body;
    html += QLatin1String( "</body>\n" );
    html += QLatin1String( "</html>\n" );

    return html;
}

void HtmlWriter::pushTag( const QString& tag, const QString& attributes )
{
    m_tags.push( tag );
    m_body += QLatin1Char( '<' );
    m_body += tag;
    if ( !attributes.isEmpty() ) {
        m_body += QLatin1Char( ' ' );
        m_body += attributes;
    }
    m_body += QLatin1Char( '>' );
}

void HtmlWriter::popTag( const QString& tag )
{
    if ( !m_tags.isEmpty() && m_tags.top() == tag ) {
        m_body += QLatin1Char( '<' );
        m_body += QLatin1Char( '/' );
        m_body += m_tags.pop();
        m_body += QLatin1Char( '>' );
        m_body += QLatin1Char( '\n' );
    }
}

void HtmlWriter::popAll()
{
    while ( !m_tags.isEmpty() ) {
        m_body += QLatin1Char( '<' );
        m_body += QLatin1Char( '/' );
        m_body += m_tags.pop();
        m_body += QLatin1Char( '>' );
        m_body += QLatin1Char( '\n' );
    }
}
