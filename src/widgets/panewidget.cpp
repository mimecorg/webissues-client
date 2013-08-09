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

#include "panewidget.h"

#include <QLabel>
#include <QLayout>

PaneWidget::PaneWidget( QWidget* parent ) : QStackedWidget( parent )
{
    m_labelWidget = new QWidget( this );
    m_label = new QLabel( m_labelWidget );

    QVBoxLayout* layout = new QVBoxLayout( m_labelWidget );
    layout->setContentsMargins( 0, 0, 0, 0 );
    layout->addWidget( m_label );

    m_label->setAlignment( Qt::AlignCenter );
    m_label->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );
    m_label->setForegroundRole( QPalette::Shadow );
    m_label->setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
#if defined( Q_WS_WIN )
    m_label->setAutoFillBackground( true );
#endif

    addWidget( m_labelWidget );
}

PaneWidget::~PaneWidget()
{
}

void PaneWidget::setPlaceholderText( const QString& text )
{
    m_label->setText( text );
}

void PaneWidget::showPlaceholder()
{
    setCurrentWidget( m_labelWidget );
}
