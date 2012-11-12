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

#include "viewsettingshelper.h"

#include "data/datamanager.h"
#include "data/issuetypecache.h"
#include "data/entities.h"
#include "models/foldermodel.h"
#include "utils/attributehelper.h"

ViewSettingsHelper::ViewSettingsHelper( int typeId ) :
    m_typeId( typeId )
{
}

ViewSettingsHelper::~ViewSettingsHelper()
{
}

QString ViewSettingsHelper::attributeNames( const QList<int>& attributes ) const
{
    IssueTypeCache* cache = dataManager->issueTypeCache( m_typeId );

    QStringList names;
    for ( int i = 0; i < attributes.count(); i++ )
        names.append( cache->attributeName( attributes.at( i ) ) );
    return names.join( ", " );
}

QString ViewSettingsHelper::columnName( int column ) const
{
    switch ( column ) {
        case Column_Name:
            return tr( "Name" );
        case Column_ID:
            return tr( "ID" );
        case Column_CreatedDate:
            return tr( "Created Date" );
        case Column_CreatedBy:
            return tr( "Created By" );
        case Column_ModifiedDate:
            return tr( "Modified Date" );
        case Column_ModifiedBy:
            return tr( "Modified By" );
        default:
            if ( column > Column_UserDefined ) {
                IssueTypeCache* cache = dataManager->issueTypeCache( m_typeId );
                return cache->attributeName( column - Column_UserDefined );
            }
            return QString();
    }
}

QString ViewSettingsHelper::columnNames( const QList<int>& columns ) const
{
    QStringList names;
    for ( int i = 0; i < columns.count(); i++ )
        names.append( columnName( columns.at( i ) ) );
    return names.join( ", " );
}

QString ViewSettingsHelper::sortOrderInfo(const QPair<int, Qt::SortOrder>& order ) const
{
    QString name = columnName( order.first );

    if ( order.second == Qt::AscendingOrder )
        return tr( "%1 (ascending)" ).arg( name );
    else
        return tr( "%1 (descending)" ).arg( name );
}

QString ViewSettingsHelper::filtersInfo( const QList<DefinitionInfo>& filters ) const
{
    QStringList result;

    for ( int i = 0; i < filters.count(); i++ ) {
        DefinitionInfo filter = filters.at( i );

        int column = filter.metadata( "column" ).toInt();
        QString name = columnName( column );

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
        else if ( filter.type() == QLatin1String( "IN" ) )
            operatorName = tr( "in" );

        QString value = filter.metadata( "value" ).toString();

        IssueTypeCache* cache = dataManager->issueTypeCache( m_typeId );
    
        DefinitionInfo valueInfo = cache->filterValueInfo( column );
        if ( !valueInfo.isEmpty() ) {
            AttributeHelper helper;
            value = helper.formatExpression( valueInfo, value );
        }

        if ( !value.isEmpty() )
            value = QString( "\"%1\"" ).arg( value );
        else
            value = tr( "empty" );

        result.append( QString( "%1 %2 %3" ).arg( name, operatorName, value ) );
    }

    return result.join( QString( " %1 " ).arg( tr( "AND" ) ) );
}

QString ViewSettingsHelper::operatorName( const QString& type ) const
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
    if ( type == QLatin1String( "IN" ) )
        return tr( "in list" );
    return QString();
}

QString ViewSettingsHelper::viewName( int viewId ) const
{
    if ( viewId != 0 ) {
        ViewEntity view = ViewEntity::find( viewId );
        if ( view.isValid() )
            return view.name();
    }
    return tr( "All Issues" );
}
