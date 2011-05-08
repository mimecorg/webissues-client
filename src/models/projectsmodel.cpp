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

#include "projectsmodel.h"

#include "data/datamanager.h"
#include "utils/iconloader.h"

#include <QPixmap>

ProjectsModel::ProjectsModel( QObject* parent ) : BaseModel( parent )
{
    for ( int i = 0; i < 3; i++ )
        appendModel( new QSqlQueryModel( this ) );

    setColumnMapping( 0, QList<int>() << 1 );
    setColumnMapping( 2, QList<int>() << 3 );

    setHeaderData( 0, Qt::Horizontal, tr( "Name" ) );
    setHeaderData( 1, Qt::Horizontal, tr( "Type" ) );

    setSort( 0, Qt::AscendingOrder );

    updateQueries();
}

ProjectsModel::~ProjectsModel()
{
}

QVariant ProjectsModel::data( const QModelIndex& index, int role /*= Qt::DisplayRole*/ ) const
{
    int level = levelOf( index );
    int row = mappedRow( index );

    if ( role == Qt::DisplayRole ) {
        QVariant value = rawData( level, row, mappedColumn( index ), role );

        if ( level == 2 && index.column() == 0 ) {
            int viewId = rawData( level, row, 2 ).toInt();
            QString name = ( viewId != 0 ) ? value.toString() : tr( "All Issues" );

            int alert = rawData( level, row, 5 ).toInt() + rawData( level, row, 6 ).toInt();
            if ( alert > 0 ) {
                value = name + QString( " (%1)" ).arg( alert );
            } else {
                int total = rawData( level, row, 4 ).toInt();
                if ( total > 0 )
                    value = name + QString( " [%1]" ).arg( total );
                else
                    value = name;
            }
        }

        return value;
    }

    if ( role == Qt::DecorationRole && index.column() == 0 ) {
        if ( level == 0 ) {
            if ( dataManager->currentUserAccess() == AdminAccess )
                return IconLoader::overlayedPixmap( "project", "overlay-admin" );
            int access = rawData( level, row, 2 ).toInt();
            if ( access == AdminAccess )
                return IconLoader::overlayedPixmap( "project", "overlay-admin" );
            return IconLoader::pixmap( "project" );
        } else if ( level == 1 ) {
            return IconLoader::pixmap( "folder" );
        } else if ( level == 2 ) {
            int unread = rawData( level, row, 6 ).toInt();
            if ( unread > 0 )
                return IconLoader::pixmap( "alert-unread" );
            int modified = rawData( level, row, 5 ).toInt();
            if ( modified > 0 )
                return IconLoader::pixmap( "alert-modified" );
            return IconLoader::pixmap( "alert" );
        }
    }

    if ( role == Qt::FontRole && level == 2 ) {
        int alert = rawData( level, row, 5 ).toInt() + rawData( level, row, 6 ).toInt();
        if ( alert > 0 ) {
            QFont font;
            font.setBold( true );
            return font;
        }
    }

    return QVariant();
}

void ProjectsModel::updateQueries()
{
    QString order = ( sortOrder() == Qt::AscendingOrder ) ? "ASC" : "DESC";

    switch ( sortColumn() ) {
        case 0:
            m_projectsOrder = QString( "project_name COLLATE LOCALE %1" ).arg( order );
            m_foldersOrder = QString( "f.folder_name COLLATE LOCALE %1" ).arg( order );
            m_alertsOrder = QString( "v.view_name COLLATE LOCALE %1" ).arg( order );
            break;

        case 1:
            m_foldersOrder = QString( "t.type_name COLLATE LOCALE %1" ).arg( order );
            break;
    }

    refresh();
}

void ProjectsModel::refresh()
{
    QString projectsQuery = "SELECT p.project_id, p.project_name, r.project_access"
        " FROM projects AS p"
        " LEFT OUTER JOIN rights AS r ON r.project_id = p.project_id AND r.user_id = ?";

    QString foldersQuery = "SELECT f.folder_id, f.project_id, f.folder_name, t.type_name"
        " FROM folders AS f"
        " JOIN issue_types AS t ON t.type_id = f.type_id";

    QString alertsQuery = "SELECT a.alert_id, f.folder_id, a.view_id, v.view_name, ac.total_count, ac.modified_count, ac.new_count"
        " FROM alerts AS a"
        " JOIN folders AS f ON f.folder_id = a.folder_id"
        " LEFT OUTER JOIN views AS v ON v.view_id = a.view_id"
        " LEFT OUTER JOIN alerts_cache AS ac ON ac.alert_id = a.alert_id";

    QSqlQuery sqlQuery;
    sqlQuery.prepare( QString( "%1 ORDER BY %2" ).arg( projectsQuery, m_projectsOrder ) );
    sqlQuery.addBindValue( dataManager->currentUserId() );
    sqlQuery.exec();

    modelAt( 0 )->setQuery( sqlQuery );
    modelAt( 1 )->setQuery( QString( "%1 ORDER BY %2" ).arg( foldersQuery, m_foldersOrder ) );
    modelAt( 2 )->setQuery( QString( "%1 ORDER BY %2" ).arg( alertsQuery, m_alertsOrder ) );

    updateData();
}

void ProjectsModel::updateEvent( UpdateEvent* e )
{
    if ( e->unit() == UpdateEvent::Projects || e->unit() == UpdateEvent::Types || e->unit() == UpdateEvent::AlertStates )
        refresh();
}
