/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2015 WebIssues Team
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

#include "linklabel.h"

#include <QEvent>
#include <QTextDocument>

LinkLabel::LinkLabel( const QString& caption, QWidget* parent ) : QLabel( parent ),
    m_linkUrl( "#" ),
    m_linkCaption( caption )
{
    setTextFormat( Qt::RichText );
    updateLink();
}

LinkLabel::~LinkLabel()
{
}

void LinkLabel::setLinkUrl( const QString& url )
{
    m_linkUrl = url;
    updateLink();
}

void LinkLabel::setLinkCaption( const QString& caption )
{
    m_linkCaption = caption;
    updateLink();
}

void LinkLabel::changeEvent( QEvent* e )
{
    QLabel::changeEvent( e );

    if ( e->type() == QEvent::EnabledChange || e->type() == QEvent::PaletteChange )
        updateLink();
}

void LinkLabel::updateLink()
{
    QString style;
    if ( !isEnabled() ) {
        QColor color = palette().color( QPalette::Disabled, QPalette::WindowText );
        style = QString( " style=\"color: %1; text-decoration: none;\"" ).arg( color.name() );
    }
    setText( QString( "<a href=\"%1\"%2>%3</a>" ).arg( m_linkUrl.toHtmlEscaped(), style, m_linkCaption.toHtmlEscaped() ) );
}
