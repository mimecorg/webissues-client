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

#include "validator.h"

#include "data/datamanager.h"
#include "utils/datetimehelper.h"
#include "utils/errorhelper.h"

#include <QRegExp>
#include <QSet>

#include <math.h>

Validator::Validator()
{
}

Validator::~Validator()
{
}

QString Validator::normalizeString( const QString& string, int maxLength )
{
    QString result = string.simplified();

    if ( result.isEmpty() )
        return QString();

    if ( result.length() > maxLength ) {
        appendError( ErrorHelper::StringTooLong );
        return QString();
    }

    QRegExp nonPrintableRegExp( "[\\x00-\\x1f\\x7f]" );

    if ( result.contains( nonPrintableRegExp ) ) {
        appendError( ErrorHelper::InvalidString );
        return QString();
    }

    return result;
}

QString Validator::normalizeMultiLineString( const QString& string )
{
    QString result = string;

    int i = result.length();
    while ( i > 0 && result.at( i - 1 ).isSpace() )
        i--;
    result.truncate( i );

    if ( result.isEmpty() )
        return QString();

    QRegExp nonPrintableRegExp( "[\\x00-\\x08\\x0b-\\x1f\\x7f]" );

    if ( result.contains( nonPrintableRegExp ) ) {
        appendError( ErrorHelper::InvalidString );
        return QString();
    }

    return result;
}

QStringList Validator::normalizeStringList( const QStringList& list, int maxLength )
{
    QStringList result;

    for ( int i = 0; i < list.count(); i++ ) {
        QString normalized = normalizeString( list.at( i ), maxLength );
        if ( !normalized.isEmpty() )
            result.append( normalized );
    }

    return result;
}

bool Validator::checkEmail( const QString& email )
{
    QRegExp emailRegExp(
        "[a-zA-Z0-9_\\-\\.\\+\\^!#\\$%&*+\\/\\=\\?\\`\\|\\{\\}~\\']+" // user
        "@("
        "(?:(?:[a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\\-]*[a-zA-Z0-9])\\.?)+" // domain
        "|(\\[("
        "[0-9]{1,3}(\\.[0-9]{1,3}){3}" // IPv4
        "|"
        "[0-9a-fA-F]{1,4}(\\:[0-9a-fA-F]{1,4}){7}" // IPv6
        ")\\]))" );

    if ( !emailRegExp.exactMatch( email ) ) {
        appendError( ErrorHelper::InvalidEmail );
        return false;
    }

    return true;
}

DefinitionInfo Validator::createAttributeDefinition( AttributeType type, const QVariantMap& metadata )
{
    DefinitionInfo info = AttributeHelper::fromAttributeType( type );

    switch ( type ) {
        case TextAttribute: {
            if ( metadata.value( "multi-line" ).toBool() )
                info.setMetadata( "multi-line", 1 );
            if ( !setMinMaxLength( info, metadata ) )
                return DefinitionInfo();
            break;
        }

        case EnumAttribute: {
            bool editable = metadata.value( "editable" ).toBool();
            if ( editable )
                info.setMetadata( "editable", 1 );
            bool multiSelect = metadata.value( "multi-select" ).toBool();
            if ( multiSelect )
                info.setMetadata( "multi-select", 1 );
            QStringList items = metadata.value( "items" ).toStringList();
            if ( items.isEmpty() ) {
                appendError( ErrorHelper::NoItems );
                return DefinitionInfo();
            }
            QSet<QString> seen;
            seen.reserve( items.count() );
            for ( int i = 0; i < items.count(); i++ ) {
                if ( seen.contains( items.at( i ) ) ) {
                    appendError( ErrorHelper::DuplicateItems );
                    return DefinitionInfo();
                }
                if ( multiSelect && items.at( i ).contains( QLatin1Char( ',' ) ) ) {
                    appendError( ErrorHelper::CommaNotAllowed );
                    return DefinitionInfo();
                }
                seen.insert( items.at( i ) );
            }
            info.setMetadata( "items", items );
            if ( editable && !multiSelect ) {
                if ( !setMinMaxLength( info, metadata ) )
                    return DefinitionInfo();
                for ( int i = 0; i < items.count(); i++ ) {
                    if ( !checkStringLength( items.at( i ), info ) )
                        return DefinitionInfo();
                }
            }
            break;
        }

        case NumericAttribute: {
            int decimal = metadata.value( "decimal" ).toInt();
            if ( !checkInteger( decimal, 0, 6 ) )
                return DefinitionInfo();
            info.setMetadata( "decimal", decimal );
            if ( metadata.value( "strip" ).toBool() )
                info.setMetadata( "strip", 1 );
            QVariant minValue = metadata.value( "min-value" );
            if ( !minValue.isNull() )
                info.setMetadata( "min-value", minValue.toString() );
            QVariant maxValue = metadata.value( "max-value" );
            if ( !maxValue.isNull() )
                info.setMetadata( "max-value", maxValue.toString() );
            if ( !minValue.isNull() && !maxValue.isNull() && minValue.toDouble() > maxValue.toDouble() ) {
                appendError( ErrorHelper::InvalidLimits );
                return DefinitionInfo();
            }
            break;
        }

        case DateTimeAttribute: {
            if ( metadata.value( "time" ).toBool() ) {
                info.setMetadata( "time", 1 );
                if ( metadata.value( "local" ).toBool() )
                    info.setMetadata( "local", 1 );
            }
            break;
        }

        case UserAttribute: {
            if ( metadata.value( "members" ).toBool() )
                info.setMetadata( "members", 1 );
            if ( metadata.value( "multi-select" ).toBool() )
                info.setMetadata( "multi-select", 1 );
            break;
        }

        default:
            break;
    }

    return info;
}

bool Validator::checkInteger( int value, int minValue, int maxValue )
{
    if ( value < minValue ) {
        appendError( ErrorHelper::NumberTooLittle );
        return false;
    }

    if ( value > maxValue ) {
        appendError( ErrorHelper::NumberTooGreat );
        return false;
    }

    return true;
}

bool Validator::checkStringLength( const QString& string, const DefinitionInfo& info )
{
    int length = string.length();

    QVariant minLength = info.metadata( "min-length" );
    if ( !minLength.isNull() && length < minLength.toInt() ) {
        appendError( ErrorHelper::StringTooShort );
        return false;
    }

    QVariant maxLength = info.metadata( "max-length" );
    if ( !maxLength.isNull() && length > maxLength.toInt() ) {
        appendError( ErrorHelper::StringTooLong );
        return false;
    }

    return true;
}

bool Validator::setMinMaxLength( DefinitionInfo& info, const QVariantMap& metadata )
{
    QVariant minLength = metadata.value( "min-length" );
    if ( !minLength.isNull() ) {
        int value = minLength.toInt();
        if ( !checkInteger( value, 1, 255 ) )
            return false;
        info.setMetadata( "min-length", value );
    }

    QVariant maxLength = metadata.value( "max-length" );
    if ( !maxLength.isNull() ) {
        int value = maxLength.toInt();
        if ( !checkInteger( value, 1, 255 ) )
            return false;
        info.setMetadata( "max-length", value );
    }

    if ( !minLength.isNull() && !maxLength.isNull() && minLength.toInt() > maxLength.toInt() ) {
        appendError( ErrorHelper::InvalidLimits );
        return false;
    }

    return true;
}

double Validator::parseNumber( const QString& value, int decimal )
{
    DefinitionInfo numberFormat = dataManager->numberFormat();

    QString pattern;

    QString groupSeparator = numberFormat.metadata( "group-separator" ).toString();
    if ( !groupSeparator.isEmpty() )
        pattern = "(-?\\d\\d?\\d?(?:" + QRegExp::escape( groupSeparator ) + "\\d\\d\\d)+|-?\\d+)";
    else
        pattern = "(-?\\d+)";

    pattern += "(?:" + QRegExp::escape( numberFormat.metadata( "decimal-separator" ).toString() ) + "(\\d*))?";

    QRegExp patternRegExp( pattern );

    if ( !patternRegExp.exactMatch( value ) ) {
        appendError( ErrorHelper::InvalidFormat );
        return 0.0;
    }

    QString fractionPart = patternRegExp.cap( 2 );
    int i = fractionPart.length();
    while ( i > 0 && fractionPart.at( i - 1 ) == '0' )
        i--;
    fractionPart.truncate( i );

    if ( i > decimal ) {
        appendError( ErrorHelper::TooManyDecimals );
        return 0.0;
    }

    QString integerPart = patternRegExp.cap( 1 );
    if ( !groupSeparator.isEmpty() )
        integerPart.replace( groupSeparator, "" );

    double number;
    if ( !fractionPart.isEmpty() )
        number = ( integerPart + QChar( '.' ) + fractionPart ).toDouble();
    else
        number = integerPart.toDouble();

    if ( fabs( number ) >= pow( 10.0, 14 - decimal ) ) {
        appendError( ErrorHelper::TooManyDigits );
        return 0.0;
    }

    return number;
}

QString Validator::convertNumber( const QString& value, int decimal )
{
    double number = parseNumber( value, decimal );
    return QString::number( number, 'f', decimal );
}

QDate Validator::parseDate( const QString& value )
{
    DefinitionInfo dateFormat = dataManager->dateFormat();

    QString dateSeparator = dateFormat.metadata( "date-separator" ).toString();
    QString order = dateFormat.metadata( "date-order" ).toString();

    QString pattern = buildDatePattern( dateSeparator, order );

    QRegExp patternRegExp( pattern );

    if ( !patternRegExp.exactMatch( value ) ) {
        appendError( ErrorHelper::InvalidFormat );
        return QDate();
    }

    QMap<QChar, int> matches;
    for ( int i = 0; i < 3; i++ )
        matches.insert( order.at( i ), patternRegExp.cap( i + 1 ).toInt() );

    QDate date( matches.value( 'y' ), matches.value( 'm' ), matches.value( 'd' ) );

    if ( !date.isValid() ) {
        appendError( ErrorHelper::InvalidDate );
        return QDate();
    }

    return date;
}

QString Validator::convertDate( const QString& value )
{
    QDate date = parseDate( value );
    return DateTimeHelper::formatDate( date );
}

QDateTime Validator::parseDateTime( const QString& value, bool fromLocal )
{
    DefinitionInfo dateFormat = dataManager->dateFormat();

    QString dateSeparator = dateFormat.metadata( "date-separator" ).toString();
    QString order = dateFormat.metadata( "date-order" ).toString();

    QString pattern = buildDatePattern( dateSeparator, order );

    DefinitionInfo timeFormat = dataManager->timeFormat();

    pattern += QLatin1String( " (\\d\\d?)" );
    pattern += QRegExp::escape( timeFormat.metadata( "time-separator" ).toString() );
    pattern += QLatin1String( "(\\d\\d?)" );

    int mode = timeFormat.metadata( "time-mode" ).toInt();
    if ( mode == 12 )
        pattern += QLatin1String( "(?: ?([AaPp][Mm]))?" );

    QRegExp patternRegExp( pattern );

    if ( !patternRegExp.exactMatch( value ) ) {
        appendError( ErrorHelper::InvalidFormat );
        return QDateTime();
    }

    QMap<QChar, int> matches;
    for ( int i = 0; i < 3; i++ )
        matches.insert( order.at( i ), patternRegExp.cap( i + 1 ).toInt() );

    QDate date( matches.value( 'y' ), matches.value( 'm' ), matches.value( 'd' ) );

    if ( !date.isValid() ) {
        appendError( ErrorHelper::InvalidDate );
        return QDateTime();
    }

    int hours = patternRegExp.cap( 4 ).toInt();
    int minutes = patternRegExp.cap( 5 ).toInt();

    if ( mode == 12 && !patternRegExp.cap( 6 ).isEmpty() ) {
        if ( hours < 1 || hours > 12 ) {
            appendError( ErrorHelper::InvalidTime );
            return QDateTime();
        }
        if ( hours == 12 )
            hours = 0;
        if ( patternRegExp.cap( 6 ).toLower() == QLatin1String( "pm" ) )
            hours += 12;
    } else {
        if ( hours > 23 ) {
            appendError( ErrorHelper::InvalidTime );
            return QDateTime();
        }
    }

    if ( minutes > 59 ) {
        appendError( ErrorHelper::InvalidTime );
        return QDateTime();
    }

    QDateTime dateTime( date, QTime( hours, minutes ), Qt::LocalTime );

    if ( fromLocal )
        dateTime = dateTime.toUTC();

    return dateTime;
}

QString Validator::convertDateTime( const QString& value, bool fromLocal )
{
    QDateTime dateTime = parseDateTime( value, fromLocal );
    return DateTimeHelper::formatDateTime( dateTime );
}

QString Validator::buildDatePattern( const QString& separator, const QString& order ) const
{
    QString escaped = QRegExp::escape( separator );

    QMap<QChar, QString> parts;
    parts.insert( QChar( 'd' ), "(\\d\\d?)" );
    parts.insert( QChar( 'm' ), "(\\d\\d?)" );
    parts.insert( QChar( 'y' ), "(\\d\\d\\d\\d)" );

    QString pattern = parts.value( order.at( 0 ) );
    pattern += separator;
    pattern += parts.value( order.at( 1 ) );
    pattern += separator;
    pattern += parts.value( order.at( 2 ) );

    return pattern;
}

void Validator::appendError( int code )
{
    ErrorHelper helper;
    m_errors.append( helper.errorMessage( (ErrorHelper::ErrorCode)code ) );
}
