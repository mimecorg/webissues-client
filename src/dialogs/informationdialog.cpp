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

#include "informationdialog.h"

#include "utils/iconloader.h"
#include "xmlui/gradientwidget.h"

#include <QLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>

InformationDialog::InformationDialog( QWidget* parent ) : QDialog( parent ),
    m_promptPixmap( NULL )
{
    QVBoxLayout* topLayout = new QVBoxLayout( this );
    topLayout->setMargin( 0 );
    topLayout->setSpacing( 0 );

    m_promptWidget = new XmlUi::GradientWidget( this );
    topLayout->addWidget( m_promptWidget );

    m_promptLayout = new QHBoxLayout( m_promptWidget );
    m_promptLayout->setSpacing( 10 );

    m_promptLabel = new QLabel( m_promptWidget );
    m_promptLabel->setWordWrap( true );
    m_promptLayout->addWidget( m_promptLabel, 1 );

    QFrame* separator = new QFrame( this );
    separator->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    topLayout->addWidget( separator );

    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setMargin( 9 );
    m_mainLayout->setSpacing( 6 );
    topLayout->addLayout( m_mainLayout );

    m_mainLayout->addSpacing( 5 );

    m_buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok, Qt::Horizontal, this );
    m_mainLayout->addWidget( m_buttonBox );

    m_buttonBox->button( QDialogButtonBox::Ok )->setText( tr( "&OK" ) );

    connect( m_buttonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
}

InformationDialog::~InformationDialog()
{
}

void InformationDialog::setPrompt( const QString& text )
{
    m_promptLabel->setText( text );

    m_promptLabel->setMinimumWidth( 350 );
    m_promptLabel->setFixedHeight( m_promptLabel->heightForWidth( 350 ) );
}

void InformationDialog::setPromptPixmap( const QPixmap& pixmap )
{
    if ( !m_promptPixmap ) {
        m_promptPixmap = new QLabel( m_promptWidget );
        m_promptLayout->insertWidget( 0, m_promptPixmap, 0 );
    }

    m_promptPixmap->setPixmap( pixmap );
}

void InformationDialog::setContentLayout( QLayout* layout, bool fixed )
{
    if ( layout )
        m_mainLayout->insertLayout( 0, layout );

    if ( fixed )
        setMaximumHeight( sizeHint().height() );
}
