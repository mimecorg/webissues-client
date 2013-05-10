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

#include "userprojectsmodel.h"

#include "data/datamanager.h"
#include "utils/iconloader.h"

#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QPixmap>

UserProjectsModel::UserProjectsModel( int userId, QObject* parent ) : BaseModel( parent ),
    m_userId( userId )
{
    appendModel( new QSqlQueryModel( this ) );

    setHeaderData( 0, Qt::Horizontal, tr( "Name" ) );
    setHeaderData( 1, Qt::Horizontal, tr( "Access" ) );

    setSort( 0, Qt::AscendingOrder );

    updateQueries();
}

UserProjectsModel::~UserProjectsModel()
{
}

QVariant UserProjectsModel::data( const QModelIndex& index, int role ) const
{
    int level = levelOf( index );
    int row = mappedRow( index );

    if ( role == Qt::DisplayRole ) {
        QVariant value = rawData( level, row, mappedColumn( index ), role );

        if ( index.column() == 1 ) {
            int access = value.toInt();
            if ( access == NormalAccess )
                return tr( "Regular member" );
            if ( access == AdminAccess )
                return tr( "Project administrator" );
            return QString();
        }

        return value;
    }

    if ( role == Qt::DecorationRole && index.column() == 0 ) {
        int access = rawData( level, row, 2, Qt::DisplayRole ).toInt();
        if ( access == AdminAccess )
            return IconLoader::overlayedPixmap( "project", "overlay-admin" );
        return IconLoader::pixmap( "project" );
    }

    return QVariant();
}

void UserProjectsModel::updateQueries()
{
    QString order = ( sortOrder() == Qt::AscendingOrder ) ? "ASC" : "DESC";

    switch ( sortColumn() ) {
        case 0:
            m_order = QString( "project_name COLLATE LOCALE %1" ).arg( order );
            break;

        case 1:
            m_order = QString( "project_access %1" ).arg( order );
            break;
    }

    refresh();
}

void UserProjectsModel::refresh()
{
    QString query = "SELECT p.project_id, p.project_name, r.project_access"
        " FROM projects AS p"
        " JOIN rights AS r ON r.project_id = p.project_id"
        " WHERE r.user_id = ?";

    QSqlQuery sqlQuery;
    sqlQuery.prepare( QString( "%1 ORDER BY %2" ).arg( query, m_order ) );
    sqlQuery.addBindValue( m_userId );
    sqlQuery.exec();

    modelAt( 0 )->setQuery( sqlQuery );

    updateData();
}

void UserProjectsModel::updateEvent( UpdateEvent* e )
{
    if ( e->unit() == UpdateEvent::Users || e->unit() == UpdateEvent::Projects )
        refresh();
}
