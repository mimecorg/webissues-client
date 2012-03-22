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

#include "statuslabel.h"
#include "elidedlabel.h"

#include <QLayout>

StatusLabel::StatusLabel( QWidget* parent ) : QWidget( parent )
{
    QHBoxLayout* layout = new QHBoxLayout( this );
    layout->setMargin( 0 );
    layout->setSpacing( 6 );

    m_pixmapLabel = new QLabel( this );
    m_pixmapLabel->setFixedSize( 16, 16 );
    m_pixmapLabel->setVisible( false );
    layout->addWidget( m_pixmapLabel );

    m_label = new ElidedLabel( this );
    layout->addWidget( m_label, 1 );

    layout->addSpacing( 6 );
}

StatusLabel::~StatusLabel()
{
}

void StatusLabel::setText( const QString& text )
{
    m_label->setText( text );
}

QString StatusLabel::text() const
{
    return m_label->text();
}

void StatusLabel::setPixmap( const QPixmap& pixmap )
{
    m_pixmapLabel->setPixmap( pixmap );
    m_pixmapLabel->setVisible( !pixmap.isNull() );
}

const QPixmap* StatusLabel::pixmap() const
{
    return m_pixmapLabel->pixmap();
}
