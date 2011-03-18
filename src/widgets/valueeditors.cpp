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

#include "valueeditors.h"
#include "inputlineedit.h"
#include "inputtextedit.h"

#include "data/datamanager.h"
#include "rdb/utilities.h"
#include "utils/definitioninfo.h"
#include "utils/datetimehelper.h"
#include "utils/attributehelper.h"

#include <QVariant>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QDateEdit>
#include <QDateTimeEdit>
#include <QCalendarWidget>

// fix for GCC 4.3-snapshot
#include <climits>

InputLineValueEditor::InputLineValueEditor( const DefinitionInfo& info, bool initial, QObject* parent, QWidget* parentWidget ) : AbstractValueEditor( parent )
{
    switch ( AttributeHelper::toAttributeType( info ) ) {
        case TextAttribute:
            initializeText( info, initial, parentWidget );
            break;
        case EnumAttribute:
            initializeEnum( info, initial, parentWidget );
            break;
        case NumericAttribute:
            initializeNumeric( info, initial, parentWidget );
            break;
        case DateTimeAttribute:
            initializeDateTime( info, initial, parentWidget );
            break;
        case UserAttribute:
            initializeUser( info, initial, parentWidget );
            break;
        default:
            break;
    }
}

InputLineValueEditor::~InputLineValueEditor()
{
}

void InputLineValueEditor::initializeText( const DefinitionInfo& info, bool initial, QWidget* parentWidget )
{
    InputLineEdit* edit = new InputLineEdit( parentWidget );

    QVariant maxLength = info.metadata( "max-length" );
    if ( maxLength.isValid() )
        edit->setMaxLength( maxLength.toInt() );
    else
        edit->setMaxLength( 255 );

    QVariant minLength = info.metadata( "min-length" );
    if ( minLength.isValid() )
        edit->setMinLength( minLength.toInt() );

    edit->setRequired( info.metadata( "required" ).toBool() );

    if ( initial )
        edit->setFunctions( InputLineEdit::MeFunction );

    setWidget( edit );
}

void InputLineValueEditor::initializeEnum( const DefinitionInfo& info, bool initial, QWidget* parentWidget )
{
    EnumLineEdit* edit = new EnumLineEdit( parentWidget );

    QVariant items = info.metadata( "items" );
    edit->setItems( items.toStringList() );

    QVariant maxLength = info.metadata( "max-length" );
    if ( maxLength.isValid() )
        edit->setMaxLength( maxLength.toInt() );
    else
        edit->setMaxLength( 255 );

    QVariant minLength = info.metadata( "min-length" );
    if ( minLength.isValid() )
        edit->setMinLength( minLength.toInt() );

    edit->setEditable( info.metadata( "editable" ).toBool() );

    edit->setRequired( info.metadata( "required" ).toBool() );

    if ( initial )
        edit->setFunctions( InputLineEdit::MeFunction );

    setWidget( edit );
}

void InputLineValueEditor::initializeNumeric( const DefinitionInfo& info, bool /*initial*/, QWidget* parentWidget )
{
    NumericLineEdit* edit = new NumericLineEdit( parentWidget );

    edit->setDecimal( info.metadata( "decimal" ).toInt() );
    edit->setStripZeros( info.metadata( "strip" ).toBool() );

    QVariant minValue = info.metadata( "min-value" );
    if ( minValue.isValid() )
        edit->setMinValue( minValue.toDouble() );

    QVariant maxValue = info.metadata( "max-value" );
    if ( maxValue.isValid() )
        edit->setMaxValue( maxValue.toDouble() );

    edit->setRequired( info.metadata( "required" ).toBool() );

    setWidget( edit );
}

void InputLineValueEditor::initializeDateTime( const DefinitionInfo& info, bool initial, QWidget* parentWidget )
{
    DateTimeLineEdit* edit = new DateTimeLineEdit( parentWidget );

    edit->setWithTime( info.metadata( "time" ).toBool() );
    edit->setLocalTime( info.metadata( "local" ).toBool() );

    edit->setRequired( info.metadata( "required" ).toBool() );

    if ( initial )
        edit->setFunctions( DateTimeLineEdit::TodayFunction );

    setWidget( edit );
}

void InputLineValueEditor::initializeUser( const DefinitionInfo& info, bool initial, QWidget* parentWidget )
{
    EnumLineEdit* edit = new EnumLineEdit( parentWidget );

    QStringList items;

    if ( initial )
        items.append( QString( "[%1]" ).arg( tr( "Me" ) ) );

    RDB::IndexConstIterator<UserRow> it( dataManager->users()->index() );
    QList<const UserRow*> users = localeAwareSortRows( it, &UserRow::name );

    for ( int i = 0; i < users.count(); i++ ) {
        const UserRow* user = users.at( i );
        if ( user->access() == NoAccess )
            continue;
        items.append( user->name() );
    }

    edit->setItems( items );
    edit->setEditable( false );

    edit->setRequired( info.metadata( "required" ).toBool() );

    if ( initial )
        edit->setFunctions( InputLineEdit::MeFunction );

    setWidget( edit );
}

void InputLineValueEditor::setInputValue( const QString& value )
{
    InputLineEdit* edit = (InputLineEdit*)widget();
    edit->setInputValue( value );
}

QString InputLineValueEditor::inputValue()
{
    InputLineEdit* edit = (InputLineEdit*)widget();
    return edit->inputValue();
}

InputTextValueEditor::InputTextValueEditor( const DefinitionInfo& info, QObject* parent, QWidget* parentWidget ) : AbstractValueEditor( parent )
{
    InputTextEdit* edit = new InputTextEdit( parentWidget );

    QVariant maxLength = info.metadata( "max-length" );
    if ( maxLength.isValid() )
        edit->setMaxLength( maxLength.toInt() );
    else
        edit->setMaxLength( 255 );

    QVariant minLength = info.metadata( "min-length" );
    if ( minLength.isValid() )
        edit->setMinLength( minLength.toInt() );

    edit->setRequired( info.metadata( "required" ).toBool() );

    setWidget( edit );
}

InputTextValueEditor::~InputTextValueEditor()
{
}

void InputTextValueEditor::setInputValue( const QString& value )
{
    InputTextEdit* edit = (InputTextEdit*)widget();
    edit->setInputValue( value );
}

QString InputTextValueEditor::inputValue()
{
    InputTextEdit* edit = (InputTextEdit*)widget();
    return edit->inputValue();
}

ComboBoxValueEditor::ComboBoxValueEditor( const DefinitionInfo& info, int projectId, QObject* parent, QWidget* parentWidget ) : AbstractValueEditor( parent )
{
    QComboBox* combo = new QComboBox( parentWidget );

    if ( !info.metadata( "required" ).toBool() )
        combo->addItem( QString() );

    AttributeType type = AttributeHelper::toAttributeType( info );

    if ( type == EnumAttribute ) {
        QVariant items = info.metadata( "items" );
        combo->addItems( items.toStringList() );
    } else if ( type == UserAttribute ) {
        bool members = info.metadata( "members" ).toBool() && projectId != 0;

        RDB::IndexConstIterator<UserRow> it( dataManager->users()->index() );
        QList<const UserRow*> users = localeAwareSortRows( it, &UserRow::name );

        for ( int i = 0; i < users.count(); i++ ) {
            const UserRow* user = users.at( i );
            if ( user->access() == NoAccess )
                continue;
            if ( members && !dataManager->members()->find( user->userId(), projectId ) )
                continue;
            combo->addItem( user->name() );
        }
    }

    setWidget( combo );
}

ComboBoxValueEditor::~ComboBoxValueEditor()
{
}

void ComboBoxValueEditor::setInputValue( const QString& value )
{
    QComboBox* combo = (QComboBox*)widget();

    int index = combo->findText( value );
    if ( index >= 0 )
        combo->setCurrentIndex( index );
}

QString ComboBoxValueEditor::inputValue()
{
    QComboBox* combo = (QComboBox*)widget();
    return combo->currentText();
}
