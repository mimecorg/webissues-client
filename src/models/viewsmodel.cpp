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

#include "viewsmodel.h"

#include "data/datamanager.h"
#include "data/issuetypecache.h"
#include "utils/viewsettingshelper.h"
#include "utils/definitioninfo.h"
#include "utils/iconloader.h"

#include <QPixmap>

ViewsModel::ViewsModel( int typeId, bool isPublic, QObject* parent ) : BaseModel( parent ),
    m_typeId( typeId ),
    m_isPublic( isPublic )
{
    appendModel( new QSqlQueryModel( this ) );

    setColumnMapping( 0, QList<int>() << 1 << -1 << -1 << -1 );

    setHeaderData( 0, Qt::Horizontal, tr( "Name" ) );
    setHeaderData( 1, Qt::Horizontal, tr( "Columns" ) );
    setHeaderData( 2, Qt::Horizontal, tr( "Sort By" ) );
    setHeaderData( 3, Qt::Horizontal, tr( "Filter" ) );

    refresh();
}

ViewsModel::~ViewsModel()
{
}

QVariant ViewsModel::data( const QModelIndex& index, int role /*= Qt::DisplayRole*/ ) const
{
    int level = levelOf( index );
    int row = mappedRow( index );

    if ( role == Qt::DisplayRole ) {
        if ( index.column() > 0 ) {
            QString definition = rawData( level, row, 2, role ).toString();
            DefinitionInfo info = DefinitionInfo::fromString( definition );

            if ( !info.isEmpty() ) {
                ViewSettingsHelper helper( m_typeId );
                IssueTypeCache* cache = dataManager->issueTypeCache( m_typeId );

                switch ( index.column() ) {
                    case 1:
                        return helper.columnNames( cache->viewColumns( info ) );
                    case 2:
                        return helper.sortOrderInfo( cache->viewSortOrder( info ) );
                    case 3:
                        return helper.filtersInfo( cache->viewFilters( info ) );
                }
            }

            return QVariant();
        }

        return rawData( level, row, mappedColumn( index ), role );
    }

    if ( role == Qt::DecorationRole && index.column() == 0 )
        return IconLoader::pixmap( "view" );

    return QVariant();
}

void ViewsModel::refresh()
{
    QString query = "SELECT view_id, view_name, view_def"
        " FROM views"
        " WHERE type_id = ? AND is_public = ?"
        " ORDER BY view_name COLLATE LOCALE ASC";

    QSqlQuery sqlQuery;
    sqlQuery.prepare( query );
    sqlQuery.addBindValue( m_typeId );
    sqlQuery.addBindValue( m_isPublic ? 1 : 0 );
    sqlQuery.exec();

    modelAt( 0 )->setQuery( sqlQuery );

    updateData();
}

void ViewsModel::updateEvent( UpdateEvent* e )
{
    if ( e->unit() == UpdateEvent::Type )
        refresh();
}
