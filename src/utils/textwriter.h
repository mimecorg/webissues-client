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

#ifndef ISSUEBROWSER_H
#define ISSUEBROWSER_H

#include <QTextCursor>

class TextWithLinks;

/**
* Class for writing formatted text to a QTextDocument.
*
* It supports multi-column layout, tables, lists, blocks and frames.
*/
class TextWriter
{
public:
    /**
    * Style of a layout cell.
    */
    enum CellStyle
    {
        /** Normal cell **/
        NormalCell,
        /** Cell used for comment text. */
        CommentCell,
        /** Cell used for file information. */
        FileCell,
        /** Cell used for right side links. */
        LinksCell
    };

    /**
    * Style of a block of text.
    */
    enum BlockStyle
    {
        /** Normal paragraph. */
        NormalBlock,
        /** Level 1 header. */
        Header1Block,
        /** Level 2 header. */
        Header2Block,
        /** Level 3 header. */
        Header3Block,
        /** Paragraph using smaller font. */
        SmallBlock,
        /** Right side links. */
        LinksBlock
    };

    /**
    * Flags affecting appearance of text.
    */
    enum Flag
    {
        /** Use smaller fonts for headers. */
        UseSmallHeaders = 1
    };

    Q_DECLARE_FLAGS( Flags, Flag );

public:
    /**
    * Constructor.
    * @param document The text document used by this class.
    * @param flags Optional flags affecting appearance of text.
    */
    TextWriter( QTextDocument* document, Flags flags = 0 );

    /**
    * Destructor.
    */
    ~TextWriter();

public:
    /**
    * Create a multi-column layout.
    * @param rows Number of rows in the layout.
    * @param columns Number of columns in the layout.
    */
    void createLayout( int rows, int columns );

    /**
    * Merge given cells of the current layout.
    * @param row Row index of the first cell to be merged.
    * @param column Column index of the first cell to be merged.
    * @param mergeRows Number of cells to merge horizontally.
    * @param mergeColumns Number of cells to merge vertically.
    */
    void mergeLayoutCells( int row, int column, int mergeRows, int mergeColumns );

    /**
    * Move the cursor to the given cell in the current layout.
    * @param row Row index of the cell.
    * @param column Column index of the cell.
    * @param style Style of the cell.
    */
    void gotoLayoutCell( int row, int column, CellStyle stlye );

    /**
    * Add more rows to the current layout.
    * @param count Number of rows to add.
    * @return Index of the first added row.
    */
    int appendLayoutRows( int count );

    /**
    * Move the cursor below the current layout.
    */
    void endLayout();

    /**
    * Write a single block of text using the given style.
    * @param text Text with links to write.
    * @param style Style of the block.
    * @param anchor Optional name of the anchor associated with this block.
    */
    void writeBlock( const TextWithLinks& text, BlockStyle style, const QString& anchor = QString() );

    /**
    * Write a bulletted list.
    * @param items Items of the list.
    */
    void writeBulletList( const QList<TextWithLinks>& items );

    /**
    * Write a header-value list.
    * @param headers Headers of the list.
    * @param values Values displayed in the list.
    */
    void writeInfoList( const QStringList& headers, const QList<TextWithLinks>& values );

    /**
    * Create a data table with given headers.
    * @param headers List of table column headers.
    */
    void createTable( const QStringList& headers );

    /**
    * Append a row of data to the current table.
    * @param cells List of row cells.
    */
    void appendTableRow( const QList<TextWithLinks>& cells );

    /**
    * Move the cursor below the current table.
    */
    void endTable();

private:
    QTextDocument* m_document;
    Flags m_flags;

    QTextTable* m_layout;
    QTextTable* m_table;

    QTextCursor m_cursor;
};

#endif // ISSUEBROWSER_H
