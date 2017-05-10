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

#include "formatter.h"

#include "data/datamanager.h"
#include "utils/definitioninfo.h"
#include "utils/datetimehelper.h"
#include "utils/attributehelper.h"

Formatter::Formatter()
{
}

Formatter::~Formatter()
{
}

QString Formatter::formatNumber( double number, int decimal, bool strip ) const
{
    DefinitionInfo info = dataManager->numberFormat();

    QString formatted;
    formatted.setNum( number, 'f', decimal );

    QString integerPart = formatted;
    QString fractionalPart;

    int pos = ( decimal > 0 ) ? formatted.indexOf( '.' ) : -1;
    if ( pos >= 0 ) {
        integerPart = formatted.left( pos );
        fractionalPart = formatted.mid( pos + 1 );
    }

    QString groupSeparator = info.metadata( "group-separator" ).toString();
    if ( !groupSeparator.isEmpty() ) {
        for ( int i = integerPart.length() - 3; i > 0 && integerPart.at( i - 1 ) != QLatin1Char( '-' ); i -= 3 )
            integerPart.insert( i, groupSeparator );
    }

    if ( pos >= 0 ) {
        if ( strip ) {
            int i = fractionalPart.length();
            while ( i > 0 && fractionalPart.at( i - 1 ) == QLatin1Char( '0' ) )
                i--;
            if ( i > 0 )
                fractionalPart.truncate( i );
            else
                return integerPart;
        }

        return integerPart + info.metadata( "decimal-separator" ).toString() + fractionalPart;
    }

    return integerPart;
}

QString Formatter::convertNumber( const QString& value, int decimal, bool strip ) const
{
    bool ok;
    double number = value.toDouble( &ok );

    if ( !ok )
        return QString();

    return formatNumber( number, decimal, strip );
}

QString Formatter::formatDate( const QDate& date ) const
{
    return date.toString( dateFormat() );
}

QString Formatter::convertDate( const QString& value ) const
{
    QDate date = DateTimeHelper::parseDate( value );

    if ( date.isNull() )
        return QString();

    return formatDate( date );
}

QString Formatter::formatDateTime( const QDateTime& dateTime, bool toLocal ) const
{
    if ( toLocal && dateTime.timeSpec() == Qt::UTC )
        return dateTime.toLocalTime().toString( dateTimeFormat() );

    return dateTime.toString( dateTimeFormat() );
}

QString Formatter::convertDateTime( const QString& value, bool toLocal ) const
{
    QDateTime dateTime = DateTimeHelper::parseDateTime( value );

    if ( dateTime.isNull() )
        return QString();

    dateTime.setTimeSpec( Qt::UTC );

    return formatDateTime( dateTime, toLocal );
}

QString Formatter::formatTime( const QTime& time ) const
{
    return time.toString( timeFormat() );
}

QString Formatter::dateFormat() const
{
    DefinitionInfo info = dataManager->dateFormat();

    QMap<QChar, QString> parts;
    parts.insert( QChar( 'd' ), info.metadata( "pad-day" ).toBool() ? "dd" : "d" );
    parts.insert( QChar( 'm' ), info.metadata( "pad-month" ).toBool() ? "MM" : "M" );
    parts.insert( QChar( 'y' ), "yyyy" );

    QString separator = info.metadata( "date-separator" ).toString();
    QString order = info.metadata( "date-order" ).toString();

    QString date = parts.value( order.at( 0 ) );
    date += separator;
    date += parts.value( order.at( 1 ) );
    date += separator;
    date += parts.value( order.at( 2 ) );

    return date;
}

QString Formatter::timeFormat() const
{
    DefinitionInfo info = dataManager->timeFormat();

    int mode = info.metadata( "time-mode" ).toInt();
    QString time = ( mode == 12 ) ? "h" : "H";
    if ( info.metadata( "pad-hour" ).toBool() )
        time += time;
    time += info.metadata( "time-separator" ).toString();
    time += QLatin1String( "mm" );
    if ( mode == 12 )
        time += QLatin1String( " ap" );

    return time;
}

QString Formatter::dateTimeFormat() const
{
    return dateFormat() + " " + timeFormat();
}

QString Formatter::convertAttributeValue( const DefinitionInfo& info, const QString& value, bool multiLine ) const
{
    if ( value.isEmpty() )
        return QString();

    switch ( AttributeHelper::toAttributeType( info ) ) {
        case TextAttribute:
            if ( info.metadata( "multi-line" ).toBool() && multiLine )
                return value;
            else
                return value.simplified();
            break;

        case EnumAttribute:
        case UserAttribute:
            return value.simplified();
            break;

        case NumericAttribute:
            return convertNumber( value, info.metadata( "decimal" ).toInt(), info.metadata( "strip" ).toBool() );

        case DateTimeAttribute:
            if ( info.metadata( "time" ).toBool() )
                return convertDateTime( value, info.metadata( "local" ).toBool() );
            else
                return convertDate( value );
            break;

        default:
            return QString();
    }
}

QString Formatter::formatSize( int size ) const
{
    if ( size >= 1048576 ) {
        double mb = (double)size / 1048576.0;
        return tr( "%1 MB" ).arg( formatNumber( mb, 1, true ) );
    }
    if ( size >= 1024 ) {
        double kb = (double)size / 1024.0;
        return tr( "%1 kB" ).arg( formatNumber( kb, 1, true ) );
    }
    return tr( "%1 bytes" ).arg( formatNumber( size, 0, false ) );
}
