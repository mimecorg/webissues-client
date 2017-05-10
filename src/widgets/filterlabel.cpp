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

#include "filterlabel.h"

#include <QUrl>

FilterLabel::FilterLabel( QWidget* parent ) : QLabel( parent ),
    m_currentIndex( 0 )
{
    setTextFormat( Qt::RichText );
    setAlignment( Qt::AlignVCenter | Qt::AlignRight );
    setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Preferred );

    connect( this, SIGNAL( linkActivated( QString ) ), this, SLOT( itemActivated( QString ) ) );
}

FilterLabel::~FilterLabel()
{
}

void FilterLabel::addItem( const QString& item )
{
    m_items.append( item );
    updateText();
}

void FilterLabel::setCurrentIndex( int index )
{
    if ( m_currentIndex != index ) {
        m_currentIndex = index;
        updateText();
        emit currentIndexChanged( m_currentIndex );
    }
}

void FilterLabel::itemActivated( const QString& link )
{
    QUrl url( link );
    if ( url.scheme() == QLatin1String( "link" ) )
        setCurrentIndex( url.path().toInt() );
}

void FilterLabel::updateText()
{
    QString text;

    for ( int i = 0; i < m_items.count(); i++ ) {
        if ( i > 0 )
            text += QLatin1String( " | " );
        if ( i == m_currentIndex )
            text += m_items.at( i );
        else
            text += QString( "<a href=\"link:%1\">%2</a>" ).arg( i ).arg( m_items.at( i ) );
    }

    setText( text );
}
