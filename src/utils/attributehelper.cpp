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

#include "attributehelper.h"

#include "data/datamanager.h"
#include "utils/definitioninfo.h"
#include "utils/formatter.h"
#include "utils/datetimehelper.h"

#include <QStringList>

AttributeHelper::AttributeHelper()
{
}

AttributeHelper::~AttributeHelper()
{
}

AttributeType AttributeHelper::toAttributeType( const DefinitionInfo& info )
{
    if ( info.type() == QLatin1String( "TEXT" ) )
        return TextAttribute;
    if ( info.type() == QLatin1String( "ENUM" ) )
        return EnumAttribute;
    if ( info.type() == QLatin1String( "NUMERIC" ) )
        return NumericAttribute;
    if ( info.type() == QLatin1String( "DATETIME" ) )
        return DateTimeAttribute;
    if ( info.type() == QLatin1String( "USER" ) )
        return UserAttribute;
    return InvalidAttribute;
}

DefinitionInfo AttributeHelper::fromAttributeType( AttributeType type )
{
    DefinitionInfo info;
    switch ( type ) {
        case TextAttribute:
            info.setType( "TEXT" );
            break;
        case EnumAttribute:
            info.setType( "ENUM" );
            break;
        case NumericAttribute:
            info.setType( "NUMERIC" );
            break;
        case DateTimeAttribute:
            info.setType( "DATETIME" );
            break;
        case UserAttribute:
            info.setType( "USER" );
            break;
        default:
            break;
    }
    return info;
}

QString AttributeHelper::convertInitialValue( const DefinitionInfo& info, const QString& value ) const
{
    AttributeType type = toAttributeType( info );

    if ( ( type == TextAttribute || type == EnumAttribute || type == UserAttribute ) && value.startsWith( QLatin1String( "[Me]" ) ) )
        return dataManager->currentUserName();

    if ( type == DateTimeAttribute && value.startsWith( QLatin1String( "[Today]" ) ) ) {
        QDateTime date;
        if ( info.metadata( "local" ).toBool() )
            date = QDateTime::currentDateTime().toUTC();
        else
            date = QDateTime::currentDateTime();

        QString offset = value.mid( 7 );
        if ( !offset.isEmpty() )
            date = date.addDays( offset.toInt() );

        if ( info.metadata( "time" ).toBool() )
            return DateTimeHelper::formatDateTime( date );
        else
            return DateTimeHelper::formatDate( date.date() );
    }

    return value;
}

QString AttributeHelper::formatExpression( const DefinitionInfo& info, const QString& value ) const
{
    AttributeType type = toAttributeType( info );

    if ( ( type == TextAttribute || type == EnumAttribute || type == UserAttribute ) && value.startsWith( QLatin1String( "[Me]" ) ) )
        return QString( "[%1]" ).arg( tr( "Me" ) );

    if ( type == DateTimeAttribute && value.startsWith( QLatin1String( "[Today]" ) ) )
        return QString( "[%1]%2" ).arg( tr( "Today" ), value.mid( 7 ) );

    Formatter formatter;
    return formatter.convertAttributeValue( info, value, false );
}

QString AttributeHelper::typeName( AttributeType type ) const
{
    switch ( type ) {
        case TextAttribute:
            return tr( "Text" );
        case EnumAttribute:
            return tr( "Dropdown list" );
        case NumericAttribute:
            return tr( "Numeric" );
        case DateTimeAttribute:
            return tr( "Date & time" );
        case UserAttribute:
            return tr( "User" );
        default:
            return QString();
    }
}

QString AttributeHelper::metadataDetails( const DefinitionInfo& info ) const
{
    switch ( toAttributeType( info ) ) {
        case TextAttribute:
            return textMetadataDetails( info );
        case EnumAttribute:
            return enumMetadataDetails( info );
        case NumericAttribute:
            return numericMetadataDetails( info );
        case DateTimeAttribute:
            return dateTimeMetadataDetails( info );
        case UserAttribute:
            return userMetadataDetails( info );
        default:
            return QString();
    }
}

QString AttributeHelper::textMetadataDetails( const DefinitionInfo& info ) const
{
    QStringList details;

    bool multiLine = info.metadata( "multi-line" ).toBool();
    if ( multiLine )
        details.append( tr( "Multiple lines" ) );

    QVariant minLength = info.metadata( "min-length" );
    if ( minLength.isValid() )
        details.append( tr( "Min. length: %1" ).arg( minLength.toString() ) );

    QVariant maxLength = info.metadata( "max-length" );
    if ( maxLength.isValid() )
        details.append( tr( "Max. length: %1" ).arg( maxLength.toString() ) );

    return details.join( "; " );
}

QString AttributeHelper::enumMetadataDetails( const DefinitionInfo& info ) const
{
    QStringList details;

    bool editable = info.metadata( "editable" ).toBool();
    if ( editable )
        details.append( tr( "Editable" ) );

    bool multiSelect = info.metadata( "multi-select" ).toBool();
    if ( multiSelect )
        details.append( tr( "Multiple selection" ) );

    QStringList items = info.metadata( "items" ).toStringList();
    details.append( tr( "Items: %1" ).arg( items.join( ", " ) ) );

    QVariant minLength = info.metadata( "min-length" );
    if ( minLength.isValid() )
        details.append( tr( "Min. length: %1" ).arg( minLength.toString() ) );

    QVariant maxLength = info.metadata( "max-length" );
    if ( maxLength.isValid() )
        details.append( tr( "Max. length: %1" ).arg( maxLength.toString() ) );

    return details.join( "; " );
}

QString AttributeHelper::numericMetadataDetails( const DefinitionInfo& info ) const
{
    QStringList details;

    int decimal = info.metadata( "decimal" ).toInt();
    if ( decimal > 0 )
        details.append( tr( "Decimal places: %1" ).arg( decimal ) );

    bool strip = info.metadata( "strip" ).toBool();

    Formatter formatter;

    QVariant minValue = info.metadata( "min-value" );
    if ( minValue.isValid() )
        details.append( tr( "Min. value: %1" ).arg( formatter.convertNumber( minValue.toString(), decimal, strip ) ) );

    QVariant maxValue = info.metadata( "max-value" );
    if ( maxValue.isValid() )
        details.append( tr( "Max. value: %1" ).arg( formatter.convertNumber( maxValue.toString(), decimal, strip ) ) );

    if ( strip )
        details.append( tr( "Strip zeros" ) );

    return details.join( "; " );
}

QString AttributeHelper::dateTimeMetadataDetails( const DefinitionInfo& info ) const
{
    QStringList details;

    bool time = info.metadata( "time" ).toBool();
    if ( time )
        details.append( tr( "With time" ) );

    bool local = info.metadata( "local" ).toBool();
    if ( local )
        details.append( tr( "Local time zone" ) );

    return details.join( "; " );
}

QString AttributeHelper::userMetadataDetails( const DefinitionInfo& info ) const
{
    QStringList details;

    bool members = info.metadata( "members" ).toBool();
    if ( members )
        details.append( tr( "Members only" ) );

    bool multiSelect = info.metadata( "multi-select" ).toBool();
    if ( multiSelect )
        details.append( tr( "Multiple selection" ) );

    return details.join( "; " );
}

QList<AttributeType> AttributeHelper::compatibleTypes( AttributeType type ) const
{
    QList<AttributeType> compatible;
    compatible << TextAttribute << EnumAttribute << UserAttribute;

    if ( compatible.contains( type ) )
        return compatible;

    return QList<AttributeType>() << type;
}
