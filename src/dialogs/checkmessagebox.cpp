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

#include "checkmessagebox.h"

#include <QCheckBox>
#include <QVBoxLayout>

CheckMessageBox::CheckMessageBox( QWidget* parent ) : MessageBox( parent )
{
    QVBoxLayout* checkLayout = new QVBoxLayout();

    m_checkBox = new QCheckBox( tr( "&Do not show this message again" ), this );
    checkLayout->addWidget( m_checkBox );

    setContentLayout( checkLayout, true );
}

CheckMessageBox::~CheckMessageBox()
{
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
