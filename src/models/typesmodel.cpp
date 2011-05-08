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

#include "typesmodel.h"

#include "data/datamanager.h"
#include "utils/definitioninfo.h"
#include "utils/attributehelper.h"
#include "utils/iconloader.h"

#include <QPixmap>

TypesModel::TypesModel( QObject* parent ) : BaseModel( parent )
{
    for ( int i = 0; i < 2; i++ )
        appendModel( new QSqlQueryModel( this ) );

    setColumnMapping( 1, QList<int>() << 2 << -1 << -1 << -1 << -1 );

    setHeaderData( 0, Qt::Horizontal, tr( "Name" ) );
    setHeaderData( 1, Qt::Horizontal, tr( "Type" ) );
    setHeaderData( 2, Qt::Horizontal, tr( "Default Value" ) );
    setHeaderData( 3, Qt::Horizontal, tr( "Required" ) );
    setHeaderData( 4, Qt::Horizontal, tr( "Details" ) );

    refresh();
}

TypesModel::~TypesModel()
{
}

QVariant TypesModel::data( const QModelIndex& index, int role /*= Qt::DisplayRole*/ ) const
{
    int level = levelOf( index );
    int row = mappedRow( index );

    if ( role == Qt::DisplayRole ) {
        if ( level == 1 && index.column() > 0 ) {
            QString definition = rawData( level, row, 3, role ).toString();
            DefinitionInfo info = DefinitionInfo::fromString( definition );

            if ( !info.isEmpty() ) {
                AttributeHelper helper;

                switch ( index.column() ) {
                    case 1:
                        return helper.typeName( AttributeHelper::toAttributeType( info ) );
                    case 2:
                        return helper.formatExpression( info, info.metadata( "default" ).toString() );
                    case 3:
                        return info.metadata( "required" ).toBool() ? tr( "Yes" ) : tr( "No" );
                    case 4:
                        return helper.metadataDetails( info );
                }
            }

            return QVariant();
        }

        return rawData( level, row, mappedColumn( index ), role );
    }

    if ( role == Qt::DecorationRole && index.column() == 0 ) {
        if ( level == 0 )
            return IconLoader::pixmap( "type" );
        else if ( level == 1 )
            return IconLoader::pixmap( "attribute" );
    }

    return QVariant();
}

void TypesModel::refresh()
{
    QString typesQuery = "SELECT type_id, type_name"
        " FROM types"
        " ORDER BY type_name COLLATE LOCALE ASC";

    QString attributesQuery = "SELECT attr_id, type_id, attr_name, attr_def"
        " FROM attributes"
        " ORDER BY attr_name COLLATE LOCALE ASC";

    modelAt( 0 )->setQuery( typesQuery );
    modelAt( 1 )->setQuery( attributesQuery );

    updateData();
}

void TypesModel::updateEvent( UpdateEvent* e )
{
    if ( e->unit() == UpdateEvent::Types )
        refresh();
}
