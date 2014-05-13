/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2014 WebIssues Team
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

#ifndef HTMLWRITER_H
#define HTMLWRITER_H

#include <QString>
#include <QStack>

class HtmlText;

/**
* Class for writing documents in HTML format.
*/
class HtmlWriter
{
public:
    enum Pane {
        TopPane,
        BottomPane
    };

    enum BlockStyle {
        Header2Block,
        Header3Block,
        Header4Block,
        FloatBlock,
        HistoryInfoBlock,
        CommentBlock,
        AttachmentBlock,
        NoItemsBlock
    };

public:
    /**
    * Constructor.
    */
    HtmlWriter();

    /**
    * Destructor.
    */
    ~HtmlWriter();

public:
    void setTitle( const QString& title );
    const QString title() const { return m_title; }

    void setEmbedded( bool on );
    bool isEmbedded() const { return m_embedded; }

    void createLayout();
    void appendLayoutRow();
    void beginCell( Pane pane, int mergeColumns = 1 );
    void endLayout();

    void beginHistoryItem();
    void endHistoryItem();

    void writeBlock( const HtmlText& text, BlockStyle style );

    void writeBulletList( const QList<HtmlText>& items );

    void writeInfoList( const QStringList& headers, const QList<HtmlText>& values, bool multiLine );

    void createTable( const QStringList& headers );
    void appendTableRow( const QList<HtmlText>& cells );
    void endTable();

    /**
    * Return the resulting HTML.
    */
    QString toHtml();

private:
    void pushTag( const QString& tag, const QString& attributes = QString() );
    void popTag( const QString& tag );
    void popAll();
    
private:
    QString m_title;
    bool m_embedded;

    QString m_body;

    QStack<QString> m_tags;
};

#endif
