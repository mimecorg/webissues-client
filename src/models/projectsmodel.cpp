/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2015 WebIssues Team
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

#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QPixmap>
#include <QFont>

ProjectsModel::ProjectsModel( QObject* parent ) : BaseModel( parent )
{
    appendModel( new QSqlQueryModel( this ) );
    appendModel( new QSqlQueryModel( this ) );
    appendModel( new QSqlQueryModel( this ) );
    insertModel( new QSqlQueryModel( this ) );
    appendModel( new QSqlQueryModel( this ) );
    appendModel( new QSqlQueryModel( this ) );

    setColumnMapping( Projects, QList<int>() << 1 );
    setColumnMapping( GlobalAlerts, QList<int>() << 3 );
    setColumnMapping( Alerts, QList<int>() << 3 );

    setHeaderData( 0, Qt::Horizontal, tr( "Name" ) );
    setHeaderData( 1, Qt::Horizontal, tr( "Type" ) );

    setSort( 0, Qt::AscendingOrder );

    updateQueries();
}

ProjectsModel::~ProjectsModel()
{
}

QVariant ProjectsModel::data( const QModelIndex& index, int role ) const
{
    int level = levelOf( index );
    int row = mappedRow( index );

    if ( role == Qt::DisplayRole ) {
        QVariant value = rawData( level, row, mappedColumn( index ), role );

        if ( level == AllProjects && index.column() == 0 )
            value = tr( "All Projects" );

        if ( ( level == Alerts || level == GlobalAlerts ) && index.column() == 0 ) {
            int viewId = rawData( level, row, 2 ).toInt();
            QString name = ( viewId != 0 ) ? value.toString() : tr( "All Issues" );

            int alert = rawData( level, row, 5 ).toInt() + rawData( level, row, 6 ).toInt();
            if ( alert > 0 )
                value = name + QString( " (%1)" ).arg( alert );
            else
                value = name;
        }

        return value;
    }

    if ( role == Qt::DecorationRole && index.column() == 0 ) {
        if ( level == AllProjects ) {
            return IconLoader::pixmap( "project-all" );
        } else if ( level == Types ) {
            return IconLoader::pixmap( "folder-type" );
        } else if ( level == Projects ) {
            bool isPublic = rawData( level, row, 3 ).toBool();
            bool isAdmin = dataManager->currentUserAccess() == AdminAccess || rawData( level, row, 2 ).toInt() == AdminAccess;
            if ( isPublic && isAdmin )
                return IconLoader::overlayedPixmap( "project", "overlay-public", "overlay-admin" );
            if ( isPublic )
                return IconLoader::overlayedPixmap( "project", "overlay-public" );
            if ( isAdmin )
                return IconLoader::overlayedPixmap( "project", "overlay-admin" );
            return IconLoader::pixmap( "project" );
        } else if ( level == Folders ) {
            return IconLoader::pixmap( "folder" );
        } else if ( level == Alerts || level == GlobalAlerts ) {
            QString name = "alert";
            int unread = rawData( level, row, 6 ).toInt();
            if ( unread > 0 ) {
                name = "alert-unread";
            } else {
                int modified = rawData( level, row, 5 ).toInt();
                if ( modified > 0 )
                    name = "alert-modified";
            }
            if ( rawData( level, row, 7 ).toBool() )
                return IconLoader::overlayedPixmap( name, "overlay-public" );
            return IconLoader::pixmap( name );
        }
    }

    if ( role == Qt::FontRole && ( level == Alerts || level == GlobalAlerts ) ) {
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
            m_typesOrder = QString( "t.type_name COLLATE LOCALE %1" ).arg( order );
            m_projectsOrder = QString( "p.project_name COLLATE LOCALE %1" ).arg( order );
            m_foldersOrder = QString( "f.folder_name COLLATE LOCALE %1" ).arg( order );
            m_alertsOrder = QString( "v.view_name COLLATE LOCALE %1" ).arg( order );
            break;

        case 1:
            m_typesOrder = QString( "t.type_name COLLATE LOCALE %1" ).arg( order );
            m_foldersOrder = QString( "t.type_name COLLATE LOCALE %1" ).arg( order );
            break;
    }

    refresh();
}

void ProjectsModel::refresh()
{
    QString allProjectsQuery = "SELECT 0";

    QString typesQuery = "SELECT t.type_id, 0, t.type_name"
        " FROM issue_types AS t";
    if ( dataManager->currentUserAccess() != AdminAccess )
        typesQuery += " WHERE t.type_id IN ( SELECT f.type_id FROM folders AS f JOIN effective_rights AS r ON r.project_id = f.project_id AND r.user_id = ? )";

    QString globalAlertsQuery = "SELECT a.alert_id, t.type_id, a.view_id, v.view_name, ac.total_count, ac.modified_count, ac.new_count, a.is_public"
        " FROM alerts AS a"
        " JOIN issue_types AS t ON t.type_id = a.type_id"
        " LEFT OUTER JOIN views AS v ON v.view_id = a.view_id"
        " LEFT OUTER JOIN alerts_cache AS ac ON ac.alert_id = a.alert_id";

    QString projectsQuery = "SELECT p.project_id, p.project_name, r.project_access, p.is_public"
        " FROM projects AS p"
        " LEFT OUTER JOIN effective_rights AS r ON r.project_id = p.project_id AND r.user_id = ?";

    QString foldersQuery = "SELECT f.folder_id, f.project_id, f.folder_name, t.type_name"
        " FROM folders AS f"
        " JOIN issue_types AS t ON t.type_id = f.type_id";

    QString alertsQuery = "SELECT a.alert_id, f.folder_id, a.view_id, v.view_name, ac.total_count, ac.modified_count, ac.new_count, a.is_public"
        " FROM alerts AS a"
        " JOIN folders AS f ON f.folder_id = a.folder_id"
        " LEFT OUTER JOIN views AS v ON v.view_id = a.view_id"
        " LEFT OUTER JOIN alerts_cache AS ac ON ac.alert_id = a.alert_id";

    QSqlQuery sqlQuery;

    modelAt( AllProjects )->setQuery( allProjectsQuery );

    sqlQuery.prepare( QString( "%1 ORDER BY %2" ).arg( typesQuery, m_typesOrder ) );
    if ( dataManager->currentUserAccess() != AdminAccess )
        sqlQuery.addBindValue( dataManager->currentUserId() );
    sqlQuery.exec();

    modelAt( Types )->setQuery( sqlQuery );
    modelAt( GlobalAlerts )->setQuery( QString( "%1 ORDER BY %2" ).arg( globalAlertsQuery, m_alertsOrder ) );

    sqlQuery.prepare( QString( "%1 ORDER BY %2" ).arg( projectsQuery, m_projectsOrder ) );
    sqlQuery.addBindValue( dataManager->currentUserId() );
    sqlQuery.exec();

    modelAt( Projects )->setQuery( sqlQuery );

    modelAt( Folders )->setQuery( QString( "%1 ORDER BY %2" ).arg( foldersQuery, m_foldersOrder ) );
    modelAt( Alerts )->setQuery( QString( "%1 ORDER BY %2" ).arg( alertsQuery, m_alertsOrder ) );

    updateData();
}

void ProjectsModel::updateEvent( UpdateEvent* e )
{
    if ( e->unit() == UpdateEvent::Projects || e->unit() == UpdateEvent::Types || e->unit() == UpdateEvent::AlertStates || e->unit() == UpdateEvent::States || e->unit() == UpdateEvent::Summary )
        refresh();
}
