/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2013 WebIssues Team
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

#include "foldermodel.h"

#include "data/datamanager.h"
#include "data/issuetypecache.h"
#include "models/querygenerator.h"
#include "models/issuedetailsgenerator.h"
#include "utils/formatter.h"
#include "utils/viewsettingshelper.h"
#include "utils/iconloader.h"

#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QDateTime>
#include <QTextDocument>
#include <QPixmap>

FolderModel::FolderModel( int folderId, QObject* parent ) : BaseModel( parent ),
    m_folderId( folderId ),
    m_viewId( 0 ),
    m_typeId( 0 ),
    m_forceColumns( false ),
    m_searchColumn( -1 )
{
    appendModel( new QSqlQueryModel( this ) );
}

FolderModel::~FolderModel()
{
}

void FolderModel::setView( int viewId, bool resort )
{
    m_viewId = viewId;
    m_forceColumns = false;

    generateQueries( resort );
}

void FolderModel::setColumns( const QList<int>& columns )
{
    m_viewId = 0;
    m_columns = columns;
    m_forceColumns = true;

    generateQueries( true );
}

void FolderModel::setSearchText( int column, const QString& text )
{
    m_searchColumn = column;
    m_searchText = text;

    generateQueries( false );
}

QVariant FolderModel::data( const QModelIndex& index, int role ) const
{
    int level = levelOf( index );
    int row = mappedRow( index );

    if ( role == Qt::DisplayRole ) {
        QVariant value = rawData( level, row, mappedColumn( index ), role );

        int column = m_columns.value( index.column() );

        switch ( column ) {
            case Column_ID:
                return QString( "#%1" ).arg( value.toInt() );
            case Column_Name:
            case Column_CreatedBy:
            case Column_ModifiedBy:
                return value;
            case Column_CreatedDate:
            case Column_ModifiedDate: {
                QDateTime dateTime;
                dateTime.setTime_t( value.toInt() );
                Formatter formatter;
                return formatter.formatDateTime( dateTime, true );
            }
            default:
                if ( !value.isNull() ) {
                    int column = m_columns.value( index.column() );
                    if ( column > Column_UserDefined ) {
                        Formatter formatter;
                        IssueTypeCache* cache = dataManager->issueTypeCache( m_typeId );
                        DefinitionInfo info = cache->attributeDefinition( column - Column_UserDefined );
                        return formatter.convertAttributeValue( info, value.toString(), false );
                    }
                }
        }
    }

    if ( role == Qt::DecorationRole && index.column() == 1 ) {
        QString name, overlay;
        int readId = rawData( level, row, 2, Qt::DisplayRole ).toInt();
        if ( readId == 0 ) {
            name = "issue-unread";
        } else {
            int stampId = rawData( level, row, 1, Qt::DisplayRole ).toInt();
            if ( readId < stampId )
                name = "issue-modified";
            else
                name = "issue";
        }
        bool emailEnabled = dataManager->setting( "email_enabled" ).toInt();
        if ( emailEnabled ) {
            int subscriptionId = rawData( level, row, 3, Qt::DisplayRole ).toInt();
            if ( subscriptionId != 0 )
                overlay = "overlay-subscribed";
        }
        if ( !overlay.isEmpty() )
            return IconLoader::overlayedPixmap( name, overlay );
        else
            return IconLoader::pixmap( name );
    }

    if ( role == Qt::FontRole ) {
        bool bold = false;
        int readId = rawData( level, row, 2, Qt::DisplayRole ).toInt();
        if ( readId == 0 ) {
            bold = true;
        } else {
            int stampId = rawData( level, row, 1, Qt::DisplayRole ).toInt();
            if ( readId < stampId )
                bold = true;
        }
        if ( bold ) {
            QFont font;
            font.setBold( true );
            return font;
        }
    }

    return QVariant();
}

void FolderModel::generateQueries( bool resort )
{
    QueryGenerator generator( m_folderId, m_viewId );

    if ( !m_searchText.isEmpty() )
        generator.setSearchText( m_searchColumn, m_searchText );

    if ( m_forceColumns )
        generator.setColumns( m_columns );

    m_typeId = generator.typeId();

    m_query = generator.query( true );
    m_arguments = generator.arguments();

    m_columns = generator.columns();
    m_sortColumns = generator.sortColumns();

    ViewSettingsHelper helper( m_typeId );
    for ( int i = 0; i < m_columns.count(); i++ )
        setHeaderData( i, Qt::Horizontal, helper.columnName( m_columns.at( i ) ) );

    setColumnMapping( 0, generator.columnMapping() );

    if ( resort )
        setSort( generator.sortColumn(), generator.sortOrder() );

    updateQueries();
}

void FolderModel::updateQueries()
{
    if ( !m_query.isEmpty() ) {
        QString column = m_sortColumns.value( sortColumn() );
        QString order = ( sortOrder() == Qt::AscendingOrder ) ? "ASC" : "DESC";

        m_order = QString( "%1 %2" ).arg( column, order );

        refresh();
    }
}

void FolderModel::refresh()
{
    if ( !m_query.isEmpty() ) {
        QSqlQuery sqlQuery;
        sqlQuery.prepare( QString( "%1 ORDER BY %2" ).arg( m_query, m_order ) );

        foreach ( const QVariant& arg, m_arguments )
            sqlQuery.addBindValue( arg );

        sqlQuery.exec();
            
        modelAt( 0 )->setQuery( sqlQuery );

        updateData();
    }
}

void FolderModel::updateEvent( UpdateEvent* e )
{
    switch ( e->unit() ) {
        case UpdateEvent::Folder:
        case UpdateEvent::IssueList:
            if ( e->id() == m_folderId )
                refresh();
            break;

        case UpdateEvent::Users:
        case UpdateEvent::States:
            refresh();
            break;

        default:
            break;
    }
}
