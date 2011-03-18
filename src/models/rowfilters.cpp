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

#include "rowfilters.h"
#include "tablemodels.h"

#include "data/datamanager.h"
#include "data/datarows.h"
#include "utils/tablemodelshelper.h"
#include "utils/datetimehelper.h"
#include "utils/attributehelper.h"

IssueRowFilter::IssueRowFilter( QObject* parent ) : AbstractRowFilter( parent ),
    m_searchColumn( Column_Name )
{
}

IssueRowFilter::~IssueRowFilter()
{
}

void IssueRowFilter::setQuickSearch( int column, const QString& text )
{
    m_searchColumn = column;
    m_searchText = text;

    emit conditionsChanged();
}

void IssueRowFilter::setFilters( const QList<DefinitionInfo> filters )
{
    m_filters = filters;

    emit conditionsChanged();
}

bool IssueRowFilter::filterRow( int id )
{
    const IssueRow* row = dataManager->issues()->find( id );
    if ( !row )
        return false;

    if ( !m_searchText.isEmpty() ) {
        QString value = TableModelsHelper::extractValue( row, m_searchColumn );
        if ( !value.contains( m_searchText, Qt::CaseInsensitive ) )
            return false;
    }

    if ( !m_filters.isEmpty() ) {
        for ( int i = 0; i < m_filters.count(); i++ ) {
            if ( !matchValue( row, m_filters.at( i ) ) )
                return false;
        }
    }

    return true;
}

bool IssueRowFilter::matchValue( const IssueRow* row, const DefinitionInfo& filter )
{
    QString type = filter.type();
    int column = filter.metadata( "column" ).toInt();
    QString operand = filter.metadata( "value" ).toString();

    if ( column > Column_UserDefined ) {
        int attributeId = column - Column_UserDefined;

        const ValueRow* valueRow = dataManager->values()->find( attributeId, row->issueId() );
        QString value = valueRow ? valueRow->value() : QString();

        const AttributeRow* attribute = dataManager->attributes()->find( attributeId );
        if ( !attribute )
            return false;

        DefinitionInfo info = DefinitionInfo::fromString( attribute->definition() );
        AttributeType attributeType = AttributeHelper::toAttributeType( info );

        if ( attributeType == TextAttribute || attributeType == EnumAttribute || attributeType == UserAttribute ) {
            operand = convertUserValue( operand );

            return matchTextValue( type, value, operand );
        }

        if ( attributeType == DateTimeAttribute ) {
            QDateTime dateTimeValue = DateTimeHelper::parseDateTime( value );
            if ( !dateTimeValue.isValid() )
                return false;

            operand = convertDateTimeValue( operand );

            return matchDateTimeValue( type, dateTimeValue, DateTimeHelper::parseDate( operand ), info.metadata( "local" ).toBool() );
        }

        if ( attributeType == NumericAttribute ) {
            bool ok;
            double numericValue = value.toDouble( &ok );
            if ( !ok )
                return false;

            return matchNumericValue( type, numericValue, operand.toDouble() );
        }

        return false;
    }

    switch ( column ) {
        case Column_ID:
            return matchNumericValue( type, (double)row->issueId(), (double)operand.toInt() );

        case Column_Name:
        case Column_CreatedBy:
        case Column_ModifiedBy:
            operand = convertUserValue( operand );
            return matchTextValue( type, TableModelsHelper::extractValue( row, column ), operand );

        case Column_CreatedDate:
            operand = convertDateTimeValue( operand );
            return matchDateTimeValue( type, row->createdDate(), DateTimeHelper::parseDate( operand ), true );

        case Column_ModifiedDate:
            operand = convertDateTimeValue( operand );
            return matchDateTimeValue( type, row->modifiedDate(), DateTimeHelper::parseDate( operand ), true );

        default:
            return false;
    }
}

bool IssueRowFilter::matchTextValue( const QString& type, const QString& value1, const QString& value2 )
{
    if ( type == QLatin1String( "EQ" ) )
        return value1.compare( value2, Qt::CaseInsensitive ) == 0;
    if ( type == QLatin1String( "NEQ" ) )
        return value1.compare( value2, Qt::CaseInsensitive ) != 0;
    if ( type == QLatin1String( "CON" ) )
        return value1.contains( value2, Qt::CaseInsensitive );
    if ( type == QLatin1String( "BEG" ) )
        return value1.startsWith( value2, Qt::CaseInsensitive );
    if ( type == QLatin1String( "END" ) )
        return value1.endsWith( value2, Qt::CaseInsensitive );

    return false;
}

bool IssueRowFilter::matchNumericValue( const QString& type, double value1, double value2 )
{
    if ( type == QLatin1String( "EQ" ) )
        return value1 == value2;
    if ( type == QLatin1String( "NEQ" ) )
        return value1 != value2;
    if ( type == QLatin1String( "GT" ) )
        return value1 > value2;
    if ( type == QLatin1String( "GTE" ) )
        return value1 >= value2;
    if ( type == QLatin1String( "LT" ) )
        return value1 < value2;
    if ( type == QLatin1String( "LTE" ) )
        return value1 <= value2;

    return false;
}

bool IssueRowFilter::matchDateTimeValue( const QString& type, const QDateTime& value1, const QDate& value2, bool local )
{
    QDateTime lower = QDateTime( value2, QTime( 0, 0 ), local ? Qt::LocalTime : Qt::UTC );
    QDateTime upper = lower.addDays( 1 );

    if ( local ) {
        lower = lower.toUTC();
        upper = upper.toUTC();
    }

    if ( type == QLatin1String( "EQ" ) )
        return value1 >= lower && value1 < upper;
    if ( type == QLatin1String( "NEQ" ) )
        return value1 < lower || value1 >= upper;
    if ( type == QLatin1String( "GT" ) )
        return value1 >= upper;
    if ( type == QLatin1String( "GTE" ) )
        return value1 >= lower;
    if ( type == QLatin1String( "LT" ) )
        return value1 < lower;
    if ( type == QLatin1String( "LTE" ) )
        return value1 < upper;

    return false;
}

QString IssueRowFilter::convertUserValue( const QString& value )
{
    if ( value.startsWith( QLatin1String( "[Me]" ) ) )
        return dataManager->currentUserName();

    return value;
}

QString IssueRowFilter::convertDateTimeValue( const QString& value )
{
    if ( value.startsWith( QLatin1String( "[Today]" ) ) ) {
        QDate date = QDate::currentDate();
        if ( value.length() > 7 ) {
            int days = value.mid( 8 ).toInt();
            if ( value.at( 7 ) == '-' )
                days = -days;
            date = date.addDays( days );
        }
        return DateTimeHelper::formatDate( date );
    }

    return value;
}

ViewRowFilter::ViewRowFilter( bool isPublic, QObject* parent ) : AbstractRowFilter( parent ),
    m_isPublic( isPublic )
{
}

ViewRowFilter::~ViewRowFilter()
{
}

bool ViewRowFilter::filterRow( int id )
{
    const ViewRow* row = dataManager->views()->find( id );
    if ( !row )
        return false;

    return row->isPublic() == m_isPublic;
}
