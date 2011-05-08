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

#include "issuetypecache.h"

#include "models/foldermodel.h"
#include "utils/attributehelper.h"

IssueTypeCache::IssueTypeCache( int typeId, QObject* parent ) : QObject( parent )
{
    QString query = "SELECT attr_id, attr_name, attr_def"
        " FROM attr_types"
        " WHERE type_id = ?"
        " ORDER BY attr_name COLLATE LOCALE ASC";

    QSqlQuery sqlQuery;
    sqlQuery.prepare( query );
    sqlQuery.addBindValue( typeId );
    sqlQuery.exec();

    QList<int> remaining;

    while ( sqlQuery.next() ) {
        int attributeId = sqlQuery.value( 0 ).toInt();
        remaining.append( attributeId );

        m_attributeNames.insert( attributeId, sqlQuery.value( 1 ).toString() );

        DefinitionInfo info = DefinitionInfo::fromString( sqlQuery.value( 2 ).toString() );
        m_attributeDefinitions.insert( attributeId, info );
    }

    QStringList list;

    query = "SELECT set_key, set_value"
        " FROM view_settings"
        " WHERE type_id = ?";

    sqlQuery.prepare( query );
    sqlQuery.addBindValue( typeId );
    sqlQuery.exec();

    while ( sqlQuery.next() ) {
        QString key = sqlQuery.value( 0 ).toString();
        QString value = sqlQuery.value( 1 ).toString();
        if ( key == QLatin1String( "attribute_order" ) )
            list = value.split( ',' );
        else if ( key == QLatin1String( "default_view" ) )
            m_defaultView = DefinitionInfo::fromString( value );
    }

    for ( int i = 0; i < list.count(); i++ ) {
        bool ok;
        int attributeId = list.at( i ).toInt( &ok );
        if ( ok && remaining.contains( attributeId ) && !m_attributes.contains( attributeId ) ) {
            m_attributes.append( attributeId );
            remaining.removeOne( attributeId );
        }
    }

    m_attributes += remaining;
}

IssueTypeCache::~IssueTypeCache()
{
}

QList<int> IssueTypeCache::availableColumns() const
{
    QList<int> columns;
    columns.append( Column_ID );
    columns.append( Column_Name );
    columns.append( Column_CreatedDate );
    columns.append( Column_CreatedBy );
    columns.append( Column_ModifiedDate );
    columns.append( Column_ModifiedBy );

    foreach( int attributeId, m_attributes )
        columns.append( Column_UserDefined + attributeId );

    return columns;
}

QString IssueTypeCache::attributeName( int attributeId ) const
{
    return m_attributeNames.value( attributeId );
}

DefinitionInfo IssueTypeCache::attributeDefinition( int attributeId ) const
{
    return m_attributeDefinitions.value( attributeId );
}

QList<int> IssueTypeCache::viewColumns( const DefinitionInfo& info ) const
{
    QList<int> result;
    result.append( Column_ID );
    result.append( Column_Name );

    QString columns = info.metadata( "columns" ).toString();

    if ( !columns.isEmpty() ) {
        QList<int> available = availableColumns();

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

QPair<int, Qt::SortOrder> IssueTypeCache::viewSortOrder( const DefinitionInfo& info ) const
{
    QPair<int, Qt::SortOrder> result( Column_ID, Qt::AscendingOrder );

    bool ok;
    int column = info.metadata( "sort-column" ).toInt( &ok );

    if ( ok ) {
        QList<int> columns = viewColumns( info );

        if ( columns.contains( column ) ) {
            result.first = column;
            if ( info.metadata( "sort-desc" ).toBool() )
                result.second = Qt::DescendingOrder;
        }
    }

    return result;
}

QList<DefinitionInfo> IssueTypeCache::viewFilters( const DefinitionInfo& info ) const
{
    QList<DefinitionInfo> result;

    QStringList definitions = info.metadata( "filters" ).toStringList();

    if ( !definitions.isEmpty() ) {
        QList<int> columns = availableColumns();

        for ( int i = 0; i < definitions.count(); i++ ) {
            DefinitionInfo filter = DefinitionInfo::fromString( definitions.at( i ) );
            int column = filter.metadata( "column" ).toInt();
            if ( columns.contains( column ) )
                result.append( filter );
        }
    }

    return result;
}

DefinitionInfo IssueTypeCache::filterValueInfo( int column ) const
{
    DefinitionInfo result;

    if ( column > Column_UserDefined ) {
        int attributeId = column - Column_UserDefined;
        DefinitionInfo attributeInfo = attributeDefinition( attributeId );

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

QStringList IssueTypeCache::availableOperators( int column ) const
{
    QStringList result;
    result.append( "EQ" );
    result.append( "NEQ" );

    DefinitionInfo info = filterValueInfo( column );
    AttributeType type = AttributeHelper::toAttributeType( info );

    switch ( type ) {
        case TextAttribute:
        case EnumAttribute:
        case UserAttribute:
            result.append( "BEG" );
            result.append( "CON" );
            result.append( "END" );
            break;

        case NumericAttribute:
        case DateTimeAttribute:
            result.append( "LT" );
            result.append( "LTE" );
            result.append( "GT" );
            result.append( "GTE" );
            break;
    }

    return result;
}

QMap<int, int> IssueTypeCache::defaultWidths() const
{
    QMap<int, int> widths;

    widths.insert( Column_ID, 100 );
    widths.insert( Column_Name, 300 );
    widths.insert( Column_CreatedDate, 150 );
    widths.insert( Column_CreatedBy, 150 );
    widths.insert( Column_ModifiedDate, 150 );
    widths.insert( Column_ModifiedBy, 150 );

    foreach ( int attributeId, m_attributes )
        widths.insert( Column_UserDefined + attributeId, 150 );

    return widths;
}
