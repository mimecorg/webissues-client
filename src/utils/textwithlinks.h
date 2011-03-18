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

#ifndef TEXTWITHLINKS_H 
#define TEXTWITHLINKS_H 

#include <QStringList>
#include <QTextCursor>

/**
* Class representing a fragment of text containing hyperlinks.
*
* Text can be parsed to automatically convert URLs and e-mail addresses
* to hyperlinks. Item identifiers prefixed with a # character are converted
* to links using the <tt>id://{number}</tt> URL.
*/
class TextWithLinks
{
public:
    /**
    * Flags affecting extracting of links.
    */
    enum Flag
    {
        /** Use anchors instead of internal id:// links. */
        NoInternalLinks = 1
    };

    Q_DECLARE_FLAGS( Flags, Flag );

public:
    /**
    * Default constructor.
    * @param flags Flags affecting extracting of links.
    */
    TextWithLinks( Flags flags = 0 );

    /**
    * Conversion constructor.
    * @param text Plain text to convert.
    */
    TextWithLinks( const QString& text );

    /**
    * Destructor.
    */
    ~TextWithLinks();

public:
    /**
    * Clear the text.
    */
    void clear();

    /**
    * Append a fragment of plain text.
    * @param text Plain text to append.
    */
    void appendText( const QString& text );

    /**
    * Append a hyperlink.
    * @param text Text of the link.
    * @param url Destination URL of the link.
    */
    void appendLink( const QString& text, const QString& url );

    /**
    * Append a fragment of text and automatically exctract links.
    * @param text Text containing URLs and item references.
    */
    void appendParsed( const QString& text );

    /**
    * Write the entire text at the cursor position.
    */
    void write( QTextCursor& cursor, const QTextCharFormat& format = QTextCharFormat() ) const;

    /**
    * Extract links from the given fragment of text.
    * @param text Text containing URLs and item references.
    * @param flags Flags affecting extracting of links.
    */
    static TextWithLinks parse( const QString& text, Flags flags = 0 );

private:
    Flags m_flags;
    QStringList m_texts;
    QStringList m_urls;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( TextWithLinks::Flags )

#endif
