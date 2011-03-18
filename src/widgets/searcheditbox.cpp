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

#include "searcheditbox.h"
#include "edittoolbutton.h"

#include "utils/iconloader.h"

#include <QStyle>
#include <QStyleOption>
#include <QPainter>
#include <QKeyEvent>

SearchEditBox::SearchEditBox( QWidget* parent ) : QLineEdit( parent ),
    m_showPrompt( false )
{
    m_optionsButton = new EditToolButton( this );
    m_optionsButton->setIcon( IconLoader::icon( "find-options" ) );
    m_optionsButton->setToolTip( tr( "Search Options" ) );
    m_optionsButton->setPopupMode( QToolButton::InstantPopup );

    m_clearButton = new EditToolButton( this );
    m_clearButton->setIcon( IconLoader::icon( "find-clear" ) );
    m_clearButton->setToolTip( tr( "Clear" ) );
    m_clearButton->hide();
  
    connect( m_clearButton, SIGNAL( clicked() ), this, SLOT( clear() ) );
    connect( this, SIGNAL( textChanged( const QString& ) ), this, SLOT( updateCloseButton( const QString& ) ) );

    QSize buttonSize = m_optionsButton->sizeHint();
    int frameWidth = style()->pixelMetric( QStyle::PM_DefaultFrameWidth );

    int padding = buttonSize.width() + frameWidth + 1;
#if QT_VERSION >= 0x040500
    setTextMargins( padding, 0, padding, 0 );
#else
    setStyleSheet( QString( "QLineEdit { padding-left: %1px; padding-right: %1px; } " ).arg( padding ) );
#endif

    setMinimumHeight( qMax( minimumSizeHint().height(), buttonSize.height() + 2 * frameWidth ) );

    showPrompt( true );
}

SearchEditBox::~SearchEditBox()
{
}

void SearchEditBox::resizeEvent( QResizeEvent* e )
{
    QLineEdit::resizeEvent( e );

    QSize buttonSize = m_optionsButton->sizeHint();
    int frameWidth = style()->pixelMetric( QStyle::PM_DefaultFrameWidth );

    int y = ( rect().bottom() + 1 - buttonSize.height() ) / 2;

    m_optionsButton->move( frameWidth, y );
    m_clearButton->move( rect().right() - frameWidth - buttonSize.width(), y );
}

void SearchEditBox::updateCloseButton( const QString& text )
{
    m_clearButton->setVisible( !text.isEmpty() );
}

void SearchEditBox::focusInEvent( QFocusEvent* e )
{
    QLineEdit::focusInEvent( e );

    if ( m_showPrompt )
        showPrompt( false );
}

void SearchEditBox::focusOutEvent( QFocusEvent* e )
{
    QLineEdit::focusOutEvent( e );

    if ( text().isEmpty() )
        showPrompt( true );
}

void SearchEditBox::setPromptText( const QString& text )
{
    m_promptText = text;

    if ( m_showPrompt )
        update();
}

void SearchEditBox::showPrompt( bool on )
{
    m_showPrompt = on;

    update();
}

void SearchEditBox::clear()
{
    m_showPrompt = true;

    QLineEdit::clear();
}

void SearchEditBox::paintEvent( QPaintEvent* e )
{
    QLineEdit::paintEvent( e );

    if ( m_showPrompt && !m_promptText.isEmpty() ) {
        QPainter painter( this );

        painter.setPen( palette().color( QPalette::Disabled, QPalette::Text ) );

        int frameWidth = style()->pixelMetric( QStyle::PM_DefaultFrameWidth );

        QRect textRect = rect().adjusted( 2 * frameWidth + 20, frameWidth, -frameWidth, -frameWidth );
        painter.setClipRect( textRect );

        int x = textRect.x();
        int y = textRect.y() + ( textRect.height() - fontMetrics().height() + 1 ) / 2;

        painter.drawText( x, y + fontMetrics().ascent(), m_promptText );
    }
}

void SearchEditBox::setOptionsMenu( QMenu* menu )
{
    m_optionsButton->setMenu( menu );
}

void SearchEditBox::keyPressEvent( QKeyEvent * e )
{
    if ( e->key() == Qt::Key_Escape ) {
        clear();
        emit deactivate();
    } else if ( e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter ) {
        emit deactivate();
    } else if ( e->key() == Qt::Key_F4 ) {
        m_optionsButton->showMenu();
    } else {
        QLineEdit::keyPressEvent( e );
    }
}
