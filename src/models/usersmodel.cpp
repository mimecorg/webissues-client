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

#include "usersmodel.h"

#include "data/datamanager.h"
#include "utils/iconloader.h"

#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QPixmap>

UsersModel::UsersModel( QObject* parent ) : BaseModel( parent ),
    m_filter( UsersModel::AllUsers )
{
    appendModel( new QSqlQueryModel( this ) );

    setHeaderData( 0, Qt::Horizontal, tr( "Name" ) );
    setHeaderData( 1, Qt::Horizontal, tr( "Login" ) );
    setHeaderData( 2, Qt::Horizontal, tr( "Access" ) );

    setSort( 0, Qt::AscendingOrder );

    updateQueries();
}

UsersModel::~UsersModel()
{
}

void UsersModel::setFilter( Filter filter )
{
    if ( m_filter != filter ) {
        m_filter = filter;
        refresh();
    }
}

QVariant UsersModel::data( const QModelIndex& index, int role /*= Qt::DisplayRole*/ ) const
{
    int level = levelOf( index );
    int row = mappedRow( index );

    if ( role == Qt::DisplayRole ) {
        QVariant value = rawData( level, row, mappedColumn( index ), role );

        if ( index.column() == 2 ) {
            int access = value.toInt();
            if ( access == NormalAccess )
                return tr( "Regular user" );
            if ( access == AdminAccess )
                return tr( "System administrator" );
            return tr( "Disabled" );
        }

        return value;
    }

    if ( role == Qt::DecorationRole && index.column() == 0 ) {
        int access = rawData( level, row, 3, Qt::DisplayRole ).toInt();
        if ( access == AdminAccess )
            return IconLoader::overlayedPixmap( "user", "overlay-admin" );
        if ( access == NoAccess )
            return IconLoader::pixmap( "user-disabled" );
        return IconLoader::pixmap( "user" );
    }

    return QVariant();
}

void UsersModel::updateQueries()
{
    QString order = ( sortOrder() == Qt::AscendingOrder ) ? "ASC" : "DESC";

    switch ( sortColumn() ) {
        case 0:
            m_order = QString( "user_name COLLATE LOCALE %1" ).arg( order );
            break;

        case 1:
            m_order = QString( "user_login COLLATE LOCALE %1" ).arg( order );
            break;

        case 2:
            m_order = QString( "user_access %1" ).arg( order );
            break;
    }

    refresh();
}

void UsersModel::refresh()
{
    QString query = "SELECT user_id, user_name, user_login, user_access"
        " FROM users";

    if ( m_filter == Active )
        query += " WHERE user_access > 0";
    else if ( m_filter == Disabled )
        query += " WHERE user_access = 0";

    modelAt( 0 )->setQuery( QString( "%1 ORDER BY %2" ).arg( query, m_order ) );

    updateData();
}

void UsersModel::updateEvent( UpdateEvent* e )
{
    if ( e->unit() == UpdateEvent::Users )
        refresh();
}
