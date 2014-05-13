/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2014 WebIssues Team
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

#include "metadataeditors.h"

#include "widgets/inputlineedit.h"
#include "widgets/inputtextedit.h"

#include <QLayout>
#include <QRadioButton>
#include <QButtonGroup>
#include <QLabel>
#include <QGroupBox>
#include <QCheckBox>
#include <QSpinBox>

TextMetadataEditor::TextMetadataEditor( QObject* parent, QWidget* parentWidget ) : AbstractMetadataEditor( parent )
{
    QWidget* widget = new QWidget( parentWidget );

    QGridLayout* layout = new QGridLayout( widget );
    layout->setMargin( 0 );

    m_multiLineCheckBox = new QCheckBox( tr( "&Allow entering multiple lines of text" ), widget );
    layout->addWidget( m_multiLineCheckBox, 0, 0, 1, 2 );

    QFrame* separator = new QFrame( widget );
    separator->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    layout->addWidget( separator, 1, 0, 1, 2 );

    QLabel* minLengthLabel = new QLabel( tr( "Mi&nimum length:" ), widget );
    layout->addWidget( minLengthLabel, 2, 0 );

    m_minLengthEdit = new NumericLineEdit( widget );
    m_minLengthEdit->setMinValue( 1 );
    m_minLengthEdit->setMaxValue( 255 );
    layout->addWidget( m_minLengthEdit, 2, 1 );

    minLengthLabel->setBuddy( m_minLengthEdit );

    QLabel* maxLengthLabel = new QLabel( tr( "Ma&ximum length:" ), widget );
    layout->addWidget( maxLengthLabel, 3, 0 );

    m_maxLengthEdit = new NumericLineEdit( widget );
    m_maxLengthEdit->setMinValue( 1 );
    m_maxLengthEdit->setMaxValue( 255 );
    layout->addWidget( m_maxLengthEdit, 3, 1 );

    maxLengthLabel->setBuddy( m_maxLengthEdit );

    setWidget( widget );
    setFixedHeight( true );
}

TextMetadataEditor::~TextMetadataEditor()
{
}

void TextMetadataEditor::setMetadata( const QVariantMap& metadata )
{
    m_multiLineCheckBox->setChecked( metadata.value( "multi-line" ).toBool() );
    m_minLengthEdit->setInputValue( metadata.value( "min-length" ).toString() );
    m_maxLengthEdit->setInputValue( metadata.value( "max-length" ).toString() );
}

void TextMetadataEditor::updateMetadata( QVariantMap& metadata )
{
    metadata.insert( "multi-line", m_multiLineCheckBox->isChecked() );

    if ( !m_minLengthEdit->inputValue().isEmpty() )
        metadata.insert( "min-length", m_minLengthEdit->inputValue().toInt() );
    else
        metadata.remove( "min-length" );

    if ( !m_maxLengthEdit->inputValue().isEmpty() )
        metadata.insert( "max-length", m_maxLengthEdit->inputValue().toInt() );
    else
        metadata.remove( "max-length" );
}

EnumMetadataEditor::EnumMetadataEditor( QObject* parent, QWidget* parentWidget ) : AbstractMetadataEditor( parent )
{
    QWidget* widget = new QWidget( parentWidget );

    QGridLayout* layout = new QGridLayout( widget );
    layout->setMargin( 0 );

    m_editableCheckBox = new QCheckBox( tr( "Allow &entering custom values" ), widget );
    layout->addWidget( m_editableCheckBox, 0, 0, 1, 2 );

    connect( m_editableCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( checkBoxToggled() ) );

    m_multiSelectCheckBox = new QCheckBox( tr( "Allow &selecting multiple items" ), widget );
    layout->addWidget( m_multiSelectCheckBox, 1, 0, 1, 2 );

    connect( m_multiSelectCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( checkBoxToggled() ) );

    QFrame* separator = new QFrame( widget );
    separator->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    layout->addWidget( separator, 2, 0, 1, 2 );

    QLabel* itemsLabel = new QLabel( tr( "&Dropdown list items (enter one item per line):" ), widget );
    layout->addWidget( itemsLabel, 3, 0, 1, 2 );

    m_itemsEdit = new InputTextEdit( widget );
    m_itemsEdit->setRequired( true );
    m_itemsEdit->setFormat( InputTextEdit::ItemsFormat );
    layout->addWidget( m_itemsEdit, 4, 0, 1, 2 );

    itemsLabel->setBuddy( m_itemsEdit );

    QFrame* separator2 = new QFrame( widget );
    separator2->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    layout->addWidget( separator2, 5, 0, 1, 2 );

    QLabel* limitsLabel = new QLabel( tr( "Limits can only be used when entering custom values is allowed and selecting multiple items is not allowed." ), widget );
    limitsLabel->setWordWrap( true );
    layout->addWidget( limitsLabel, 6, 0, 1, 2 );

    QLabel* minLengthLabel = new QLabel( tr( "Mi&nimum length:" ), widget );
    layout->addWidget( minLengthLabel, 7, 0 );

    m_minLengthEdit = new NumericLineEdit( widget );
    m_minLengthEdit->setMinValue( 1 );
    m_minLengthEdit->setMaxValue( 255 );
    layout->addWidget( m_minLengthEdit, 7, 1 );

    minLengthLabel->setBuddy( m_minLengthEdit );

    QLabel* maxLengthLabel = new QLabel( tr( "Ma&ximum length:" ), widget );
    layout->addWidget( maxLengthLabel, 8, 0 );

    m_maxLengthEdit = new NumericLineEdit( widget );
    m_maxLengthEdit->setMinValue( 1 );
    m_maxLengthEdit->setMaxValue( 255 );
    layout->addWidget( m_maxLengthEdit, 8, 1 );

    maxLengthLabel->setBuddy( m_maxLengthEdit );

    setWidget( widget );
    setFixedHeight( false );

    checkBoxToggled();
}

EnumMetadataEditor::~EnumMetadataEditor()
{
}

void EnumMetadataEditor::setMetadata( const QVariantMap& metadata )
{
    m_editableCheckBox->setChecked( metadata.value( "editable" ).toBool() );
    m_multiSelectCheckBox->setChecked( metadata.value( "multi-select" ).toBool() );
    m_itemsEdit->setInputValue( metadata.value( "items" ).toStringList().join( "\n" ) );
    m_minLengthEdit->setInputValue( metadata.value( "min-length" ).toString() );
    m_maxLengthEdit->setInputValue( metadata.value( "max-length" ).toString() );
}

void EnumMetadataEditor::updateMetadata( QVariantMap& metadata )
{
    metadata.insert( "editable", m_editableCheckBox->isChecked() );
    metadata.insert( "multi-select", m_multiSelectCheckBox->isChecked() );
    metadata.insert( "items", m_itemsEdit->inputValue().split( "\n" ) );

    if ( m_editableCheckBox->isChecked() && !m_multiSelectCheckBox->isChecked() ) {
        if ( !m_minLengthEdit->inputValue().isEmpty() )
            metadata.insert( "min-length", m_minLengthEdit->inputValue().toInt() );
        else
            metadata.remove( "min-length" );

        if ( !m_maxLengthEdit->inputValue().isEmpty() )
            metadata.insert( "max-length", m_maxLengthEdit->inputValue().toInt() );
        else
            metadata.remove( "max-length" );
    }
}

void EnumMetadataEditor::checkBoxToggled()
{
    bool limits = ( m_editableCheckBox->isChecked() && !m_multiSelectCheckBox->isChecked() );
    m_minLengthEdit->setEnabled( limits );
    m_maxLengthEdit->setEnabled( limits );
}

NumericMetadataEditor::NumericMetadataEditor( QObject* parent, QWidget* parentWidget ) : AbstractMetadataEditor( parent )
{
    QWidget* widget = new QWidget( parentWidget );

    QGridLayout* layout = new QGridLayout( widget );
    layout->setMargin( 0 );

    QLabel* decimalLabel = new QLabel( tr( "&Decimal places:" ), widget );
    layout->addWidget( decimalLabel, 0, 0 );

    m_decimalSpinBox = new QSpinBox( widget );
    m_decimalSpinBox->setRange( 0, 6 );
    layout->addWidget( m_decimalSpinBox, 0, 1 );

    decimalLabel->setBuddy( m_decimalSpinBox );

    connect( m_decimalSpinBox, SIGNAL( valueChanged( int ) ), this, SLOT( decimalValueChanged( int ) ) );

    m_stripCheckBox = new QCheckBox( tr( "&Strip trailing decimal zeros" ), widget );
    layout->addWidget( m_stripCheckBox, 1, 0, 1, 2 );

    connect( m_stripCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( stripToggled( bool ) ) );

    QFrame* separator = new QFrame( widget );
    separator->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    layout->addWidget( separator, 2, 0, 1, 2 );

    QLabel* minValueLabel = new QLabel( tr( "Mi&nimum value:" ), widget );
    layout->addWidget( minValueLabel, 3, 0 );

    m_minValueEdit = new NumericLineEdit( widget );
    layout->addWidget( m_minValueEdit, 3, 1 );

    minValueLabel->setBuddy( m_minValueEdit );

    QLabel* maxValueLabel = new QLabel( tr( "Ma&ximum value:" ), widget );
    layout->addWidget( maxValueLabel, 4, 0 );

    m_maxValueEdit = new NumericLineEdit( widget );
    layout->addWidget( m_maxValueEdit, 4, 1 );

    maxValueLabel->setBuddy( m_maxValueEdit );

    setWidget( widget );
    setFixedHeight( true );
}

NumericMetadataEditor::~NumericMetadataEditor()
{
}

void NumericMetadataEditor::decimalValueChanged( int value )
{
    m_minValueEdit->setDecimal( value );
    m_maxValueEdit->setDecimal( value );
}

void NumericMetadataEditor::stripToggled( bool checked )
{
    m_minValueEdit->setStripZeros( checked );
    m_maxValueEdit->setStripZeros( checked );
}

void NumericMetadataEditor::setMetadata( const QVariantMap& metadata )
{
    m_decimalSpinBox->setValue( metadata.value( "decimal", 0 ).toInt() );
    m_stripCheckBox->setChecked( metadata.value( "strip" ).toBool() );
    m_minValueEdit->setInputValue( metadata.value( "min-value" ).toString() );
    m_maxValueEdit->setInputValue( metadata.value( "max-value" ).toString() );
}

void NumericMetadataEditor::updateMetadata( QVariantMap& metadata )
{
    metadata.insert( "decimal", m_decimalSpinBox->value() );
    metadata.insert( "strip", m_stripCheckBox->isChecked() );

    if ( !m_minValueEdit->inputValue().isEmpty() )
        metadata.insert( "min-value", m_minValueEdit->inputValue() );
    else
        metadata.remove( "min-value" );

    if ( !m_maxValueEdit->inputValue().isEmpty() )
        metadata.insert( "max-value", m_maxValueEdit->inputValue() );
    else
        metadata.remove( "max-value" );
}

DateTimeMetadataEditor::DateTimeMetadataEditor( QObject* parent, QWidget* parentWidget ) : AbstractMetadataEditor( parent )
{
    QWidget* widget = new QWidget( parentWidget );

    QVBoxLayout* layout = new QVBoxLayout( widget );
    layout->setMargin( 0 );

    QRadioButton* dateOnlyRadio = new QRadioButton( tr( "&Date only" ), widget );
    layout->addWidget( dateOnlyRadio );
    QRadioButton* dateTimeRadio = new QRadioButton( tr( "Date and &time without time zone conversion" ), widget );
    layout->addWidget( dateTimeRadio );
    QRadioButton* localTimeRadio = new QRadioButton( tr( "Date and time &using local time zone" ), widget );
    layout->addWidget( localTimeRadio );

    m_timeGroup = new QButtonGroup( this );
    m_timeGroup->addButton( dateOnlyRadio, 0 );
    m_timeGroup->addButton( dateTimeRadio, 1 );
    m_timeGroup->addButton( localTimeRadio, 2 );

    setWidget( widget );
    setFixedHeight( true );
}

DateTimeMetadataEditor::~DateTimeMetadataEditor()
{
}

void DateTimeMetadataEditor::setMetadata( const QVariantMap& metadata )
{
    if ( metadata.value( "time" ).toBool() ) {
        if ( metadata.value( "local" ).toBool() )
            m_timeGroup->button( 2 )->setChecked( true );
        else
            m_timeGroup->button( 1 )->setChecked( true );
    } else {
        m_timeGroup->button( 0 )->setChecked( true );
    }
}

void DateTimeMetadataEditor::updateMetadata( QVariantMap& metadata )
{
    metadata.insert( "time", m_timeGroup->checkedId() > 0 );
    metadata.insert( "local", m_timeGroup->checkedId() == 2 );
}

UserMetadataEditor::UserMetadataEditor( QObject* parent, QWidget* parentWidget ) : AbstractMetadataEditor( parent )
{
    QWidget* widget = new QWidget( parentWidget );

    QVBoxLayout* layout = new QVBoxLayout( widget );
    layout->setMargin( 0 );

    m_membersCheckBox = new QCheckBox( tr( "&Allow selecting only project members" ), widget );
    layout->addWidget( m_membersCheckBox );
    m_multiSelectCheckBox = new QCheckBox( tr( "Allow &selecting multiple items" ), widget );
    layout->addWidget( m_multiSelectCheckBox );

    setWidget( widget );
    setFixedHeight( true );
}

UserMetadataEditor::~UserMetadataEditor()
{
}

void UserMetadataEditor::setMetadata( const QVariantMap& metadata )
{
    m_membersCheckBox->setChecked( metadata.value( "members" ).toBool() );
    m_multiSelectCheckBox->setChecked( metadata.value( "multi-select" ).toBool() );
}

void UserMetadataEditor::updateMetadata( QVariantMap& metadata )
{
    metadata.insert( "members", m_membersCheckBox->isChecked() );
    metadata.insert( "multi-select", m_multiSelectCheckBox->isChecked() );
}
