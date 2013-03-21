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

#include "markupprocessor.h"

#include <QRegExp>

enum MarkupMode
{
    NormalMode,
    ListMode,
    CodeMode,
    QuoteMode
};

struct MarkupState
{
    MarkupState( MarkupMode mode = NormalMode, int nest = 0, bool start = false ) :
        m_mode( mode ),
        m_nest( nest ),
        m_start( start )
    {
    }

    MarkupMode m_mode;
    int m_nest;
    bool m_start;
};

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

static QString parseText( const QString& text, HtmlText::Flags flags, MarkupState& state )
{
    if ( text.isEmpty() )
        return QString();

    // ignore any formatting in a code block
    if ( state.m_mode == CodeMode )
        return Qt::escape( text );

    QString result;
    int pos = 0;

    // handle initial asterisks in a list block
    if ( state.m_mode == ListMode ) {
        QRegExp listExp( "[ \\t]*(\\*{1,6})[ \\t](.*)" );

        if ( listExp.exactMatch( text ) ) {
            int nest = listExp.cap( 1 ).length();
            pos = listExp.pos( 2 );

            if ( nest > state.m_nest ) {
                result += QString( "<ul><li>" ).repeated( nest - state.m_nest );
            } else {
                if ( state.m_nest > nest )
                    result += QString( "</li></ul>" ).repeated( state.m_nest - nest );
                if ( !state.m_start )
                    result += "</li><li>";
            }

            state.m_nest = nest;
        }
    }

    if ( state.m_mode == ListMode )
        state.m_start = false;

    if ( pos < 0 )
        return result;

    QStack<QString> tags;

    QRegExp subtokenExp( "\\*\\*+|__+|`[^`]+`|\\["
        "(?:(?:mailto:)?[\\w.%+-]+@[\\w.-]+\\.[a-z]{2,4}"
        "|(?:(?:https?|ftp|file):\\/\\/|www\\.|ftp\\.|\\\\\\\\)[\\w+&@#\\/\\\\%=~|$?!:,.()-]+"
        "|#\\d+)(?:[ \\t][^]]*)?\\]", Qt::CaseInsensitive );
    
    for ( ; ; ) {
        int oldpos = pos;
        pos = subtokenExp.indexIn( text, pos );

        if ( pos < 0 ) {
            result += HtmlText::parse( text.mid( oldpos ), flags ).toString();
            break;
        }

        if ( pos > oldpos )
            result += HtmlText::parse( text.mid( oldpos, pos - oldpos ), flags ).toString();

        pos += subtokenExp.matchedLength();

        QString subtoken = subtokenExp.cap( 0 );

        if ( subtoken == QLatin1String( "**" ) || subtoken == QLatin1String( "__" ) ) {
            QString tag = subtoken.at( 0 ) == QLatin1Char( '*' ) ? "strong" : "em";

            // find a matching opening tag
            int key = tags.indexOf( tag );
            if ( key < 0 ) {
                tags.push( tag );
                result += QString( "<%1>" ).arg( tag );
            } else {
                while ( tags.count() > key ) {
                    tag = tags.pop();
                    result += QString( "</%1>" ).arg( tag );
                }
            }
            continue;
        }

        if ( subtoken.at( 0 ) == QLatin1Char( '`' ) ) {
            // display monotype text without further processing
            result += QString( "<code>%1</code>" ).arg( Qt::escape( subtoken.mid( 1, subtoken.length() - 2 ) ) );
            continue;
        }

        if ( subtoken.at( 0 ) == QLatin1Char( '[' ) ) {
            int index = strcspn( subtoken );
            QString url = subtoken.mid( 1, index - 1 );
            QString title = subtoken.mid( index, subtoken.lastIndexOf( ']' ) - index ).trimmed();

            if ( title.isEmpty() )
                title = url;

            if ( url.at( 0 ) == QLatin1Char( '#' ) )
                url = ( ( flags & HtmlText::NoInternalLinks ) ? "#id" : "id://" ) + url.mid( 1 );
            else if ( url.startsWith( QLatin1String( "www." ), Qt::CaseInsensitive ) )
                url = "http://" + url;
            else if ( url.startsWith( QLatin1String( "ftp." ), Qt::CaseInsensitive ) )
                url = "ftp://" + url;
            else if ( url.startsWith( QLatin1String( "\\\\" ) ) )
                url = "file:///" + url;
            else if ( !url.contains( QLatin1Char( ':' ) ) )
                url = "mailto:" + url;

            result += QString( "<a href=\"%1\">%2</a>" ).arg( Qt::escape( url ), Qt::escape( title ) );
            continue;
        }

        result += Qt::escape( subtoken );
    }

    // pop the remaining inline tags from the stack
    while ( !tags.isEmpty() ) {
        QString tag = tags.pop();
        result += QString( "</%1>" ).arg( tag );
    }

    return result;
}

HtmlText MarkupProcessor::parse( const QString& text, HtmlText::Flags flags /* = 0 */ )
{
    MarkupState state( NormalMode );
    QStack<MarkupState> stack;

    QRegExp tokenExp( "\\n|\\[\\/?(?:list|code|quote)(?:[ \\t][^]\\n]*)?\\](?:[ \\t]*\\n)?", Qt::CaseInsensitive );

    QString result;

    int pos = 0;
    for ( ; ; ) {
        int oldpos = pos;
        pos = tokenExp.indexIn( text, pos );

        if ( pos < 0 ) {
            result += parseText( text.mid( oldpos ), flags, state );
            break;
        }

        if ( pos > oldpos )
            result += parseText( text.mid( oldpos, pos - oldpos ), flags, state );

        pos += tokenExp.matchedLength();

        QString token = tokenExp.cap( 0 );

        if ( token.at( 0 ) == QLatin1Char( '[' ) ) {
            int index = strcspn( token );
            QString tag = token.mid( 1, index - 1 ).toLower();
            QString extra = token.mid( index, token.lastIndexOf( ']' ) - index ).trimmed();

            // ignore all block tags in a code block, but count nested [code] and [/code] tags
            if ( state.m_mode == CodeMode ) {
                if ( tag == QLatin1String( "code" ) ) {
                    state.m_nest++;
                } else if ( tag == QLatin1String( "/code" ) ) {
                    if ( --state.m_nest == 0 ) {
                        result += QLatin1String( "</pre>" );
                        state = stack.pop();
                        continue;
                    }
                }
                result += Qt::escape( token );
                continue;
            }

            if ( state.m_mode == ListMode )
                state.m_start = false;

            if ( tag == QLatin1String( "/list" ) || tag == QLatin1String( "/quote" ) ) {
                // find a matching opening tag
                MarkupMode mode = tag.at( 1 ) == QLatin1Char( 'l' ) ? ListMode : QuoteMode;
                int pop = 0;
                if ( mode == state.m_mode ) {
                    pop = 1;
                } else {
                    for ( int i = stack.count() - 1; i > 0; i-- ) {
                        if ( mode == stack.at( i ).m_mode ) {
                            pop = stack.count() - i + 1;
                            break;
                        }
                    }
                }
                if ( pop > 0 ) {
                    // pop the block tags from the stack
                    for ( int i = 0; i < pop; i++ ) {
                        if ( state.m_mode == ListMode )
                            result += QString( "</li></ul>" ).repeated( state.m_nest );
                        else if ( state.m_mode == CodeMode )
                            result += QLatin1String( "</pre>" );
                        else
                            result += QLatin1String( "</div>" );
                        state = stack.pop();
                    }
                    continue;
                }
                // fall through if not matching opening tag found; it will be emitted as-is
            }

            if ( tag == QLatin1String( "list" ) ) {
                stack.push( state );
                state = MarkupState( ListMode, 1, true );
                result += QLatin1String( "<ul><li>" );
                continue;
            }

            if ( tag == QLatin1String( "code" ) ) {
                stack.push( state );
                state = MarkupState( CodeMode, 1 );
                QString classes;
                if ( !extra.isEmpty() ) {
                    // enable pretty printing if a valid language is given
                    QString lang = extra.toLower();
                    static const char* const langs[] = { "bash", "c", "c++", "c#", "css", "html", "java", "javascript", "js", "perl", "php", "python", "ruby", "sh", "sql", "vb", "xml" };
                    for ( int i = 0; i < sizeof( langs ) / sizeof( langs[ 0 ] ); i++ ) {
                        if ( lang == QLatin1String( langs[ i ] ) ) {
                            lang = lang.replace( '+', 'p' ).replace( '#', 's' );
                            classes = " prettyprint lang-" + lang;
                            break;
                        }
                    }
                }
                result += QString( "<pre class=\"code%1\">" ).arg( classes );
                continue;
            }

            if ( tag == QLatin1String( "quote" ) ) {
                stack.push( state );
                state = MarkupState( QuoteMode );
                result += QLatin1String( "<div class=\"quote\">" );
                if ( !extra.isEmpty() ) {
                    QString title = HtmlText::parse( extra, flags ).toString();
                    if ( title.at( title.length() - 1 ) != QLatin1Char( ':' ) )
                        title += QLatin1Char( ':' );
                    result += QString( "<div class=\"quote-title\">%1</div>" ).arg( title );
                }
                continue;
            }
        }

        result += Qt::escape( token );
    }

    // pop the remaining block tags from the stack
    while ( !stack.isEmpty() ) {
        if ( state.m_mode == ListMode )
            result += QString( "</li></ul>" ).repeated( state.m_nest );
        else if ( state.m_mode == CodeMode )
            result += QLatin1String( "</pre>" );
        else
            result += QLatin1String( "</div>" );
        state = stack.pop();
    }

    HtmlText htmlText( flags );
    htmlText.m_html = result;

    return htmlText;
}
