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

#include "tablemodels.h"
#include "issuedetailsgenerator.h"

#include "data/datamanager.h"
#include "data/datarows.h"
#include "data/updateevent.h"
#include "rdb/utilities.h"
#include "utils/definitioninfo.h"
#include "utils/attributehelper.h"
#include "utils/formatter.h"
#include "utils/datetimehelper.h"
#include "utils/tablemodelshelper.h"
#include "utils/viewsettingshelper.h"
#include "utils/textwriter.h"
#include "utils/iconloader.h"

#include <QPixmap>
#include <QTextDocument>

BaseTableModel::BaseTableModel( QObject* parent ) : RDB::AbstractTableModel( parent )
{
    dataManager->addObserver( this );
}

BaseTableModel::~BaseTableModel()
{
    if ( dataManager )
        dataManager->removeObserver( this );
}

QString BaseTableModel::columnName( int column ) const
{
    return TableModelsHelper::columnName( column );
}

void BaseTableModel::updateEvent( UpdateEvent* /*e*/ )
{
}

void BaseTableModel::customEvent( QEvent* e )
{
    if ( e->type() == UpdateEvent::Type )
        updateEvent( (UpdateEvent*)e );
}

UsersTableModel::UsersTableModel( QObject* parent ) : BaseTableModel( parent )
{
}

UsersTableModel::~UsersTableModel()
{
}

QString UsersTableModel::text( int id, int column ) const
{
    const UserRow* row = dataManager->users()->find( id );
    if ( !row )
        return QString();

    switch ( column ) {
        case Column_Name:
            return row->name();
        case Column_Login:
            return row->login();
        case Column_Access:
            if ( row->access() == NormalAccess )
                return tr( "Regular user" );
            if ( row->access() == AdminAccess )
                return tr( "System administrator" );
            return tr( "Disabled" );
        default:
            return QString();
    }
}

QPixmap UsersTableModel::icon( int id, int column ) const
{
    if ( column != Column_Name )
        return QPixmap();

    const UserRow* row = dataManager->users()->find( id );
    if ( row ) {
        if ( row->access() == AdminAccess )
            return IconLoader::overlayedPixmap( "user", "overlay-admin" );
        if ( row->access() == NoAccess )
            return IconLoader::pixmap( "user-disabled" );
    }
    return IconLoader::pixmap( "user" );
}

void UsersTableModel::updateEvent( UpdateEvent* e )
{
    if ( e->unit() == UpdateEvent::Users )
        emit dataChanged();
}

MembersTableModel::MembersTableModel( int projectId, QObject* parent ) : BaseTableModel( parent ),
    m_projectId( projectId )
{
}

MembersTableModel::~MembersTableModel()
{
}

QString MembersTableModel::text( int id, int column ) const
{
    const MemberRow* row = dataManager->members()->find( id, m_projectId );
    if ( !row )
        return QString();

    switch ( column ) {
        case Column_Name:
            return TableModelsHelper::userName( row->userId() );
        case Column_Access:
            if ( row->access() == NormalAccess )
                return tr( "Regular member" );
            if ( row->access() == AdminAccess )
                return tr( "Project administrator" );
            return QString();
        default:
            return QString();
    }
}

QPixmap MembersTableModel::icon( int id, int column ) const
{
    if ( column != Column_Name )
        return QPixmap();

    const MemberRow* row = dataManager->members()->find( id, m_projectId );
    if ( row && row->access() == AdminAccess )
        return IconLoader::overlayedPixmap( "user", "overlay-admin" );

    return IconLoader::pixmap( "user" );
}

void MembersTableModel::updateEvent( UpdateEvent* e )
{
    if ( e->unit() == UpdateEvent::Users )
        emit dataChanged();
}

TypesTableModel::TypesTableModel( QObject* parent ) : BaseTableModel( parent )
{
}

TypesTableModel::~TypesTableModel()
{
}

QString TypesTableModel::text( int id, int column ) const
{
    const TypeRow* row = dataManager->types()->find( id );
    if ( !row )
        return QString();

    switch ( column ) {
        case Column_Name:
            return row->name();
        default:
            return QString();
    }
}

QPixmap TypesTableModel::icon( int /*id*/, int column ) const
{
    if ( column != Column_Name )
        return QPixmap();
    return IconLoader::pixmap( "type" );
}

void TypesTableModel::updateEvent( UpdateEvent* e )
{
    if ( e->unit() == UpdateEvent::Types )
        emit dataChanged();
}

AttributesTableModel::AttributesTableModel( QObject* parent ) : BaseTableModel( parent )
{
}

AttributesTableModel::~AttributesTableModel()
{
}

QString AttributesTableModel::text( int id, int column ) const
{
    const AttributeRow* row = dataManager->attributes()->find( id );
    if ( !row )
        return QString();

    if ( column == Column_Name )
        return row->name();

    DefinitionInfo info = DefinitionInfo::fromString( row->definition() );

    switch ( column ) {
        case Column_Type:
            return AttributeHelper::typeName( AttributeHelper::toAttributeType( info ) );
        case Column_DefaultValue:
            return AttributeHelper::formatExpression( info, row->definition(), info.metadata( "default" ).toString() );
        case Column_Required:
            return info.metadata( "required" ).toBool() ? tr( "Yes" ) : tr( "No" );
        case Column_Details:
            return AttributeHelper::metadataDetails( info );
        default:
            return QString();
    }
}

QPixmap AttributesTableModel::icon( int /*id*/, int column ) const
{
    if ( column != Column_Name )
        return QPixmap();
    return IconLoader::pixmap( "attribute" );
}

void AttributesTableModel::updateEvent( UpdateEvent* /*e*/ )
{
}

ProjectsTableModel::ProjectsTableModel( QObject* parent ) : BaseTableModel( parent )
{
}

ProjectsTableModel::~ProjectsTableModel()
{
}

QString ProjectsTableModel::text( int id, int column ) const
{
    const ProjectRow* row = dataManager->projects()->find( id );
    if ( !row )
        return QString();

    switch ( column ) {
        case Column_Name:
            return row->name();
        default:
            return QString();
    }
}

QPixmap ProjectsTableModel::icon( int id, int column ) const
{
    if ( column != Column_Name )
        return QPixmap();

    if ( TableModelsHelper::isProjectAdmin( id ) )
        return IconLoader::overlayedPixmap( "project", "overlay-admin" );

    return IconLoader::pixmap( "project" );
}

void ProjectsTableModel::updateEvent( UpdateEvent* /*e*/ )
{
}

FoldersTableModel::FoldersTableModel( QObject* parent ) : BaseTableModel( parent )
{
}

FoldersTableModel::~FoldersTableModel()
{
}

QString FoldersTableModel::text( int id, int column ) const
{
    const FolderRow* row = dataManager->folders()->find( id );
    if ( !row )
        return QString();

    switch ( column ) {
        case Column_Name:
            return row->name();
        case Column_Type:
            return TableModelsHelper::typeName( row->typeId() );
        default:
            return QString();
    }
}

QPixmap FoldersTableModel::icon( int /*id*/, int column ) const
{
    if ( column != Column_Name )
        return QPixmap();

    return IconLoader::pixmap( "folder" );
}

void FoldersTableModel::updateEvent( UpdateEvent* e )
{
    if ( e->unit() == UpdateEvent::Types )
        emit dataChanged();
}

IssuesTableModel::IssuesTableModel( int folderId, QObject* parent ) : BaseTableModel( parent ),
    m_folderId( folderId )
{
}

IssuesTableModel::~IssuesTableModel()
{
}

QString IssuesTableModel::text( int id, int column ) const
{
    const IssueRow* row = dataManager->issues()->find( id );
    if ( !row )
        return QString();

    return TableModelsHelper::extractValue( row, column );
}

QPixmap IssuesTableModel::icon( int id, int column ) const
{
    if ( column != Column_Name )
        return QPixmap();

    const IssueRow* row = dataManager->issues()->find( id );
    if ( row ) {
        int read = dataManager->issueReadStamp( id );
        if ( read == 0 )
            return IconLoader::pixmap( "issue-unread" );
        if ( read < row->stamp() )
            return IconLoader::pixmap( "issue-modified" );
    }
    return IconLoader::pixmap( "issue" );
}

QString IssuesTableModel::toolTip( int id, int /*column*/ ) const
{
    QTextDocument document;

    TextWriter writer( &document, TextWriter::UseSmallHeaders );

    IssueDetailsGenerator generator;
    generator.setIssue( id, IssueDetailsGenerator::NoHistory );
    generator.write( &writer );

    return document.toHtml( "UTF-8" );
}

int IssuesTableModel::compare( int id1, int id2, int column ) const
{
    if ( column == Column_ID )
        return id1 - id2;

    if ( column == Column_CreatedDate || column == Column_ModifiedDate ) {
        const IssueRow* row1 = dataManager->issues()->find( id1 );
        const IssueRow* row2 = dataManager->issues()->find( id2 );

        if ( !row1 || !row2 )
            return 0;

        if ( column == Column_CreatedDate )
            return DateTimeHelper::compareDateTime( row1->createdDate(), row2->createdDate() );
        if ( column == Column_ModifiedDate )
            return DateTimeHelper::compareDateTime( row1->modifiedDate(), row2->modifiedDate() );
    }

    return AbstractTableModel::compare( id1, id2, column );
}

void IssuesTableModel::updateEvent( UpdateEvent* e )
{
    if ( e->unit() == UpdateEvent::Folder && e->id() == m_folderId )
        emit dataChanged();

    if ( e->unit() == UpdateEvent::IssueList && e->id() == m_folderId )
        emit dataChanged();

    if ( e->unit() == UpdateEvent::Users )
        emit dataChanged();

    if ( e->unit() == UpdateEvent::States )
        emit dataChanged();
}

ViewsTableModel::ViewsTableModel( int typeId, QObject* parent ) : BaseTableModel( parent ),
    m_typeId( typeId )
{
}

ViewsTableModel::~ViewsTableModel()
{
}

QString ViewsTableModel::text( int id, int column ) const
{
    const ViewRow* row = dataManager->views()->find( id );
    if ( !row )
        return QString();

    if ( column == Column_Name )
        return row->name();

    DefinitionInfo info = DefinitionInfo::fromString( row->definition() );

    switch ( column ) {
        case Column_Columns:
            return ViewSettingsHelper::columnNames( ViewSettingsHelper::viewColumns( m_typeId, info ) );
        case Column_SortBy:
            return ViewSettingsHelper::sortOrderInfo( ViewSettingsHelper::viewSortOrder( m_typeId, info ) );
        case Column_Filter:
            return ViewSettingsHelper::filtersInfo( ViewSettingsHelper::viewFilters( m_typeId, info ) );
        default:
            return QString();
    }
}

QPixmap ViewsTableModel::icon( int /*id*/, int column ) const
{
    if ( column != Column_Name )
        return QPixmap();

    return IconLoader::pixmap( "view" );
}

void ViewsTableModel::updateEvent( UpdateEvent* e )
{
    if ( e->unit() == UpdateEvent::Types )
        emit dataChanged();
}

AlertsTableModel::AlertsTableModel( bool treeMode, QObject* parent ) : BaseTableModel( parent ),
    m_treeMode( treeMode )
{
}

AlertsTableModel::~AlertsTableModel()
{
}

QString AlertsTableModel::text( int id, int column ) const
{
    const AlertRow* row = dataManager->alerts()->find( id );
    if ( !row )
        return QString();

    switch ( column ) {
        case Column_Name:
            if ( m_treeMode ) {
                int unread, modified, total;
                dataManager->getAlertIssuesCount( id, &unread, &modified, &total );
                QString info;
                if ( unread > 0 )
                    info = QString( " (%1+%2)" ).arg( unread ).arg( modified );
                else if ( modified > 0 )
                    info = QString( " (%1)" ).arg( modified );
                else if ( total > 0 )
                    info = QString( " [%1]" ).arg( total );
                return TableModelsHelper::viewName( row->viewId() ) + info;
            } else {
                return TableModelsHelper::viewName( row->viewId() );
            }
        case Column_Status: {
            int unread, modified, total;
            dataManager->getAlertIssuesCount( id, &unread, &modified, &total );
            return tr( "%1 new, %2 modified, %3 total issues" ).arg( unread ).arg( modified ).arg( total );
        }
        case Column_EmailType:
            if ( row->alertEmail() == ImmediateNotificationEmail )
                return tr( "Immediate Notifications" );
            if ( row->alertEmail() == SummaryNotificationEmail )
                return tr( "Summary Notifications" );
            if ( row->alertEmail() == SummaryReportEmail )
                return tr( "Summary Reports" );
            return tr( "None" );
        default:
            return QString();
    }
}

QPixmap AlertsTableModel::icon( int id, int column ) const
{
    if ( column != Column_Name )
        return QPixmap();

    int unread, modified, total;
    dataManager->getAlertIssuesCount( id, &unread, &modified, &total );

    if ( unread > 0 )
        return IconLoader::pixmap( "alert-unread" );
    if ( modified > 0 )
        return IconLoader::pixmap( "alert-modified" );
    return IconLoader::pixmap( "alert" );
}

int AlertsTableModel::compare( int id1, int id2, int column ) const
{
    if ( column == Column_Name ) {
        const AlertRow* row1 = dataManager->alerts()->find( id1 );
        const AlertRow* row2 = dataManager->alerts()->find( id2 );

        if ( !row1 || !row2 )
            return 0;

        if ( row1->viewId() == 0 && row2->viewId() != 0 )
            return -1;
        if ( row1->viewId() != 0 && row2->viewId() == 0 )
            return 1;
    }

    return AbstractTableModel::compare( id1, id2, column );
}

void AlertsTableModel::updateEvent( UpdateEvent* e )
{
    if ( e->unit() == UpdateEvent::AlertStates || e->unit() == UpdateEvent::Projects )
        emit dataChanged();
}
