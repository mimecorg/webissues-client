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

#include "statusbar.h"
#include "elidedlabel.h"

#include "utils/iconloader.h"

#include <QLabel>
#include <QMessageBox>
#include <QAccessible>

StatusBar::StatusBar( QWidget* parent ) : QStatusBar( parent ),
    m_summaryPixmap( NULL ),
    m_summaryLabel( NULL )
{
    setStyleSheet( "QStatusBar::item { border-style: none }" );

    m_pixmap = new QLabel( this );
    m_label = new ElidedLabel( this );
    m_summaryPixmap = new QLabel( this );
    m_summaryLabel = new QLabel( this );

    m_pixmap->setFixedSize( 16, 16 );
    m_summaryPixmap->setFixedSize( 16, 16 );

    addWidget( m_pixmap, 0 );
    addWidget( m_label, 1 );
    addWidget( m_summaryPixmap, 0 );
    addWidget( m_summaryLabel, 0 );
}

StatusBar::~StatusBar()
{
}

void StatusBar::showInfo( const QString& text )
{
    showStatus( IconLoader::pixmap( "status-info" ), text );
}

void StatusBar::showWarning( const QString& text )
{
    showStatus( IconLoader::pixmap( "status-warning" ), text, QMessageBox::Warning );
}

void StatusBar::showError( const QString& text )
{
    showStatus( IconLoader::pixmap( "status-error" ), text, QMessageBox::Critical );
}

void StatusBar::showBusy( const QString& text )
{
    showStatus( IconLoader::pixmap( "status-busy" ), text );
}

void StatusBar::showStatus( const QPixmap& pixmap, const QString& text, int icon /*= 0*/ )
{
    m_pixmap->setPixmap( pixmap );
    m_label->setText( text );

    if ( icon != 0 && topLevelWidget()->isActiveWindow() ) {
        QMessageBox box;
        box.setIcon( (QMessageBox::Icon)icon );
        QAccessible::updateAccessibility( &box, 0, QAccessible::Alert );
    }
}

void StatusBar::showSummary( const QPixmap& pixmap, const QString& text )
{
    if ( !pixmap.isNull() ) {
        m_summaryPixmap->setPixmap( pixmap );
        m_summaryPixmap->show();
    } else {
        m_summaryPixmap->hide();
    }

    if ( !text.isEmpty() ) {
        m_summaryLabel->setText( text );
        m_summaryLabel->show();
    } else {
        m_summaryLabel->hide();
    }
}
