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

#ifndef HTMLTEXT_H 
#define HTMLTEXT_H 

#include <QString>

/**
* Class representing a fragment of HTML text containing links and images.
*
* Text can be parsed to automatically convert URLs and e-mail addresses
* to hyperlinks. Item identifiers prefixed with a # character are converted
* to links using the <tt>id://{number}</tt> URL.
*/
class HtmlText
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
    HtmlText( Flags flags = 0 );

    /**
    * Conversion constructor.
    * @param text Plain text to convert.
    */
    HtmlText( const QString& text );

    /**
    * Destructor.
    */
    ~HtmlText();

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
    * Create an image.
    * @param image Name of the image icon.
    * @param text Text of the alt attribute.
    */
    void appendImage( const QString& image, const QString& text );

    /**
    * Create an image and text label.
    * @param image Name of the image icon.
    * @param text Text of the label.
    */
    void appendImageAndText( const QString& image, const QString& text );

    /**
    * Create an image and text link.
    * @param image Name of the image icon.
    * @param text Text of the link.
    * @param url Destination URL of the link.
    */
    void appendImageAndTextLink( const QString& image, const QString& text, const QString& url );

    /**
    * Start an anchor.
    * @param name Name of the anchor.
    */
    void createAnchor( const QString& name );

    /**
    * End the anchor.
    */
    void endAnchor();

    /**
    * Return the entire text as HTML.
    */
    QString toString() const { return m_html; }

    /**
    * Extract links from the given fragment of text.
    * @param text Text containing URLs and item references.
    * @param flags Flags affecting extracting of links.
    */
    static HtmlText parse( const QString& text, Flags flags = 0 );

    /**
    * Convert tabs to spaces.
    */
    static QString convertTabsToSpaces( const QString& text );

private:
    static QString convertUrl( const QString& url, Flags flags );

    static QString convertTabsToSpaces( const QString& text, int& column );

private:
    Flags m_flags;
    QString m_html;

    friend class MarkupProcessor;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( HtmlText::Flags )

#endif
