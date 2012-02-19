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

#include "alertsmodel.h"

#include "data/datamanager.h"
#include "utils/iconloader.h"

#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QPixmap>
#include <QFont>

AlertsModel::AlertsModel( int folderId, QObject* parent ) : BaseModel( parent ),
    m_folderId( folderId )
{
    appendModel( new QSqlQueryModel( this ) );

    bool emailEnabled = dataManager->setting( "email_enabled" ).toInt();

    QList<int> columnMapping;
    columnMapping << 2 << 3 << 4 << 5;
    if ( emailEnabled )
        columnMapping << 6;
    setColumnMapping( 0, columnMapping );

    setHeaderData( 0, Qt::Horizontal, tr( "Name" ) );
    setHeaderData( 1, Qt::Horizontal, tr( "Total" ) );
    setHeaderData( 2, Qt::Horizontal, tr( "Unread" ) );
    setHeaderData( 3, Qt::Horizontal, tr( "Modified" ) );
    if ( emailEnabled )
        setHeaderData( 4, Qt::Horizontal, tr( "Email Type" ) );

    refresh();
}

AlertsModel::~AlertsModel()
{
}

QVariant AlertsModel::data( const QModelIndex& index, int role /*= Qt::DisplayRole*/ ) const
{
    int level = levelOf( index );
    int row = mappedRow( index );

    if ( role == Qt::DisplayRole ) {
        QVariant value = rawData( level, row, mappedColumn( index ), role );

        if ( index.column() == 0 ) {
            int viewId = rawData( level, row, 1 ).toInt();
            return ( viewId != 0 ) ? value.toString() : tr( "All Issues" );
        }

        if ( index.column() == 4 ) {
            int email = value.toInt();
            if ( email == ImmediateNotificationEmail )
                return tr( "Immediate notifications" );
            if ( email == SummaryNotificationEmail )
                return tr( "Summary of notifications" );
            if ( email == SummaryReportEmail )
                return tr( "Summary reports" );
            return tr( "None" );
        }

        return value;
    }

    if ( role == Qt::DecorationRole && index.column() == 0 ) {
        int unread = rawData( level, row, 4 ).toInt();
        if ( unread > 0 )
            return IconLoader::pixmap( "alert-unread" );
        int modified = rawData( level, row, 5 ).toInt();
        if ( modified > 0 )
            return IconLoader::pixmap( "alert-modified" );
        return IconLoader::pixmap( "alert" );
    }

    if ( role == Qt::FontRole && index.column() >= 1 && index.column() <= 3 ) {
        int count = rawData( level, row, mappedColumn( index ), Qt::DisplayRole ).toInt();
        if ( count > 0 ) {
            QFont font;
            font.setBold( true );
            return font;
        }
    }

    return QVariant();
}

void AlertsModel::refresh()
{
    QString query = "SELECT a.alert_id, v.view_id, v.view_name, ac.total_count, ac.new_count, ac.modified_count, a.alert_email"
        " FROM alerts AS a"
        " LEFT OUTER JOIN views AS v ON v.view_id = a.view_id"
        " LEFT OUTER JOIN alerts_cache AS ac ON ac.alert_id = a.alert_id"
        " WHERE a.folder_id = ?"
        " ORDER BY v.view_name COLLATE LOCALE ASC";

    QSqlQuery sqlQuery;
    sqlQuery.prepare( query );
    sqlQuery.addBindValue( m_folderId );
    sqlQuery.exec();

    modelAt( 0 )->setQuery( sqlQuery );

    updateData();
}

void AlertsModel::updateEvent( UpdateEvent* e )
{
    if ( e->unit() == UpdateEvent::Projects || e->unit() == UpdateEvent::Types || e->unit() == UpdateEvent::AlertStates || e->unit() == UpdateEvent::States )
        refresh();
}
