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

#include "checkmessagebox.h"

#include <QLayout>
#include <QLabel>
#include <QCheckBox>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QAccessible>

CheckMessageBox::CheckMessageBox( QWidget* parent ) : QDialog( parent ),
    m_icon( QMessageBox::NoIcon )
{
    QVBoxLayout* mainLayout = new QVBoxLayout( this );
    mainLayout->setSpacing( 15 );

    QHBoxLayout* iconLayout = new QHBoxLayout();

    m_pixmap = new QLabel( this );
    iconLayout->addWidget( m_pixmap );
    iconLayout->setAlignment( m_pixmap, Qt::AlignTop );

    QVBoxLayout* checkLayout = new QVBoxLayout();

    m_prompt = new QLabel( this );
    checkLayout->addWidget( m_prompt );

    m_checkBox = new QCheckBox( tr( "&Do not show this message again" ), this );
    checkLayout->addWidget( m_checkBox );

    iconLayout->addLayout( checkLayout );
    mainLayout->addLayout( iconLayout );

    m_buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok, Qt::Horizontal, this );
    m_buttonBox->setCenterButtons( true );
    mainLayout->addWidget( m_buttonBox );

    connect( m_buttonBox, SIGNAL( clicked( QAbstractButton* ) ), this, SLOT( buttonClicked( QAbstractButton* ) ) );
}

CheckMessageBox::~CheckMessageBox()
{
}

void CheckMessageBox::setText( const QString& text )
{
    m_prompt->setText( text );
}

void CheckMessageBox::setIcon( QMessageBox::Icon icon )
{
    m_icon = icon;
    m_pixmap->setPixmap( QMessageBox::standardIcon( icon ) );
}

void CheckMessageBox::setCheckBoxText( const QString& text )
{
    m_checkBox->setText( text );
}

void CheckMessageBox::setChecked( bool checked )
{
    m_checkBox->setChecked( checked );
}

bool CheckMessageBox::isChecked()
{
    return m_checkBox->isChecked();
}

void CheckMessageBox::setStandardButtons( QMessageBox::StandardButtons buttons )
{
    m_buttonBox->setStandardButtons( (QDialogButtonBox::StandardButtons)(int)buttons );
}

QPushButton* CheckMessageBox::button( QMessageBox::StandardButton button )
{
    return m_buttonBox->button( (QDialogButtonBox::StandardButton)button );
}

int CheckMessageBox::exec()
{
    setMaximumSize( sizeHint() );

    // hack emulating the message box sound effect
    QMessageBox box;
    box.setIcon( m_icon );
    QAccessible::updateAccessibility( &box, 0, QAccessible::Alert );

    return QDialog::exec();
}

void CheckMessageBox::reject()
{
    if ( m_buttonBox->buttons().count() == 1 )
        done ( m_buttonBox->standardButton( m_buttonBox->buttons().first() ) );
    else if ( m_buttonBox->button( QDialogButtonBox::Cancel ) )
        done( QDialogButtonBox::Cancel );
}

void CheckMessageBox::buttonClicked( QAbstractButton* button )
{
    done( m_buttonBox->standardButton( button ) );
}
