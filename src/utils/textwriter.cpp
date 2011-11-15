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

#include "textwriter.h"

#include "application.h"
#include "data/localsettings.h"
#include "utils/textwithlinks.h"

#include <QTextDocument>
#include <QTextTable>

TextWriter::TextWriter( QTextDocument* document, Flags flags /*= 0*/ ) :
    m_document( document ),
    m_flags( flags ),
    m_layout( NULL ),
    m_table( NULL )
{
    m_document->clear();
    m_document->setUndoRedoEnabled( false );

    LocalSettings* settings = application->applicationSettings();
    QString family = settings->value( "ReportFont" ).toString();
    int size = settings->value( "ReportFontSize" ).toInt();

    QFont font( family, size );
    font.setStyleHint( QFont::SansSerif );

    m_document->setDefaultFont( font );

    QTextFrame* rootFrame = m_document->rootFrame();
    m_cursor = rootFrame->firstCursorPosition();

    QTextFrameFormat frameFormat = rootFrame->frameFormat();
    frameFormat.setPadding( 6 );
    rootFrame->setFrameFormat( frameFormat );

    m_document->setIndentWidth( 25 );
}

TextWriter::~TextWriter()
{
}

void TextWriter::createLayout( int rows, int columns )
{
    QTextTableFormat tableFormat;
    tableFormat.setAlignment( Qt::AlignTop );
    tableFormat.setCellPadding( 0 );
    tableFormat.setCellSpacing( 0 );
    tableFormat.setBorder( 0 );
    tableFormat.setTopMargin( 10 );
    tableFormat.setBottomMargin( 10 );
    tableFormat.setWidth( QTextLength( QTextLength::PercentageLength, 100 ) );

    QVector<QTextLength> constraints;
    for ( int i = 0; i < columns; i++ )
        constraints.append( QTextLength( QTextLength::PercentageLength, 100.0 / columns ) );
    tableFormat.setColumnWidthConstraints( constraints );

    m_layout = m_cursor.insertTable( rows, columns, tableFormat );
}

void TextWriter::mergeLayoutCells( int row, int column, int mergeRows, int mergeColumns )
{
    m_layout->mergeCells( row, column, mergeRows, mergeColumns );
}

void TextWriter::gotoLayoutCell( int row, int column, CellStyle style )
{
    QTextTableCell cell = m_layout->cellAt( row, column );
    m_cursor = cell.firstCursorPosition();

    QTextTableCellFormat cellFormat = cell.format().toTableCellFormat();

    switch ( style ) {
        case CommentCell:
            cellFormat.setBackground( QColor( 0xf6, 0xf6, 0xf6 ) );
            cellFormat.setPadding( 3 );
            break;
        case FileCell:
            cellFormat.setBackground( QColor( 0xee, 0xee, 0xff ) );
            cellFormat.setPadding( 3 );
            break;
        default:
            if ( row > 0 )
                cellFormat.setTopPadding( 10 );
            cellFormat.setBottomPadding( 5 );
            if ( column > 0 )
                cellFormat.setLeftPadding( 7 );
            if ( column < m_layout->columns() - cell.columnSpan() )
                cellFormat.setRightPadding( 7 );
            break;
    }

    if ( style == LinksCell )
        cellFormat.setVerticalAlignment( QTextCharFormat::AlignBottom );

    cell.setFormat( cellFormat );
}

int TextWriter::appendLayoutRows( int count )
{
    int index = m_layout->rows();
    m_layout->insertRows( index, count );
    return index;
}

void TextWriter::endLayout()
{
    m_cursor = m_document->rootFrame()->lastCursorPosition();
    m_layout = NULL;
}

void TextWriter::writeBlock( const TextWithLinks& text, BlockStyle style, const QString& anchor /*= QString()*/ )
{
    m_cursor.beginEditBlock();

    QTextBlockFormat blockFormat;
    switch ( style ) {
        case Header1Block:
        case Header2Block:
            blockFormat.setTopMargin( 10 );
            blockFormat.setBottomMargin( 10 );
            break;
        case Header3Block:
        case SmallBlock:
            blockFormat.setTopMargin( 5 );
            blockFormat.setBottomMargin( 5 );
            break;
        default:
            break;
    }

    if ( style == LinksBlock )
        blockFormat.setAlignment( Qt::AlignRight );

    QTextCharFormat charFormat;
    switch ( style ) {
        case Header1Block:
            charFormat.setFontPointSize( m_flags & UseSmallHeaders ? 9 : 11 );
            charFormat.setFontWeight( QFont::Bold );
            break;
        case Header2Block:
            charFormat.setFontPointSize( m_flags & UseSmallHeaders ? 8 : 9 );
            charFormat.setFontWeight( QFont::Bold );
            break;
        case Header3Block:
            charFormat.setFontPointSize( 8 );
            charFormat.setFontWeight( QFont::Bold );
            break;
        case SmallBlock:
            charFormat.setFontPointSize( 7.5 );
            charFormat.setForeground( QColor( 0x66, 0x66, 0x66 ) );
            break;
        default:
            break;
    }

    if ( !anchor.isEmpty() ) {
        charFormat.setAnchor( true );
        charFormat.setAnchorName( anchor );
    }

    if ( m_cursor.atBlockStart() )
        m_cursor.setBlockFormat( blockFormat );
    else
        m_cursor.insertBlock( blockFormat );

    text.write( m_cursor, charFormat );

    m_cursor.endEditBlock();
}

void TextWriter::writeBulletList( const QList<TextWithLinks>& items )
{
    m_cursor.beginEditBlock();

    QTextListFormat listFormat;
    listFormat.setStyle( QTextListFormat::ListCircle );

    if ( m_cursor.atBlockStart() )
        m_cursor.createList( listFormat );
    else
        m_cursor.insertList( listFormat );

    QTextBlockFormat itemFormat = m_cursor.blockFormat();

    for ( int i = 0; i < items.count(); i++ ) {
        if ( i > 0 )
            m_cursor.insertBlock();
        itemFormat.setTopMargin( i == 0 ? 10 : 3 );
        itemFormat.setBottomMargin( i == items.count() - 1 ? 10 : 0 );
        m_cursor.setBlockFormat( itemFormat );
        items.at( i ).write( m_cursor );
    }

    m_cursor.endEditBlock();

    if ( m_layout )
        m_cursor = m_layout->cellAt( m_cursor ).lastCursorPosition();
    else
        m_cursor = m_document->rootFrame()->lastCursorPosition();
}

void TextWriter::writeInfoList( const QStringList& headers, const QList<TextWithLinks>& values )
{
    m_cursor.beginEditBlock();

    QTextTableFormat tableFormat;
    tableFormat.setAlignment( Qt::AlignTop );
    tableFormat.setCellPadding( 0 );
    tableFormat.setCellSpacing( 0 );
    tableFormat.setBorder( 0 );

    QTextTable* table = m_cursor.insertTable( headers.count(), 2, tableFormat );

    for ( int i = 0; i < headers.count(); i++ ) {
        QTextTableCell headerCell = table->cellAt( i, 0 );
        m_cursor = headerCell.firstCursorPosition();

        QTextTableCellFormat headerCellFormat = headerCell.format().toTableCellFormat();
        headerCellFormat.setTopPadding( 5 );
        headerCell.setFormat( headerCellFormat );

        m_cursor.insertText( headers.at( i ) );

        QTextTableCell valueCell = table->cellAt( i, 1 );
        m_cursor = valueCell.firstCursorPosition();

        QTextTableCellFormat valueCellFormat = valueCell.format().toTableCellFormat();
        valueCellFormat.setTopPadding( 5 );
        valueCellFormat.setLeftPadding( 15 );
        valueCell.setFormat( valueCellFormat );

        values.at( i ).write( m_cursor );
    }

    m_cursor.endEditBlock();

    if ( m_layout )
        m_cursor = m_layout->cellAt( m_cursor ).lastCursorPosition();
    else
        m_cursor = m_document->rootFrame()->lastCursorPosition();
}

void TextWriter::createTable( const QStringList& headers )
{
    QTextTableFormat tableFormat;
    tableFormat.setAlignment( Qt::AlignTop );
    tableFormat.setCellPadding( 5 );
    tableFormat.setCellSpacing( 0 );
    tableFormat.setBorder( 0.5 );
    tableFormat.setBorderStyle( QTextTableFormat::BorderStyle_Solid );
    tableFormat.setBorderBrush( QColor( 0, 0, 0 ) );
    tableFormat.setTopMargin( 10 );
    tableFormat.setBottomMargin( 10 );
    tableFormat.setWidth( QTextLength( QTextLength::PercentageLength, 100 ) );
    tableFormat.setHeaderRowCount( 1 );

    m_table = m_cursor.insertTable( 1, headers.count(), tableFormat );

    for ( int i = 0; i < headers.count(); i++ ) {
        QTextTableCell cell = m_table->cellAt( 0, i );
        m_cursor = cell.firstCursorPosition();

        QTextTableCellFormat cellFormat = cell.format().toTableCellFormat();
        cellFormat.setFontWeight( QFont::Bold );
        cell.setFormat( cellFormat );

        m_cursor.insertText( headers.at( i ) );
    }
}

void TextWriter::appendTableRow( const QList<TextWithLinks>& cells )
{
    m_table->insertRows( m_table->rows(), 1 );

    for ( int i = 0; i < cells.count(); i++ ) {
        QTextTableCell cell = m_table->cellAt( m_table->rows() - 1, i );
        m_cursor = cell.firstCursorPosition();

        cells.at( i ).write( m_cursor );
    }
}

void TextWriter::endTable()
{
    m_cursor = m_document->rootFrame()->lastCursorPosition();
    m_table = NULL;
}
