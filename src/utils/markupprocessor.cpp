/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2013 WebIssues Team
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

#include "markupprocessor.h"

HtmlText MarkupProcessor::parse( const QString& text, HtmlText::Flags flags )
{
    MarkupProcessor processor( text, flags );

    processor.next();
    processor.parse();

    HtmlText result( flags );
    result.m_html = processor.m_result;
    return result;
}

MarkupProcessor::MarkupProcessor( const QString& text, HtmlText::Flags flags ) :
    m_text( text ),
    m_flags( flags ),
    m_index( 0 ),
    m_matched( false ),
    m_token( 0 )
{
    m_regExp.setPattern( "\\n|`[^`\\n]+`"
        "|\\[\\/?(?:list|code|quote)(?:[ \\t][^]\\n]*)?\\](?:[ \\t]*\\n)?"
        "|\\[(?:(?:mailto:)?[\\w.%+-]+@[\\w.-]+\\.[a-z]{2,4}|(?:(?:https?|ftp|file):\\/\\/|www\\.|ftp\\.|\\\\\\\\)[\\w+&@#\\/\\\\%=~|$?!:,.()-]+|#\\d+)(?:[ \\t][^]\\n]*)?\\]");
    m_regExp.setCaseSensitivity( Qt::CaseInsensitive );
}

MarkupProcessor::~MarkupProcessor()
{
}

static int strcspn( const QString& text )
{
    int i = 0, len = text.length();
    while ( i < len ) {
        QChar ch = text.at( i );
        if ( ch == QLatin1Char( ' ' ) || ch == QLatin1Char( '\t' ) || ch == QLatin1Char( ']' ) )
            break;
        i++;
    }
    return i;
}

void MarkupProcessor::next()
{
    if ( m_index >= m_text.length() ) {
        m_token = T_END;
        return;
    }

    if ( !m_matched ) {
        int lastIndex = m_index;
        m_index = m_regExp.indexIn( m_text, m_index );

        if ( m_index < 0 ) {
            m_index = m_text.length();
            m_token = T_TEXT;
            m_value = m_rawValue = m_text.mid( lastIndex );
            return;
        }

        if ( m_index > lastIndex ) {
            m_token = T_TEXT;
            m_value = m_rawValue = m_text.mid( lastIndex, m_index - lastIndex );
            m_matched = true;
            return;
        }
    }

    m_index += m_regExp.matchedLength();
    m_matched = false;

    m_rawValue = m_regExp.cap( 0 );

    if ( m_rawValue.at( 0 ) == QLatin1Char( '[' ) ) {
        int index = strcspn( m_rawValue );
        m_value = m_rawValue.mid( 1, index - 1 );
        m_extra = m_rawValue.mid( index, m_rawValue.lastIndexOf( ']' ) - index ).trimmed();

        QString tag = m_value.toLower();
        if ( tag == QLatin1String( "code" ) )
            m_token = T_START_CODE;
        else if ( tag == QLatin1String( "list" ) )
            m_token = T_START_LIST;
        else if ( tag == QLatin1String( "quote" ) )
            m_token = T_START_QUOTE;
        else if ( tag == QLatin1String( "/code" ) )
            m_token = T_END_CODE;
        else if ( tag == QLatin1String( "/list" ) )
            m_token = T_END_LIST;
        else if ( tag == QLatin1String( "/quote" ) )
            m_token = T_END_QUOTE;
        else
            m_token = T_LINK;
    } else if ( m_rawValue.at( 0 ) == QLatin1Char( '`' ) ) {
        m_token = T_BACKTICK;
        m_value = m_rawValue.mid( 1, m_rawValue.length() - 2 );
    } else {
        m_token = T_NEWLINE;
    }
}

void MarkupProcessor::parse()
{
    while ( m_token != T_END )
        parseBlock();
}

void MarkupProcessor::parseBlock()
{
    switch ( m_token ) {
        case T_START_CODE:
            m_result += QLatin1String( "<pre class=\"code" );
            if ( !m_extra.isEmpty() ) {
                QString lang = m_extra.toLower();
                static const char* const langs[] = { "bash", "c", "c++", "c#", "css", "html", "java", "javascript", "js", "perl", "php", "python", "ruby", "sh", "sql", "vb", "xml" };
                for ( int i = 0; i < sizeof( langs ) / sizeof( langs[ 0 ] ); i++ ) {
                    if ( lang == QLatin1String( langs[ i ] ) ) {
                        lang = lang.replace( '+', 'p' ).replace( '#', 's' );
                        m_result += QLatin1String( " prettyprint lang-" );
                        m_result += lang;
                        break;
                    }
                }
            }
            m_result += QLatin1String( "\">" );
            next();
            parseCode();
            if ( m_token == T_END_CODE )
                next();
            m_result += QLatin1String( "</pre>" );
            break;

        case T_START_LIST:
            m_result += QLatin1String( "<ul><li>" );
            next();
            parseList();
            if ( m_token == T_END_LIST )
                next();
            m_result += QLatin1String( "</li></ul>" );
            break;

        case T_START_QUOTE:
            m_result += QLatin1String( "<div class=\"quote\">" );
            if ( !m_extra.isEmpty() ) {
                QString title = HtmlText::parse( m_extra, m_flags ).toString();
                if ( title.at( title.length() - 1 ) != QLatin1Char( ':' ) )
                    title += QLatin1Char( ':' );
                m_result += QString( "<div class=\"quote-title\">%1</div>" ).arg( title );
            }
            next();
            parseQuote();
            if ( m_token == T_END_QUOTE )
                next();
            m_result += QLatin1String( "</div>" );
            break;

        case T_TEXT:
        case T_BACKTICK:
        case T_LINK:
            parseText();
            break;

        case T_NEWLINE:
            m_result += QLatin1Char( '\n' );
            next();
            break;

        default:
            // ignore error (e.g. unbalanced closing tag)
            next();
            break;
    }
}

void MarkupProcessor::parseText()
{
    QStringList tags;

    QString text;
    int column = 0;

    for ( ; ; ) {
        switch ( m_token ) {
            case T_TEXT: {
                QRegExp subtokenExp( "\\*\\*+|__+" );
                int pos = 0;
                for ( ; ; ) {
                    int oldpos = pos;
                    pos = subtokenExp.indexIn( m_value, pos );

                    if ( pos < 0 ) {
                        text = HtmlText::convertTabsToSpaces( m_value.mid( oldpos ), column );
                        m_result += HtmlText::parse( text, m_flags ).toString();
                        break;
                    }

                    if ( pos > oldpos ) {
                        text = HtmlText::convertTabsToSpaces( m_value.mid( oldpos, pos - oldpos ), column );
                        m_result += HtmlText::parse( text, m_flags ).toString();
                    }

                    pos += subtokenExp.matchedLength();

                    QString subtoken = subtokenExp.cap( 0 );

                    if ( subtoken == QLatin1String( "**" ) || subtoken == QLatin1String( "__" ) ) {
                        QString tag = subtoken.at( 0 ) == QLatin1Char( '*' ) ? "strong" : "em";
                        int key = tags.indexOf( tag );
                        if ( key < 0 ) {
                            tags.append( tag );
                            m_result += QString( "<%1>" ).arg( tag );
                        } else {
                            for ( int i = tags.count() - 1; i >= key; i-- )
                                m_result += QString( "</%1>" ).arg( tags.at( i ) );
                            tags.removeAt( key );
                            for ( int i = key; i < tags.count(); i++ )
                                m_result += QString( "<%1>" ).arg( tags.at( i ) );
                        }
                        continue;
                    }

                    text = HtmlText::convertTabsToSpaces( subtoken, column );
                    m_result += Qt::escape( text );
                }
                next();
                break;
            }

            case T_BACKTICK:
                text = HtmlText::convertTabsToSpaces( m_value, column );
                m_result += QString( "<code>%1</code>" ).arg( Qt::escape( text ) );
                next();
                break;

            case T_LINK:
                text = HtmlText::convertTabsToSpaces( m_extra.isEmpty() ? m_value : m_extra, column );
                m_result += QString( "<a href=\"%1\">%2</a>" ).arg( Qt::escape( HtmlText::convertUrl( m_value, m_flags ) ), Qt::escape( text ) );
                next();
                break;

            default:
                for ( int i = tags.count() - 1; i >= 0; i-- )
                    m_result += QString( "</%1>" ).arg( tags.at( i ) );
                return;
        }
    }
}

void MarkupProcessor::parseCode()
{
    int nest = 1;
    int column = 0;

    while ( m_token != T_END ) {
        if ( m_token == T_START_CODE ) {
            nest++;
        } else if ( m_token == T_END_CODE ) {
            if ( --nest == 0 )
                break;
        }

        QString text = HtmlText::convertTabsToSpaces( m_rawValue, column );
        m_result += Qt::escape( text );
        next();
    }
}

void MarkupProcessor::parseList()
{
    int nest = 1;

    int level = itemLevel();
    if ( level > 1 ) {
        m_result += QString( "<ul><li>" ).repeated( level - 1 );
        nest = level;
    }

    while ( m_token != T_END && m_token != T_END_LIST ) {
        parseBlock();

        level = itemLevel();
        if ( level > nest ) {
            m_result += QString( "<ul><li>" ).repeated( level - nest );
            nest = level;
        } else if ( level > 0 ) {
            if ( level < nest )
                m_result += QString( "</li></ul>" ).repeated( nest - level );
            m_result += QLatin1String( "</li><li>" );
            nest = level;
        }
    }

    if ( nest > 1 )
        m_result += QString( "</li></ul>" ).repeated( nest - 1 );
}

int MarkupProcessor::itemLevel()
{
    if ( m_token == T_TEXT ) {
        QRegExp listExp( "[ \\t]*(\\*{1,6})[ \\t](.*)" );
        if ( listExp.exactMatch( m_value ) ) {
            m_value = listExp.cap( 2 );
            if ( m_value.isEmpty() )
                next();
            return listExp.cap( 1 ).length();
        }
    }
    return 0;
}

void MarkupProcessor::parseQuote()
{
    while ( m_token != T_END && m_token != T_END_QUOTE )
        parseBlock();
}
