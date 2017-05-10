/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2017 WebIssues Team
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

#include "edittoolbutton.h"

#include <QStylePainter>
#include <QStyleOption>

EditToolButton::EditToolButton( QWidget* parent ) : QToolButton( parent )
{
    setCursor( Qt::ArrowCursor );
    setFocusPolicy( Qt::NoFocus );
    setIconSize( QSize( 16, 16 ) );
}

EditToolButton::~EditToolButton()
{
}

QSize EditToolButton::sizeHint() const
{
    return iconSize() + QSize( 2, 2 );
}

void EditToolButton::paintEvent( QPaintEvent* /*e*/ )
{
    QStylePainter painter( this );

    QStyleOptionToolButton option;
    initStyleOption( &option );

    painter.drawControl( QStyle::CE_ToolButtonLabel, option );

    if ( option.features & QStyleOptionToolButton::HasMenu ) {
        option.rect.setLeft( option.rect.right() - 5 );
        option.rect.setTop( option.rect.bottom() - 5 );
        painter.drawPrimitive( QStyle::PE_IndicatorArrowDown, option );
    }
}
