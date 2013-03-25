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

#include "markuptextedit.h"

#include "data/datamanager.h"
#include "widgets/inputtextedit.h"

#include <QLayout>
#include <QComboBox>

MarkupTextEdit::MarkupTextEdit( QWidget* parent ) : QWidget( parent )
{
    QVBoxLayout* layout = new QVBoxLayout( this );
    layout->setMargin( 0 );

    m_edit = new InputTextEdit( this );
    layout->addWidget( m_edit, 1 );

    m_edit->setRequired( true );
    m_edit->setMaxLength( dataManager->setting( "comment_max_length" ).toInt() );

    QHBoxLayout* formatLayout = new QHBoxLayout();
    layout->addLayout( formatLayout );

    QLabel* label = new QLabel( tr( "&Text format:" ), this );
    formatLayout->addWidget( label );

    m_comboBox = new QComboBox( this );
    formatLayout->addWidget( m_comboBox );

    m_comboBox->addItem( tr( "Plain text" ) );
    m_comboBox->addItem( tr( "Text with markup" ) );

    label->setBuddy( m_comboBox );
    
    formatLayout->addStretch( 1 );
}

MarkupTextEdit::~MarkupTextEdit()
{
}

void MarkupTextEdit::setInputValue( const QString& value )
{
    m_edit->setInputValue( value );
}

QString MarkupTextEdit::inputValue()
{
    return m_edit->inputValue();
}

bool MarkupTextEdit::validate()
{
    return m_edit->validate();
}

void MarkupTextEdit::setRequired( bool required )
{
    m_edit->setRequired( required );
}

bool MarkupTextEdit::isRequired() const
{
    return m_edit->isRequired();
}

void MarkupTextEdit::setTextFormat( TextFormat format )
{
    m_comboBox->setCurrentIndex( (int)format );
}

TextFormat MarkupTextEdit::textFormat() const
{
    return (TextFormat)m_comboBox->currentIndex();
}
