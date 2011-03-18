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

#include "viewsettingshelper.h"
#include "tablemodelshelper.h"
#include "attributehelper.h"
#include "definitioninfo.h"
#include "formatter.h"

#include "data/datamanager.h"
#include "models/tablemodels.h"
#include "rdb/utilities.h"

#include <QApplication>
#include <QStringList>

QList<int> ViewSettingsHelper::attributeOrder( int typeId, const QString& attributes )
{
    QList<int> result;

    RDB::ForeignConstIterator<AttributeRow> it( dataManager->attributes()->parentIndex(), typeId );
    QList<const AttributeRow*> sorted = RDB::localeAwareSortRows( it, &AttributeRow::name );

    QList<int> remaining;
    for ( int i = 0; i < sorted.count(); i++ )
        remaining.append( sorted.at( i )->attributeId() );

    QStringList list = attributes.split( ',' );
    for ( int i = 0; i < list.count(); i++ ) {
        bool ok;
        int attributeId = list.at( i ).toInt( &ok );
        if ( ok && remaining.contains( attributeId ) && !result.contains( attributeId ) ) {
            result.append( attributeId );
            remaining.removeOne( attributeId );
        }
    }

    result += remaining;

    return result;
}

QString ViewSettingsHelper::attributeNames( const QList<int>& attributeIds )
{
    QStringList names;
    for ( int i = 0; i < attributeIds.count(); i++ )
        names.append( TableModelsHelper::attributeName( attributeIds.at( i ) ) );
    return names.join( ", " );
}

QList<int> ViewSettingsHelper::viewColumns( int typeId, const DefinitionInfo& info )
{
    QList<int> result;
    result.append( Column_ID );
    result.append( Column_Name );

    QString columns = info.metadata( "columns" ).toString();

    if ( !columns.isEmpty() ) {
        QList<int> available = availableColumns( typeId );

        QStringList list = columns.split( ',' );
        for ( int i = 0; i < list.count(); i++ ) {
            bool ok;
            int column = list.at( i ).toInt( &ok );
            if ( ok && available.contains( column ) && !result.contains( column ) )
                result.append( column );
        }
    } else {
        result.append( Column_ModifiedDate );
        result.append( Column_ModifiedBy );
    }

    return result;

}

QString ViewSettingsHelper::columnNames( const QList<int>& columns )
{
    QStringList names;
    for ( int i = 0; i < columns.count(); i++ )
        names.append( TableModelsHelper::columnName( columns.at( i ) ) );
    return names.join( ", " );
}

QPair<int, Qt::SortOrder> ViewSettingsHelper::viewSortOrder( int typeId, const DefinitionInfo& info )
{
    QPair<int, Qt::SortOrder> result( Column_ID, Qt::AscendingOrder );

    bool ok;
    int column = info.metadata( "sort-column" ).toInt( &ok );

    if ( ok ) {
        QList<int> columns = viewColumns( typeId, info );

        if ( columns.contains( column ) ) {
            result.first = column;
            if ( info.metadata( "sort-desc" ).toBool() )
                result.second = Qt::DescendingOrder;
        }
    }

    return result;
}

QString ViewSettingsHelper::sortOrderInfo(const QPair<int, Qt::SortOrder>& order )
{
    QString name = TableModelsHelper::columnName( order.first );

    if ( order.second == Qt::AscendingOrder )
        return tr( "%1 (ascending)" ).arg( name );
    else
        return tr( "%1 (descending)" ).arg( name );
}

QList<DefinitionInfo> ViewSettingsHelper::viewFilters( int typeId, const DefinitionInfo& info )
{
    QList<DefinitionInfo> result;

    QStringList definitions = info.metadata( "filters" ).toStringList();

    if ( !definitions.isEmpty() ) {
        QList<int> columns = availableColumns( typeId );

        for ( int i = 0; i < definitions.count(); i++ ) {
            DefinitionInfo filter = DefinitionInfo::fromString( definitions.at( i ) );
            int column = filter.metadata( "column" ).toInt();
            if ( columns.contains( column ) )
                result.append( filter );
        }
    }

    return result;
}

QString ViewSettingsHelper::filtersInfo( const QList<DefinitionInfo>& filters )
{
    QStringList result;

    for ( int i = 0; i < filters.count(); i++ ) {
        DefinitionInfo filter = filters.at( i );

        int column = filter.metadata( "column" ).toInt();
        QString columnName = TableModelsHelper::columnName( column );

        QString operatorName;
        if ( filter.type() == QLatin1String( "EQ" ) )
            operatorName = "=";
        else if ( filter.type() == QLatin1String( "NEQ" ) )
            operatorName = "<>";
        else if ( filter.type() == QLatin1String( "LT" ) )
            operatorName = "<";
        else if ( filter.type() == QLatin1String( "LTE" ) )
            operatorName = "<=";
        else if ( filter.type() == QLatin1String( "GT" ) )
            operatorName = ">";
        else if ( filter.type() == QLatin1String( "GTE" ) )
            operatorName = ">=";
        else if ( filter.type() == QLatin1String( "CON" ) )
            operatorName = tr( "contains" );
        else if ( filter.type() == QLatin1String( "BEG" ) )
            operatorName = tr( "begins with" );
        else if ( filter.type() == QLatin1String( "END" ) )
            operatorName = tr( "ends with" );

        QString value = filter.metadata( "value" ).toString();

        DefinitionInfo valueInfo = filterValueInfo( column );
        if ( !valueInfo.isEmpty() )
            value = AttributeHelper::formatExpression( valueInfo, valueInfo.toString(), value );

        result.append( QString( "%1 %2 %3" ).arg( columnName, operatorName, value ) );
    }

    return result.join( QString( " %1 " ).arg( tr( "AND" ) ) );
}

QList<int> ViewSettingsHelper::availableColumns( int typeId )
{
    QList<int> columns;
    columns.append( Column_ID );
    columns.append( Column_Name );
    columns.append( Column_CreatedDate );
    columns.append( Column_CreatedBy );
    columns.append( Column_ModifiedDate );
    columns.append( Column_ModifiedBy );

    QList<int> attributes = attributeOrder( typeId, dataManager->viewSetting( typeId, "attribute_order" ) );
    foreach( int attributeId, attributes )
        columns.append( Column_UserDefined + attributeId );

    return columns;
}

DefinitionInfo ViewSettingsHelper::filterValueInfo( int column )
{
    DefinitionInfo result;

    if ( column > Column_UserDefined ) {
        int attributeId = column - Column_UserDefined;
        const AttributeRow* attribute = dataManager->attributes()->find( attributeId );

        if ( attribute ) {
            DefinitionInfo attributeInfo = DefinitionInfo::fromString( attribute->definition() );

            switch ( AttributeHelper::toAttributeType( attributeInfo ) ) {
                case TextAttribute:
                case EnumAttribute:
                case UserAttribute:
                    result = AttributeHelper::fromAttributeType( TextAttribute );
                    break;

                case NumericAttribute:
                    result = AttributeHelper::fromAttributeType( NumericAttribute );
                    result.setMetadata( "decimal", attributeInfo.metadata( "decimal" ) );
                    result.setMetadata( "strip", attributeInfo.metadata( "strip" ) );
                    break;

                case DateTimeAttribute:
                    result = AttributeHelper::fromAttributeType( DateTimeAttribute );
                    break;

                default:
                    break;
            }
        }
    }

    switch ( column ) {
        case Column_ID:
            result = AttributeHelper::fromAttributeType( NumericAttribute );
            break;

        case Column_Name:
        case Column_CreatedBy:
        case Column_ModifiedBy:
            result = AttributeHelper::fromAttributeType( TextAttribute );
            break;

        case Column_CreatedDate:
        case Column_ModifiedDate:
            result = AttributeHelper::fromAttributeType( DateTimeAttribute );
            break;

        default:
            break;
    }

    return result;
}

QStringList ViewSettingsHelper::availableOperators( int column )
{
    QStringList result;
    result.append( "EQ" );
    result.append( "NEQ" );

    DefinitionInfo info = ViewSettingsHelper::filterValueInfo( column );
    AttributeType type = AttributeHelper::toAttributeType( info );

    if ( type == TextAttribute || type == EnumAttribute || type == UserAttribute ) {
        result.append( "BEG" );
        result.append( "CON" );
        result.append( "END" );
    }

    if ( type == NumericAttribute || type == DateTimeAttribute ) {
        result.append( "LT" );
        result.append( "LTE" );
        result.append( "GT" );
        result.append( "GTE" );
    }

    return result;
}

QString ViewSettingsHelper::operatorName( const QString& type )
{
    if ( type == QLatin1String( "EQ" ) )
        return tr( "is equal to" );
    if ( type == QLatin1String( "NEQ" ) )
        return tr( "is not equal to" );
    if ( type == QLatin1String( "LT" ) )
        return tr( "is less than" );
    if ( type == QLatin1String( "LTE" ) )
        return tr( "is less than or equal to" );
    if ( type == QLatin1String( "GT" ) )
        return tr( "is greater than" );
    if ( type == QLatin1String( "GTE" ) )
        return tr( "is greater than or equal to" );
    if ( type == QLatin1String( "CON" ) )
        return tr( "contains" );
    if ( type == QLatin1String( "BEG" ) )
        return tr( "begins with" );
    if ( type == QLatin1String( "END" ) )
        return tr( "ends with" );
    return QString();
}

QString ViewSettingsHelper::tr( const char* text )
{
    return qApp->translate( "ViewSettingsHelper", text );
}
