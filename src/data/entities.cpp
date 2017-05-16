/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2017 WebIssues Team
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

#include "entities.h"
#include "entities_p.h"

#include "data/issuetypecache.h"
#include "data/query.h"

#include <QVector>

ProjectEntity::ProjectEntity() :
    d( new ProjectEntityData() )
{
}

ProjectEntity::~ProjectEntity()
{
}

ProjectEntity::ProjectEntity( const ProjectEntity& other ) :
    d( other.d )
{
}

ProjectEntity& ProjectEntity::operator =( const ProjectEntity& other )
{
    d = other.d;
    return *this;
}

ProjectEntityData::ProjectEntityData() :
    m_id( 0 ),
    m_isPublic( false )
{
}

ProjectEntityData::~ProjectEntityData()
{
}

bool ProjectEntity::isValid() const
{
    return d->m_id != 0;
}

int ProjectEntity::id() const
{
    return d->m_id;
}

const QString& ProjectEntity::name() const
{
    return d->m_name;
}

bool ProjectEntity::isPublic() const
{
    return d->m_isPublic;
}

ProjectEntity ProjectEntity::find( int id )
{
    ProjectEntity entity;

    if ( id != 0 ) {
        Query query( "SELECT project_id, project_name, is_public FROM projects WHERE project_id = ?" );
        query.exec( id );

        if ( query.next() )
            entity.d->read( query );
    }

    return entity;
}

QList<ProjectEntity> ProjectEntity::list()
{
    QList<ProjectEntity> result;

    Query query( "SELECT project_id, project_name, is_public FROM projects ORDER BY project_name COLLATE LOCALE" );
    query.exec();

    while ( query.next() ) {
        ProjectEntity entity;
        entity.d->read( query );
        result.append( entity );
    }

    return result;
}

QList<ProjectEntity> UserEntity::projects() const
{
    QList<ProjectEntity> result;

    if ( d->m_id != 0 ) {
        Query query( "SELECT p.project_id, p.project_name, p.is_public"
            " FROM projects AS p"
            " JOIN rights AS r ON r.project_id = p.project_id"
            " WHERE r.user_id = ?"
            " ORDER BY p.project_name COLLATE LOCALE" );
        query.exec( d->m_id );

        while ( query.next() ) {
            ProjectEntity entity;
            entity.d->read( query );
            result.append( entity );
        }
    }

    return result;
}

void ProjectEntityData::read( const Query& query )
{
    m_id = query.value( 0 ).toInt();
    m_name = query.value( 1 ).toString();
    m_isPublic = query.value( 2 ).toBool();
}

bool ProjectEntity::isAdmin( int id )
{
    if ( dataManager->currentUserAccess() == AdminAccess )
        return true;

    Query query( "SELECT project_access FROM rights WHERE project_id = ? AND user_id = ?" );
    query.exec( id, dataManager->currentUserId() );

    if ( query.readScalar().toInt() == AdminAccess )
        return true;

    return false;
}

bool ProjectEntity::exists( const QString& name )
{
    Query query( "SELECT project_id FROM projects WHERE project_name = ?" );
    query.exec( name );

    if ( query.next() )
        return true;

    return false;
}

FolderEntity::FolderEntity() :
    d( new FolderEntityData() )
{
}

FolderEntity::~FolderEntity()
{
}

FolderEntity::FolderEntity( const FolderEntity& other ) :
    d( other.d )
{
}

FolderEntity& FolderEntity::operator =( const FolderEntity& other )
{
    d = other.d;
    return *this;
}

FolderEntityData::FolderEntityData() :
    m_id( 0 ),
    m_projectId( 0 ),
    m_typeId( 0 ),
    m_stampId( 0 )
{
}

FolderEntityData::~FolderEntityData()
{
}

bool FolderEntity::isValid() const
{
    return d->m_id != 0;
}

int FolderEntity::id() const
{
    return d->m_id;
}

int FolderEntity::projectId() const
{
    return d->m_projectId;
}

int FolderEntity::typeId() const
{
    return d->m_typeId;
}

const QString& FolderEntity::name() const
{
    return d->m_name;
}

int FolderEntity::stampId() const
{
    return d->m_stampId;
}

ProjectEntity FolderEntity::project() const
{
    return ProjectEntity::find( d->m_projectId );
}

TypeEntity FolderEntity::type() const
{
    return TypeEntity::find( d->m_typeId );
}

FolderEntity FolderEntity::find( int id )
{
    FolderEntity entity;

    if ( id != 0 ) {
        Query query( "SELECT folder_id, project_id, type_id, folder_name, stamp_id"
            " FROM folders"
            " WHERE folder_id = ?" );
        query.exec( id );

        if ( query.next() )
            entity.d->read( query );
    }

    return entity;
}

QList<FolderEntity> ProjectEntity::folders() const
{
    QList<FolderEntity> result;

    if ( d->m_id != 0 ) {
        Query query( "SELECT folder_id, project_id, type_id, folder_name, stamp_id"
            " FROM folders"
            " WHERE project_id = ?"
            " ORDER BY folder_name COLLATE LOCALE" );
        query.exec( d->m_id );

        while ( query.next() ) {
            FolderEntity entity;
            entity.d->read( query );
            result.append( entity );
        }
    }

    return result;
}

QList<FolderEntity> TypeEntity::folders() const
{
    QList<FolderEntity> result;

    if ( d->m_id != 0 ) {
        Query query( "SELECT folder_id, project_id, type_id, folder_name, stamp_id"
            " FROM folders"
            " WHERE type_id = ?" );
        query.exec( d->m_id );

        while ( query.next() ) {
            FolderEntity entity;
            entity.d->read( query );
            result.append( entity );
        }
    }

    return result;
}

QList<FolderEntity> FolderEntity::list()
{
    QList<FolderEntity> result;

    Query query( "SELECT folder_id, project_id, type_id, folder_name, stamp_id FROM folders" );
    query.exec();

    while ( query.next() ) {
        FolderEntity entity;
        entity.d->read( query );
        result.append( entity );
    }

    return result;
}

void FolderEntityData::read( const Query& query )
{
    m_id = query.value( 0 ).toInt();
    m_projectId = query.value( 1 ).toInt();
    m_typeId = query.value( 2 ).toInt();
    m_name = query.value( 3 ).toString();
    m_stampId = query.value( 4 ).toInt();
}

bool FolderEntity::isAdmin( int id )
{
    if ( dataManager->currentUserAccess() == AdminAccess )
        return true;

    Query query( "SELECT project_access"
        " FROM rights AS r"
        " JOIN folders AS f ON f.project_id = r.project_id"
        " WHERE f.folder_id = ? AND r.user_id = ?" );
    query.exec( id, dataManager->currentUserId() );

    if ( query.readScalar().toInt() == AdminAccess )
        return true;

    return false;
}

bool FolderEntity::exists( int projectId, const QString& name )
{
    Query query( "SELECT folder_id FROM folders WHERE project_id = ? AND folder_name = ?" );
    query.exec( projectId, name );

    if ( query.next() )
        return true;

    return false;
}

TypeEntity::TypeEntity() :
    d( new TypeEntityData() )
{
}

TypeEntity::~TypeEntity()
{
}

TypeEntity::TypeEntity( const TypeEntity& other ) :
    d( other.d )
{
}

TypeEntity& TypeEntity::operator =( const TypeEntity& other )
{
    d = other.d;
    return *this;
}

TypeEntityData::TypeEntityData() :
    m_id( 0 )
{
}

TypeEntityData::~TypeEntityData()
{
}

bool TypeEntity::isValid() const
{
    return d->m_id != 0;
}

int TypeEntity::id() const
{
    return d->m_id;
}

const QString& TypeEntity::name() const
{
    return d->m_name;
}

TypeEntity TypeEntity::find( int id )
{
    TypeEntity entity;

    if ( id != 0 ) {
        Query query( "SELECT type_id, type_name FROM issue_types WHERE type_id = ?" );
        query.exec( id );

        if ( query.next() )
            entity.d->read( query );
    }

    return entity;
}

QList<TypeEntity> TypeEntity::list()
{
    QList<TypeEntity> result;

    Query query( "SELECT type_id, type_name FROM issue_types ORDER BY type_name COLLATE LOCALE" );
    query.exec();

    while ( query.next() ) {
        TypeEntity entity;
        entity.d->read( query );
        result.append( entity );
    }

    return result;
}

void TypeEntityData::read( const Query& query )
{
    m_id = query.value( 0 ).toInt();
    m_name = query.value( 1 ).toString();
}

bool TypeEntity::exists( const QString& name )
{
    Query query( "SELECT type_id FROM issue_types WHERE type_name = ?" );
    query.exec( name );

    if ( query.next() )
        return true;

    return false;
}

AttributeEntity::AttributeEntity() :
    d( new AttributeEntityData() )
{
}

AttributeEntity::~AttributeEntity()
{
}

AttributeEntity::AttributeEntity( const AttributeEntity& other ) :
    d( other.d )
{
}

AttributeEntity& AttributeEntity::operator =( const AttributeEntity& other )
{
    d = other.d;
    return *this;
}

AttributeEntityData::AttributeEntityData() :
    m_id( 0 ),
    m_typeId( 0 )
{
}

AttributeEntityData::~AttributeEntityData()
{
}

bool AttributeEntity::isValid() const
{
    return d->m_id != 0;
}

int AttributeEntity::id() const
{
    return d->m_id;
}

int AttributeEntity::typeId() const
{
    return d->m_typeId;
}

const QString& AttributeEntity::name() const
{
    return d->m_name;
}

TypeEntity AttributeEntity::type() const
{
    return TypeEntity::find( d->m_typeId );
}

AttributeEntity AttributeEntity::find( int id )
{
    AttributeEntity entity;

    if ( id != 0 ) {
        Query query( "SELECT attr_id, type_id, attr_name"
            " FROM attr_types"
            " WHERE attr_id = ?" );
        query.exec( id );

        if ( query.next() )
            entity.d->read( query );
    }

    return entity;
}

QList<AttributeEntity> TypeEntity::attributes() const
{
    QList<AttributeEntity> result;

    if ( d->m_id != 0 ) {
        Query query( "SELECT attr_id, type_id, attr_name"
            " FROM attr_types"
            " WHERE type_id = ?"
            " ORDER BY attr_name COLLATE LOCALE" );
        query.exec( d->m_id );

        while ( query.next() ) {
            AttributeEntity entity;
            entity.d->read( query );
            result.append( entity );
        }
    }

    return result;
}

void AttributeEntityData::read( const Query& query )
{
    m_id = query.value( 0 ).toInt();
    m_typeId = query.value( 1 ).toInt();
    m_name = query.value( 2 ).toString();
}

bool AttributeEntity::exists( int typeId, const QString& name )
{
    Query query( "SELECT attr_id FROM attr_types WHERE type_id = ? AND attr_name = ?" );
    query.exec( typeId, name );

    if ( query.next() )
        return true;

    return false;
}

ViewEntity::ViewEntity() :
    d( new ViewEntityData() )
{
}

ViewEntity::~ViewEntity()
{
}

ViewEntity::ViewEntity( const ViewEntity& other ) :
    d( other.d )
{
}

ViewEntity& ViewEntity::operator =( const ViewEntity& other )
{
    d = other.d;
    return *this;
}

ViewEntityData::ViewEntityData() :
    m_id( 0 ),
    m_typeId( 0 ),
    m_isPublic( false )
{
}

ViewEntityData::~ViewEntityData()
{
}

bool ViewEntity::isValid() const
{
    return d->m_id != 0;
}

int ViewEntity::id() const
{
    return d->m_id;
}

int ViewEntity::typeId() const
{
    return d->m_typeId;
}

const QString& ViewEntity::name() const
{
    return d->m_name;
}

const DefinitionInfo& ViewEntity::definition() const
{
    return d->m_definition;
}

bool ViewEntity::isPublic() const
{
    return d->m_isPublic;
}

TypeEntity ViewEntity::type() const
{
    return TypeEntity::find( d->m_typeId );
}

ViewEntity ViewEntity::find( int id )
{
    ViewEntity entity;

    if ( id != 0 ) {
        Query query( "SELECT view_id, type_id, view_name, view_def, is_public"
            " FROM views"
            " WHERE view_id = ?" );
        query.exec( id );

        if ( query.next() )
            entity.d->read( query );
    }

    return entity;
}

QList<ViewEntity> TypeEntity::views() const
{
    QList<ViewEntity> result;

    if ( d->m_id != 0 ) {
        Query query( "SELECT view_id, type_id, view_name, view_def, is_public"
            " FROM views"
            " WHERE type_id = ?"
            " ORDER BY view_name COLLATE LOCALE" );
        query.exec( d->m_id );

        while ( query.next() ) {
            ViewEntity entity;
            entity.d->read( query );
            result.append( entity );
        }
    }

    return result;
}

void ViewEntityData::read( const Query& query )
{
    m_id = query.value( 0 ).toInt();
    m_typeId = query.value( 1 ).toInt();
    m_name = query.value( 2 ).toString();
    m_definition = DefinitionInfo::fromString( query.value( 3 ).toString() );
    m_isPublic = query.value( 4 ).toBool();
}

bool ViewEntity::exists( int typeId, const QString& name, bool isPublic )
{
    Query query( "SELECT view_id FROM views WHERE type_id = ? AND view_name = ? AND is_public = ?" );
    query.exec( typeId, name, isPublic ? 1 : 0 );

    if ( query.next() )
        return true;

    return false;
}

AlertEntity::AlertEntity() :
    d( new AlertEntityData() )
{
}

AlertEntity::~AlertEntity()
{
}

AlertEntity::AlertEntity( const AlertEntity& other ) :
    d( other.d )
{
}

AlertEntity& AlertEntity::operator =( const AlertEntity& other )
{
    d = other.d;
    return *this;
}

AlertEntityData::AlertEntityData() :
    m_id( 0 ),
    m_folderId( 0 ),
    m_typeId( 0 ),
    m_viewId( 0 ),
    m_alertEmail( NoEmail ),
    m_isPublic( false )
{
}

AlertEntityData::~AlertEntityData()
{
}

bool AlertEntity::isValid() const
{
    return d->m_id != 0;
}

int AlertEntity::id() const
{
    return d->m_id;
}

int AlertEntity::viewId() const
{
    return d->m_viewId;
}

AlertEmail AlertEntity::alertEmail() const
{
    return d->m_alertEmail;
}

const QString& AlertEntity::summaryDays() const
{
    return d->m_summaryDays;
}

const QString& AlertEntity::summaryHours() const
{
    return d->m_summaryHours;
}

bool AlertEntity::isPublic() const
{
    return d->m_isPublic;
}

FolderEntity AlertEntity::folder() const
{
    return FolderEntity::find( d->m_folderId );
}

TypeEntity AlertEntity::type() const
{
    return TypeEntity::find( d->m_typeId );
}

ViewEntity AlertEntity::view() const
{
    return ViewEntity::find( d->m_viewId );
}

AlertEntity AlertEntity::find( int id )
{
    AlertEntity entity;

    if ( id != 0 ) {
        Query query( "SELECT alert_id, folder_id, type_id, view_id, alert_email, summary_days, summary_hours, is_public"
            " FROM alerts"
            " WHERE alert_id = ?" );
        query.exec( id );

        if ( query.next() )
            entity.d->read( query );
    }

    return entity;
}

QList<AlertEntity> FolderEntity::alerts() const
{
    QList<AlertEntity> result;

    if ( d->m_id != 0 ) {
        Query query( "SELECT alert_id, folder_id, type_id, view_id, alert_email, summary_days, summary_hours, is_public"
            " FROM alerts"
            " WHERE folder_id = ?" );
        query.exec( d->m_id );

        while ( query.next() ) {
            AlertEntity entity;
            entity.d->read( query );
            result.append( entity );
        }
    }

    return result;
}

QList<AlertEntity> TypeEntity::alerts() const
{
    QList<AlertEntity> result;

    if ( d->m_id != 0 ) {
        Query query( "SELECT alert_id, folder_id, type_id, view_id, alert_email, summary_days, summary_hours, is_public"
            " FROM alerts"
            " WHERE type_id = ?" );
        query.exec( d->m_id );

        while ( query.next() ) {
            AlertEntity entity;
            entity.d->read( query );
            result.append( entity );
        }
    }

    return result;
}

void AlertEntityData::read( const Query& query )
{
    m_id = query.value( 0 ).toInt();
    m_folderId = query.value( 1 ).toInt();
    m_typeId = query.value( 2 ).toInt();
    m_viewId = query.value( 3 ).toInt();
    m_alertEmail = (AlertEmail)query.value( 4 ).toInt();
    m_summaryDays = query.value( 5 ).toString();
    m_summaryHours = query.value( 6 ).toString();
    m_isPublic = query.value( 7 ).toBool();
}

UserEntity::UserEntity() :
    d( new UserEntityData() )
{
}

UserEntity::~UserEntity()
{
}

UserEntity::UserEntity( const UserEntity& other ) :
    d( other.d )
{
}

UserEntity& UserEntity::operator =( const UserEntity& other )
{
    d = other.d;
    return *this;
}

UserEntityData::UserEntityData() :
    m_id( 0 ),
    m_access( NoAccess )
{
}

UserEntityData::~UserEntityData()
{
}

bool UserEntity::isValid() const
{
    return d->m_id != 0;
}

int UserEntity::id() const
{
    return d->m_id;
}

const QString& UserEntity::login() const
{
    return d->m_login;
}

const QString& UserEntity::name() const
{
    return d->m_name;
}

Access UserEntity::access() const
{
    return d->m_access;
}

UserEntity UserEntity::find( int id )
{
    UserEntity entity;

    if ( id != 0 ) {
        Query query( "SELECT user_id, user_login, user_name, user_access"
            " FROM users"
            " WHERE user_id = ?" );
        query.exec( id );

        if ( query.next() )
            entity.d->read( query );
    }

    return entity;
}

QList<UserEntity> UserEntity::list()
{
    QList<UserEntity> result;

    Query query( "SELECT user_id, user_login, user_name, user_access"
        " FROM users"
        " ORDER BY user_name COLLATE LOCALE" );
    query.exec();

    while ( query.next() ) {
        UserEntity entity;
        entity.d->read( query );
        result.append( entity );
    }

    return result;
}

QList<UserEntity> UserEntity::visible()
{
    QList<UserEntity> result;

    Query query( "SELECT u.user_id, u.user_login, u.user_name, u.user_access"
        " FROM users AS u"
        " WHERE u.user_id IN ("
        " SELECT r1.user_id FROM rights AS r1"
        " INNER JOIN effective_rights AS r2 ON r2.project_id = r1.project_id"
        " WHERE r2.user_id = ? )"
        " ORDER BY u.user_name COLLATE LOCALE" );
    query.exec( dataManager->currentUserId() );

    while ( query.next() ) {
        UserEntity entity;
        entity.d->read( query );
        result.append( entity );
    }

    return result;
}

QList<UserEntity> ProjectEntity::members() const
{
    QList<UserEntity> result;

    if ( d->m_id != 0 ) {
        Query query( "SELECT u.user_id, u.user_login, u.user_name, u.user_access"
            " FROM users AS u"
            " JOIN rights AS r ON r.user_id = u.user_id"
            " WHERE r.project_id = ?"
            " ORDER BY u.user_name COLLATE LOCALE" );
        query.exec( d->m_id );

        while ( query.next() ) {
            UserEntity entity;
            entity.d->read( query );
            result.append( entity );
        }
    }

    return result;
}

void UserEntityData::read( const Query& query )
{
    m_id = query.value( 0 ).toInt();
    m_login = query.value( 1 ).toString();
    m_name = query.value( 2 ).toString();
    m_access = (Access)query.value( 3 ).toInt();
}

bool UserEntity::exists( const QString& login, const QString& name )
{
    Query query( "SELECT user_id FROM users WHERE user_login = ? OR user_name = ?" );
    query.exec( login, name );

    if ( query.next() )
        return true;

    return false;
}

MemberEntity::MemberEntity() :
    d( new MemberEntityData() )
{
}

MemberEntity::~MemberEntity()
{
}

MemberEntity::MemberEntity( const MemberEntity& other ) :
    d( other.d )
{
}

MemberEntity& MemberEntity::operator =( const MemberEntity& other )
{
    d = other.d;
    return *this;
}

MemberEntityData::MemberEntityData() :
    m_id( 0 ),
    m_projectId( 0 ),
    m_access( NoAccess )
{
}

MemberEntityData::~MemberEntityData()
{
}

bool MemberEntity::isValid() const
{
    return d->m_id != 0;
}

int MemberEntity::id() const
{
    return d->m_id;
}

int MemberEntity::projectId() const
{
    return d->m_id;
}

const QString& MemberEntity::login() const
{
    return d->m_login;
}

const QString& MemberEntity::name() const
{
    return d->m_name;
}

Access MemberEntity::access() const
{
    return d->m_access;
}

MemberEntity MemberEntity::find( int projectId, int userId )
{
    MemberEntity entity;

    if ( projectId != 0 && userId != 0 ) {
        Query query( "SELECT r.user_id, r.project_id, u.user_login, u.user_name, r.project_access"
            " FROM users AS u"
            " JOIN rights AS r ON r.user_id = u.user_id"
            " WHERE u.user_id = ? AND r.project_id = ?" );
        query.exec( userId, projectId );

        if ( query.next() )
            entity.d->read( query );
    }

    return entity;
}

MemberEntity MemberEntity::findEffective( int projectId, int userId )
{
    MemberEntity entity;

    if ( projectId != 0 && userId != 0 ) {
        Query query( "SELECT r.user_id, r.project_id, u.user_login, u.user_name, r.project_access"
            " FROM users AS u"
            " JOIN effective_rights AS r ON r.user_id = u.user_id"
            " WHERE u.user_id = ? AND r.project_id = ?" );
        query.exec( userId, projectId );

        if ( query.next() )
            entity.d->read( query );
    }

    return entity;
}

QList<MemberEntity> MemberEntity::list( int userId )
{
    QList<MemberEntity> result;

    if ( userId != 0 ) {
        Query query( "SELECT r.user_id, r.project_id, u.user_login, u.user_name, r.project_access"
            " FROM users AS u"
            " JOIN rights AS r ON r.user_id = u.user_id"
            " WHERE r.user_id = ?" );
        query.exec( userId );

        while ( query.next() ) {
            MemberEntity entity;
            entity.d->read( query );
            result.append( entity );
        }
    }

    return result;
}

void MemberEntityData::read( const Query& query )
{
    m_id = query.value( 0 ).toInt();
    m_projectId = query.value( 1 ).toInt();
    m_login = query.value( 2 ).toString();
    m_name = query.value( 3 ).toString();
    m_access = (Access)query.value( 4 ).toInt();
}

IssueEntity::IssueEntity() :
    d( new IssueEntityData() )
{
}

IssueEntity::~IssueEntity()
{
}

IssueEntity::IssueEntity( const IssueEntity& other ) :
    d( other.d )
{
}

IssueEntity& IssueEntity::operator =( const IssueEntity& other )
{
    d = other.d;
    return *this;
}

IssueEntityData::IssueEntityData() :
    m_id( 0 ),
    m_stampId( 0 ),
    m_readId( 0 ),
    m_subscriptionId( 0 ),
    m_folderId( 0 ),
    m_typeId( 0 ),
    m_createdUserId( 0 )
{
}

IssueEntityData::~IssueEntityData()
{
}

bool IssueEntity::isValid() const
{
    return d->m_id != 0;
}

int IssueEntity::id() const
{
    return d->m_id;
}

const QString& IssueEntity::name() const
{
    return d->m_name;
}

int IssueEntity::stampId() const
{
    return d->m_stampId;
}

int IssueEntity::readId() const
{
    return d->m_readId;
}

int IssueEntity::subscriptionId() const
{
    return d->m_subscriptionId;
}

int IssueEntity::folderId() const
{
    return d->m_folderId;
}

FolderEntity IssueEntity::folder() const
{
    return FolderEntity::find( d->m_folderId );
}

const QDateTime& IssueEntity::createdDate() const
{
    return d->m_createdDate;
}

const QString& IssueEntity::createdUser() const
{
    return d->m_createdUser;
}

int IssueEntity::createdUserId() const
{
    return d->m_createdUserId;
}

const QDateTime& IssueEntity::modifiedDate() const
{
    return d->m_modifiedDate;
}

const QString& IssueEntity::modifiedUser() const
{
    return d->m_modifiedUser;
}

IssueEntity IssueEntity::find( int id )
{
    IssueEntity entity;

    if ( id != 0 ) {
        Query query( "SELECT i.issue_id, i.stamp_id, s.read_id, s.subscription_id, i.folder_id, f.type_id, i.issue_name,"
            " i.created_time, uc.user_name AS created_user, i.created_user_id, i.modified_time, um.user_name AS modified_user"
            " FROM issues AS i"
            " JOIN folders AS f ON f.folder_id = i.folder_id"
            " LEFT OUTER JOIN issue_states AS s ON s.issue_id = i.issue_id AND s.user_id = ?"
            " LEFT OUTER JOIN users AS uc ON uc.user_id = i.created_user_id"
            " LEFT OUTER JOIN users AS um ON um.user_id = i.modified_user_id"
            " WHERE i.issue_id = ?" );
        query.exec( dataManager->currentUserId(), id );

        if ( query.next() )
            entity.d->read( query );
    }

    return entity;
}

QList<IssueEntity> FolderEntity::issues() const
{
    QList<IssueEntity> result;

    if ( d->m_id != 0 ) {
        Query query( "SELECT i.issue_id, i.stamp_id, s.read_id, s.subscription_id, i.folder_id, f.type_id, i.issue_name,"
            " i.created_time, uc.user_name AS created_user, i.created_user_id, i.modified_time, um.user_name AS modified_user"
            " FROM issues AS i"
            " JOIN folders AS f ON f.folder_id = i.folder_id"
            " LEFT OUTER JOIN issue_states AS s ON s.issue_id = i.issue_id AND s.user_id = ?"
            " LEFT OUTER JOIN users AS uc ON uc.user_id = i.created_user_id"
            " LEFT OUTER JOIN users AS um ON um.user_id = i.modified_user_id"
            " WHERE i.folder_id = ?" );
        query.exec( dataManager->currentUserId(), d->m_id );

        while ( query.next() ) {
            IssueEntity entity;
            entity.d->read( query );
            result.append( entity );
        }
    }

    return result;
}

void IssueEntityData::read( const Query& query )
{
    m_id = query.value( 0 ).toInt();
    m_stampId = query.value( 1 ).toInt();
    m_readId = query.value( 2 ).toInt();
    m_subscriptionId = query.value( 3 ).toInt();
    m_folderId = query.value( 4 ).toInt();
    m_typeId = query.value( 5 ).toInt();
    m_name = query.value( 6 ).toString();
    m_createdDate.setTime_t( query.value( 7 ).toInt() );
    m_createdUser = query.value( 8 ).toString();
    m_createdUserId = query.value( 9 ).toInt();
    m_modifiedDate.setTime_t( query.value( 10 ).toInt() );
    m_modifiedUser = query.value( 11 ).toString();
}

bool IssueEntity::isOwner( int id )
{
    Query query( "SELECT created_user_id FROM issues WHERE issue_id = ?" );
    query.exec( id );

    if ( query.next() && query.value( 0 ).toInt() == dataManager->currentUserId() )
        return true;

    return false;
}

bool IssueEntity::isAdmin( int id )
{
    if ( dataManager->currentUserAccess() == AdminAccess )
        return true;

    Query query( "SELECT project_access"
        " FROM rights AS r"
        " JOIN folders AS f ON f.project_id = r.project_id"
        " JOIN issues AS i ON i.folder_id = f.folder_id"
        " WHERE i.issue_id = ? AND r.user_id = ?" );
    query.exec( id, dataManager->currentUserId() );

    if ( query.next() && query.value( 0 ).toInt() == AdminAccess )
        return true;

    return false;
}

int IssueEntity::findItem( int itemId )
{
    IssueEntity issue = find( itemId );
    if ( issue.isValid() )
        return itemId;

    Query query( "SELECT issue_id"
        " FROM changes"
        " WHERE change_id = ? AND ( change_type = ? OR change_type = ? )" );
    query.exec( itemId, CommentAdded, FileAdded );

    return query.readScalar().toInt();
}

ValueEntity::ValueEntity() :
    d( new ValueEntityData() )
{
}

ValueEntity::~ValueEntity()
{
}

ValueEntity::ValueEntity( const ValueEntity& other ) :
    d( other.d )
{
}

ValueEntity& ValueEntity::operator =( const ValueEntity& other )
{
    d = other.d;
    return *this;
}

ValueEntityData::ValueEntityData() :
    m_id( 0 )
{
}

ValueEntityData::~ValueEntityData()
{
}

bool ValueEntity::isValid() const
{
    return d->m_id != 0;
}

int ValueEntity::id() const
{
    return d->m_id;
}

const QString& ValueEntity::value() const
{
    return d->m_value;
}

QString ValueEntity::name() const
{
    if ( d->m_id != 0 && d->m_typeId != 0 ) {
        IssueTypeCache* cache = dataManager->issueTypeCache( d->m_typeId );
        return cache->attributeName( d->m_id );
    }
    return QString();
}

DefinitionInfo ValueEntity::definition() const
{
    if ( d->m_id != 0 && d->m_typeId != 0 ) {
        IssueTypeCache* cache = dataManager->issueTypeCache( d->m_typeId );
        return cache->attributeDefinition( d->m_id );
    }
    return DefinitionInfo();
}

ValueEntity ValueEntity::find( int issueId, int attributeId )
{
    ValueEntity entity;

    if ( issueId != 0 && attributeId != 0 ) {
        Query query( "SELECT a.attr_id, v.attr_value, a.type_id"
            " FROM attr_types AS a"
            " LEFT OUTER JOIN attr_values AS v ON v.attr_id = a.attr_id AND v.issue_id = ?"
            " WHERE a.attr_id = ?" );
        query.exec( issueId, attributeId );

        if ( query.next() ) {
            entity.d->read( query );
            entity.d->m_typeId = query.value( 2 ).toInt();
        }
    }

    return entity;
}

class ValueEntityLessThan
{
public:
    ValueEntityLessThan( const QList<int>& attributes ) :
        m_attributes( QVector<int>::fromList( attributes ) )
    {
    }

public:
    bool operator ()( const ValueEntity& v1, const ValueEntity& v2 )
    {
        return m_attributes.indexOf( v1.id() ) < m_attributes.indexOf( v2.id() );
    }

private:
    QVector<int> m_attributes;
};

QList<ValueEntity> IssueEntity::values() const
{
    QList<ValueEntity> result;

    if ( d->m_id != 0 && d->m_typeId != 0 ) {
        Query query( "SELECT a.attr_id, v.attr_value"
            " FROM attr_types AS a"
            " LEFT OUTER JOIN attr_values AS v ON v.attr_id = a.attr_id AND v.issue_id = ?"
            " WHERE a.type_id = ?" );
        query.exec( d->m_id, d->m_typeId );

        while ( query.next() ) {
            ValueEntity entity;
            entity.d->read( query );
            entity.d->m_typeId = d->m_typeId;
            result.append( entity );
        }

        IssueTypeCache* cache = dataManager->issueTypeCache( d->m_typeId );

        qSort( result.begin(), result.end(), ValueEntityLessThan( cache->attributes() ) );
    }

    return result;
}

QList<ValueEntity> IssueEntity::nonEmptyValues() const
{
    QList<ValueEntity> result;

    if ( d->m_id != 0 && d->m_typeId != 0 ) {
        Query query( "SELECT a.attr_id, v.attr_value"
            " FROM attr_types AS a"
            " JOIN attr_values AS v ON v.attr_id = a.attr_id AND v.issue_id = ?"
            " WHERE a.type_id = ?" );
        query.exec( d->m_id, d->m_typeId );

        while ( query.next() ) {
            ValueEntity entity;
            entity.d->read( query );
            entity.d->m_typeId = d->m_typeId;
            result.append( entity );
        }

        IssueTypeCache* cache = dataManager->issueTypeCache( d->m_typeId );

        qSort( result.begin(), result.end(), ValueEntityLessThan( cache->attributes() ) );
    }

    return result;
}

QList<ValueEntity> AttributeEntity::values() const
{
    QList<ValueEntity> result;

    if ( d->m_id != 0 && d->m_typeId != 0 ) {
        Query query( "SELECT attr_id, attr_value FROM attr_values WHERE attr_id = ?" );
        query.exec( d->m_id );

        while ( query.next() ) {
            ValueEntity entity;
            entity.d->read( query );
            entity.d->m_typeId = d->m_typeId;
            result.append( entity );
        }
    }

    return result;
}

void ValueEntityData::read( const Query& query )
{
    m_id = query.value( 0 ).toInt();
    m_value = query.value( 1 ).toString();
}

DescriptionEntity::DescriptionEntity() :
    d ( new DescriptionEntityData() )
{
}

DescriptionEntity::~DescriptionEntity()
{
}

DescriptionEntity::DescriptionEntity( const DescriptionEntity& other ) :
    d( other.d )
{
}

DescriptionEntity& DescriptionEntity::operator =( const DescriptionEntity& other )
{
    d = other.d;
    return *this;
}

DescriptionEntityData::DescriptionEntityData() :
    m_id( 0 ),
    m_format( PlainText ),
    m_modifiedUserId( 0 )
{
}

DescriptionEntityData::~DescriptionEntityData()
{
}

bool DescriptionEntity::isValid() const
{
    return d->m_id != 0;
}

int DescriptionEntity::id() const
{
    return d->m_id;
}

const QString& DescriptionEntity::text() const
{
    return d->m_text;
}

TextFormat DescriptionEntity::format() const
{
    return d->m_format;
}

const QDateTime& DescriptionEntity::modifiedDate() const
{
    return d->m_modifiedDate;
}

const QString& DescriptionEntity::modifiedUser() const
{
    return d->m_modifiedUser;
}

int DescriptionEntity::modifiedUserId() const
{
    return d->m_modifiedUserId;
}

DescriptionEntity ProjectEntity::description() const
{
    DescriptionEntity result;

    if ( d->m_id != 0 ) {
        Query query( "SELECT pd.project_id, pd.descr_text, pd.descr_format, pd.modified_time, um.user_name AS modified_user, pd.modified_user_id"
            " FROM project_descriptions AS pd"
            " LEFT OUTER JOIN users AS um ON um.user_id = pd.modified_user_id"
            " WHERE pd.project_id = ?" );
        query.exec( d->m_id );

        if ( query.next() )
            result.d->read( query );
    }

    return result;
}

DescriptionEntity IssueEntity::description() const
{
    DescriptionEntity result;

    if ( d->m_id != 0 ) {
        Query query( "SELECT id.issue_id, id.descr_text, id.descr_format, id.modified_time, um.user_name AS modified_user, id.modified_user_id"
            " FROM issue_descriptions AS id"
            " LEFT OUTER JOIN users AS um ON um.user_id = id.modified_user_id"
            " WHERE id.issue_id = ?" );
        query.exec( d->m_id );

        if ( query.next() )
            result.d->read( query );
    }

    return result;
}

void DescriptionEntityData::read( const Query& query )
{
    m_id = query.value( 0 ).toInt();
    m_text = query.value( 1 ).toString();
    m_format = (TextFormat)query.value( 2 ).toInt();
    m_modifiedDate.setTime_t( query.value( 3 ).toInt() );
    m_modifiedUser = query.value( 4 ).toString();
    m_modifiedUserId = query.value( 5 ).toInt();
}

CommentEntity::CommentEntity() :
    d( new CommentEntityData() )
{
}

CommentEntity::~CommentEntity()
{
}

CommentEntity::CommentEntity( const CommentEntity& other ) :
    d( other.d )
{
}

CommentEntity& CommentEntity::operator =( const CommentEntity& other )
{
    d = other.d;
    return *this;
}

CommentEntityData::CommentEntityData() :
    m_id( 0 )
{
}

CommentEntityData::~CommentEntityData()
{
}

bool CommentEntity::isValid() const
{
    return d->m_id != 0;
}

int CommentEntity::id() const
{
    return d->m_id;
}

const QString& CommentEntity::text() const
{
    return d->m_text;
}

TextFormat CommentEntity::format() const
{
    return d->m_format;
}

void CommentEntityData::read( const Query& query )
{
    m_id = query.value( 0 ).toInt();
    m_text = query.value( 1 ).toString();
    m_format = (TextFormat)query.value( 2 ).toInt();
}

FileEntity::FileEntity() :
    d( new FileEntityData() )
{
}

FileEntity::~FileEntity()
{
}

FileEntity::FileEntity( const FileEntity& other ) :
    d( other.d )
{
}

FileEntity& FileEntity::operator =( const FileEntity& other )
{
    d = other.d;
    return *this;
}

FileEntityData::FileEntityData() :
    m_id( 0 ),
    m_size( 0 )
{
}

FileEntityData::~FileEntityData()
{
}

bool FileEntity::isValid() const
{
    return d->m_id != 0;
}

int FileEntity::id() const
{
    return d->m_id;
}

const QString& FileEntity::name() const
{
    return d->m_name;
}

int FileEntity::size() const
{
    return d->m_size;
}

const QString& FileEntity::description() const
{
    return d->m_description;
}

void FileEntityData::read( const Query& query )
{
    m_id = query.value( 0 ).toInt();
    m_name = query.value( 1 ).toString();
    m_size = query.value( 2 ).toInt();
    m_description = query.value( 3 ).toString();
}

ChangeEntity::ChangeEntity() :
    d( new ChangeEntityData() )
{
}

ChangeEntity::~ChangeEntity()
{
}

ChangeEntity::ChangeEntity( const ChangeEntity& other ) :
    d( other.d )
{
}

ChangeEntity& ChangeEntity::operator =( const ChangeEntity& other )
{
    d = other.d;
    return *this;
}

ChangeEntityData::ChangeEntityData() :
    m_id( 0 ),
    m_type( IssueCreated ),
    m_createdUserId( 0 ),
    m_modifiedUserId( 0 ),
    m_typeId( 0 ),
    m_attributeId( 0 ),
    m_fileSize( 0 )
{
}

ChangeEntityData::~ChangeEntityData()
{
}

bool ChangeEntity::isValid() const
{
    return d->m_id != 0;
}

int ChangeEntity::id() const
{
    return d->m_id;
}

int ChangeEntity::issueId() const
{
    return d->m_issueId;
}

int ChangeEntity::stampId() const
{
    return d->m_stampId;
}

ChangeType ChangeEntity::type() const
{
    return d->m_type;
}

const QDateTime& ChangeEntity::createdDate() const
{
    return d->m_createdDate;
}

const QString& ChangeEntity::createdUser() const
{
    return d->m_createdUser;
}

int ChangeEntity::createdUserId() const
{
    return d->m_createdUserId;
}

const QDateTime& ChangeEntity::modifiedDate() const
{
    return d->m_modifiedDate;
}

const QString& ChangeEntity::modifiedUser() const
{
    return d->m_modifiedUser;
}

int ChangeEntity::modifiedUserId() const
{
    return d->m_modifiedUserId;
}

int ChangeEntity::attributeId() const
{
    return d->m_attributeId;
}

const QString& ChangeEntity::oldValue() const
{
    return d->m_oldValue;
}

const QString& ChangeEntity::newValue() const
{
    return d->m_newValue;
}

QString ChangeEntity::name() const
{
    if ( d->m_typeId != 0 && d->m_attributeId != 0 ) {
        IssueTypeCache* cache = dataManager->issueTypeCache( d->m_typeId );
        return cache->attributeName( d->m_attributeId );
    }
    return QString();
}

DefinitionInfo ChangeEntity::definition() const
{
    if ( d->m_typeId != 0 && d->m_attributeId != 0 ) {
        IssueTypeCache* cache = dataManager->issueTypeCache( d->m_typeId );
        return cache->attributeDefinition( d->m_attributeId );
    }
    return DefinitionInfo();
}

const QString& ChangeEntity::fromProject() const
{
    return d->m_fromProject;
}

const QString& ChangeEntity::fromFolder() const
{
    return d->m_fromFolder;
}

const QString& ChangeEntity::toProject() const
{
    return d->m_toProject;
}

const QString& ChangeEntity::toFolder() const
{
    return d->m_toFolder;
}

CommentEntity ChangeEntity::comment() const
{
    CommentEntity entity;

    if ( d->m_id != 0 && d->m_type == CommentAdded ) {
        entity.d->m_id = d->m_id;
        entity.d->m_text = d->m_commentText;
        entity.d->m_format = d->m_commentFormat;
    }

    return entity;
}

FileEntity ChangeEntity::file() const
{
    FileEntity entity;

    if ( d->m_id != 0 && d->m_type == FileAdded ) {
        entity.d->m_id = d->m_id;
        entity.d->m_name = d->m_fileName;
        entity.d->m_size = d->m_fileSize;
        entity.d->m_description = d->m_fileDescription;
    }

    return entity;
}

QList<ChangeEntity> IssueEntity::changes( Qt::SortOrder order ) const
{
    return d->changes( true, order );
}

QList<ChangeEntity> IssueEntity::commentsAndFiles( Qt::SortOrder order ) const
{
    return d->changes( false, order );
}

QList<ChangeEntity> IssueEntityData::changes( bool all, Qt::SortOrder order ) const
{
    QList<ChangeEntity> result;

    if ( m_id != 0 && m_typeId != 0 ) {
        QString sql = "SELECT ch.change_id, ch.issue_id, ch.stamp_id, ch.change_type,"
            " ch.created_time, uc.user_name AS created_user, ch.created_user_id,"
            " ch.modified_time, um.user_name AS modified_user, ch.modified_user_id,"
            " a.attr_id, ch.old_value, ch.new_value, fp.project_name AS from_project, ff.folder_name AS from_folder,"
            " tp.project_name AS to_project, tf.folder_name AS to_folder,"
            " c.comment_text, c.comment_format, f.file_name, f.file_size, f.file_descr"
            " FROM changes AS ch"
            " LEFT OUTER JOIN users AS uc ON uc.user_id = ch.created_user_id"
            " LEFT OUTER JOIN users AS um ON um.user_id = ch.modified_user_id"
            " LEFT OUTER JOIN attr_types AS a ON a.attr_id = ch.attr_id"
            " LEFT OUTER JOIN folders AS ff ON ff.folder_id = ch.from_folder_id"
            " LEFT OUTER JOIN projects AS fp ON fp.project_id = ff.project_id"
            " LEFT OUTER JOIN folders AS tf ON tf.folder_id = ch.to_folder_id"
            " LEFT OUTER JOIN projects AS tp ON tp.project_id = tf.project_id"
            " LEFT OUTER JOIN comments AS c ON c.comment_id = ch.change_id AND ch.change_type = ?"
            " LEFT OUTER JOIN files AS f ON f.file_id = ch.change_id AND ch.change_type = ?"
            " WHERE ch.issue_id = ?";
        if ( !all )
            sql += " AND ( c.comment_id IS NOT NULL OR f.file_id IS NOT NULL )";
        sql += " ORDER BY ch.change_id";
        sql += ( order == Qt::DescendingOrder ) ? " DESC" : " ASC";

        Query query( sql );
        query.exec( CommentAdded, FileAdded, m_id );

        while ( query.next() ) {
            ChangeEntity entity;
            entity.d->read( query );
            entity.d->m_typeId = m_typeId;
            result.append( entity );
        }
    }

    return result;
}

void ChangeEntityData::read( const Query& query )
{
    m_id = query.value( 0 ).toInt();
    m_issueId = query.value( 1 ).toInt();
    m_stampId = query.value( 2 ).toInt();
    m_type = (ChangeType)query.value( 3 ).toInt();
    m_createdDate.setTime_t( query.value( 4 ).toInt() );
    m_createdUser = query.value( 5 ).toString();
    m_createdUserId = query.value( 6 ).toInt();
    m_modifiedDate.setTime_t( query.value( 7 ).toInt() );
    m_modifiedUser = query.value( 8 ).toString();
    m_modifiedUserId = query.value( 9 ).toInt();
    if ( m_type == ValueChanged )
        m_attributeId = query.value( 10 ).toInt();
    if ( m_type <= ValueChanged ) {
        m_oldValue = query.value( 11 ).toString();
        m_newValue = query.value( 12 ).toString();
    }
    if ( m_type == IssueMoved ) {
        m_fromProject = query.value( 13 ).toString();
        m_fromFolder = query.value( 14 ).toString();
        m_toProject = query.value( 15 ).toString();
        m_toFolder = query.value( 16 ).toString();
    }
    if ( m_type == CommentAdded ) {
        m_commentText = query.value( 17 ).toString();
        m_commentFormat = (TextFormat)query.value( 18 ).toInt();
    }
    if ( m_type == FileAdded ) {
        m_fileName = query.value( 19 ).toString();
        m_fileSize = query.value( 20 ).toInt();
        m_fileDescription = query.value( 21 ).toString();
    }
}

ChangeEntity ChangeEntity::findComment( int id )
{
    ChangeEntity entity;

    if ( id != 0 ) {
        Query query( "SELECT ch.change_id, ch.issue_id, ch.stamp_id,"
            " ch.created_time, uc.user_name AS created_user, ch.created_user_id,"
            " ch.modified_time, um.user_name AS modified_user,"
            " c.comment_text, c.comment_format"
            " FROM changes AS ch"
            " LEFT OUTER JOIN users AS uc ON uc.user_id = ch.created_user_id"
            " LEFT OUTER JOIN users AS um ON um.user_id = ch.modified_user_id"
            " JOIN comments AS c ON c.comment_id = ch.change_id AND ch.change_type = ?"
            " WHERE ch.change_id = ?" );
        query.exec( CommentAdded, id );

        if ( query.next() )
            entity.d->readComment( query );
    }

    return entity;
}

QList<ChangeEntity> IssueEntity::comments( Qt::SortOrder order ) const
{
    QList<ChangeEntity> result;

    if ( d->m_id != 0 ) {
        QString sql = "SELECT ch.change_id, ch.issue_id, ch.stamp_id,"
            " ch.created_time, uc.user_name AS created_user, ch.created_user_id,"
            " ch.modified_time, um.user_name AS modified_user,"
            " c.comment_text, c.comment_format"
            " FROM changes AS ch"
            " LEFT OUTER JOIN users AS uc ON uc.user_id = ch.created_user_id"
            " LEFT OUTER JOIN users AS um ON um.user_id = ch.modified_user_id"
            " JOIN comments AS c ON c.comment_id = ch.change_id AND ch.change_type = ?"
            " WHERE ch.issue_id = ?"
            " ORDER BY ch.change_id";
        sql += ( order == Qt::DescendingOrder ) ? " DESC" : " ASC";

        Query query( sql );
        query.exec( CommentAdded, d->m_id );

        while ( query.next() ) {
            ChangeEntity entity;
            entity.d->readComment( query );
            result.append( entity );
        }
    }

    return result;
}

void ChangeEntityData::readComment( const Query& query )
{
    m_id = query.value( 0 ).toInt();
    m_issueId = query.value( 1 ).toInt();
    m_stampId = query.value( 2 ).toInt();
    m_type = CommentAdded;
    m_createdDate.setTime_t( query.value( 3 ).toInt() );
    m_createdUser = query.value( 4 ).toString();
    m_createdUserId = query.value( 5 ).toInt();
    m_modifiedDate.setTime_t( query.value( 6 ).toInt() );
    m_modifiedUser = query.value( 7 ).toString();
    m_commentText = query.value( 8 ).toString();
    m_commentFormat = (TextFormat)query.value( 9 ).toInt();
}

ChangeEntity ChangeEntity::findFile( int id )
{
    ChangeEntity entity;

    if ( id != 0 ) {
        Query query( "SELECT ch.change_id, ch.issue_id, ch.stamp_id,"
            " ch.created_time, ch.created_user_id, uc.user_name AS created_user,"
            " ch.modified_time, um.user_name AS modified_user,"
            " f.file_name, f.file_size, f.file_descr"
            " FROM changes AS ch"
            " LEFT OUTER JOIN users AS uc ON uc.user_id = ch.created_user_id"
            " LEFT OUTER JOIN users AS um ON um.user_id = ch.modified_user_id"
            " JOIN files AS f ON f.file_id = ch.change_id AND ch.change_type = ?"
            " WHERE ch.change_id = ?" );
        query.exec( FileAdded, id );

        if ( query.next() )
            entity.d->readFile( query );
    }

    return entity;
}

QList<ChangeEntity> IssueEntity::files( Qt::SortOrder order ) const
{
    QList<ChangeEntity> result;

    if ( d->m_id != 0 ) {
        QString sql = "SELECT ch.change_id, ch.issue_id, ch.stamp_id,"
            " ch.created_time, ch.created_user_id, uc.user_name AS created_user,"
            " ch.modified_time, um.user_name AS modified_user,"
            " f.file_name, f.file_size, f.file_descr"
            " FROM changes AS ch"
            " LEFT OUTER JOIN users AS uc ON uc.user_id = ch.created_user_id"
            " LEFT OUTER JOIN users AS um ON um.user_id = ch.modified_user_id"
            " JOIN files AS f ON f.file_id = ch.change_id AND ch.change_type = ?"
            " WHERE ch.issue_id = ?"
            " ORDER BY ch.change_id";
        sql += ( order == Qt::DescendingOrder ) ? " DESC" : " ASC";

        Query query( sql );
        query.exec( FileAdded, d->m_id );

        while ( query.next() ) {
            ChangeEntity entity;
            entity.d->readFile( query );
            result.append( entity );
        }
    }

    return result;
}

void ChangeEntityData::readFile( const Query& query )
{
    m_id = query.value( 0 ).toInt();
    m_issueId = query.value( 1 ).toInt();
    m_stampId = query.value( 2 ).toInt();
    m_type = FileAdded;
    m_createdDate.setTime_t( query.value( 3 ).toInt() );
    m_createdUser = query.value( 4 ).toString();
    m_createdUserId = query.value( 5 ).toInt();
    m_modifiedDate.setTime_t( query.value( 6 ).toInt() );
    m_modifiedUser = query.value( 7 ).toString();
    m_fileName = query.value( 8 ).toString();
    m_fileSize = query.value( 9 ).toInt();
    m_fileDescription = query.value( 10 ).toString();
}

PreferenceEntity::PreferenceEntity() :
    d( new PreferenceEntityData() )
{
}

PreferenceEntity::~PreferenceEntity()
{
}

PreferenceEntity::PreferenceEntity( const PreferenceEntity& other ) :
    d( other.d )
{
}

PreferenceEntity& PreferenceEntity::operator =( const PreferenceEntity& other )
{
    d = other.d;
    return *this;
}

PreferenceEntityData::PreferenceEntityData()
{
}

PreferenceEntityData::~PreferenceEntityData()
{
}

const QString& PreferenceEntity::key() const
{
    return d->m_key;
}

const QString& PreferenceEntity::value() const
{
    return d->m_value;
}

QList<PreferenceEntity> PreferenceEntity::list( int userId )
{
    QList<PreferenceEntity> result;

    Query query( "SELECT pref_key, pref_value FROM preferences WHERE user_id = ?" );
    query.exec( userId );

    while ( query.next() ) {
        PreferenceEntity entity;
        entity.d->read( query );
        result.append( entity );
    }

    return result;
}

void PreferenceEntityData::read( const Query& query )
{
    m_key = query.value( 0 ).toString();
    m_value = query.value( 1 ).toString();
}

LanguageEntity::LanguageEntity() :
    d( new LanguageEntityData() )
{
}

LanguageEntity::~LanguageEntity()
{
}

LanguageEntity::LanguageEntity( const LanguageEntity& other ) :
    d( other.d )
{
}

LanguageEntity& LanguageEntity::operator =( const LanguageEntity& other )
{
    d = other.d;
    return *this;
}

LanguageEntityData::LanguageEntityData()
{
}

LanguageEntityData::~LanguageEntityData()
{
}

const QString& LanguageEntity::code() const
{
    return d->m_code;
}

const QString& LanguageEntity::name() const
{
    return d->m_name;
}

QList<LanguageEntity> LanguageEntity::list()
{
    QList<LanguageEntity> result;

    Query query( "SELECT lang_code, lang_name FROM languages ORDER BY lang_code" );
    query.exec();

    while ( query.next() ) {
        LanguageEntity entity;
        entity.d->read( query );
        result.append( entity );
    }

    return result;
}

LanguageEntity LanguageEntity::find( const QString& code )
{
    LanguageEntity result;

    Query query( "SELECT lang_code, lang_name FROM languages WHERE lang_code = ?" );
    query.exec( code );

    if ( query.next() )
        result.d->read( query );

    return result;
}

void LanguageEntityData::read( const Query& query )
{
    m_code = query.value( 0 ).toString();
    m_name = query.value( 1 ).toString();
}

TimeZoneEntity::TimeZoneEntity() :
    d( new TimeZoneEntityData() )
{
}

TimeZoneEntity::~TimeZoneEntity()
{
}

TimeZoneEntity::TimeZoneEntity( const TimeZoneEntity& other ) :
    d( other.d )
{
}

TimeZoneEntity& TimeZoneEntity::operator =( const TimeZoneEntity& other )
{
    d = other.d;
    return *this;
}

TimeZoneEntityData::TimeZoneEntityData() :
    m_offset( 0 )
{
}

TimeZoneEntityData::~TimeZoneEntityData()
{
}

const QString& TimeZoneEntity::name() const
{
    return d->m_name;
}

int TimeZoneEntity::offset() const
{
    return d->m_offset;
}

QList<TimeZoneEntity> TimeZoneEntity::list()
{
    QList<TimeZoneEntity> result;

    Query query( "SELECT tz_name, tz_offset FROM time_zones ORDER BY tz_offset, tz_name COLLATE LOCALE" );
    query.exec();

    while ( query.next() ) {
        TimeZoneEntity entity;
        entity.d->read( query );
        result.append( entity );
    }

    return result;
}

void TimeZoneEntityData::read( const Query& query )
{
    m_name = query.value( 0 ).toString();
    m_offset = query.value( 1 ).toInt();
}
