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

#ifndef ENTITIES_H
#define ENTITIES_H

#include "data/datamanager.h"

#include <QSharedData>
#include <QList>

class ProjectEntity;
class TypeEntity;
class FolderEntity;
class AttributeEntity;
class ViewEntity;
class AlertEntity;
class UserEntity;
class MemberEntity;
class IssueEntity;
class ValueEntity;
class DescriptionEntity;
class CommentEntity;
class FileEntity;
class ChangeEntity;
class PreferenceEntity;
class LanguageEntity;
class TimeZoneEntity;

class ProjectEntityData;
class TypeEntityData;
class FolderEntityData;
class AttributeEntityData;
class ViewEntityData;
class AlertEntityData;
class UserEntityData;
class MemberEntityData;
class IssueEntityData;
class ValueEntityData;
class DescriptionEntityData;
class CommentEntityData;
class FileEntityData;
class ChangeEntityData;
class PreferenceEntityData;
class LanguageEntityData;
class TimeZoneEntityData;

class DefinitionInfo;

class ProjectEntity
{
public:
    ProjectEntity();
    ~ProjectEntity();

    ProjectEntity( const ProjectEntity& other );
    ProjectEntity& operator =( const ProjectEntity& other );

public:
    bool isValid() const;

    int id() const;
    const QString& name() const;

    DescriptionEntity description() const;

    QList<FolderEntity> folders() const;

    QList<UserEntity> members() const;

public:
    static ProjectEntity find( int id );

    static QList<ProjectEntity> list();

    static bool isAdmin( int id );

    static bool exists( const QString& name );

private:
    QExplicitlySharedDataPointer<ProjectEntityData> d;

    friend class UserEntity;
};

class FolderEntity
{
public:
    FolderEntity();
    ~FolderEntity();

    FolderEntity( const FolderEntity& other );
    FolderEntity& operator =( const FolderEntity& other );

public:
    bool isValid() const;

    int id() const;
    int projectId() const;
    int typeId() const;
    const QString& name() const;
    int stampId() const;

    ProjectEntity project() const;
    TypeEntity type() const;

    QList<AlertEntity> alerts() const;
    QList<IssueEntity> issues() const;

public:
    static FolderEntity find( int id );

    static QList<FolderEntity> list();

    static bool isAdmin( int id );

    static bool exists( int projectId, const QString& name );

private:
    QExplicitlySharedDataPointer<FolderEntityData> d;

    friend class ProjectEntity;
    friend class TypeEntity;
};

class TypeEntity
{
public:
    TypeEntity();
    ~TypeEntity();

    TypeEntity( const TypeEntity& other );
    TypeEntity& operator =( const TypeEntity& other );

public:
    bool isValid() const;

    int id() const;
    const QString& name() const;

    QList<AttributeEntity> attributes() const;

    QList<ViewEntity> views() const;
    QList<AlertEntity> alerts() const;

    QList<FolderEntity> folders() const;

public:
    static TypeEntity find( int id );

    static QList<TypeEntity> list();

    static bool exists( const QString& name );

private:
    QExplicitlySharedDataPointer<TypeEntityData> d;
};

class AttributeEntity
{
public:
    AttributeEntity();
    ~AttributeEntity();

    AttributeEntity( const AttributeEntity& other );
    AttributeEntity& operator =( const AttributeEntity& other );

public:
    bool isValid() const;

    int id() const;
    int typeId() const;
    const QString& name() const;

    TypeEntity type() const;

    QList<ValueEntity> values() const;

public:
    static AttributeEntity find( int id );

    static bool exists( int typeId, const QString& name );

private:
    QExplicitlySharedDataPointer<AttributeEntityData> d;

    friend class TypeEntity;
};

class ViewEntity
{
public:
    ViewEntity();
    ~ViewEntity();

    ViewEntity( const ViewEntity& other );
    ViewEntity& operator =( const ViewEntity& other );

public:
    bool isValid() const;

    int id() const;
    int typeId() const;
    const QString& name() const;
    const DefinitionInfo& definition() const;
    bool isPublic() const;

    TypeEntity type() const;

public:
    static ViewEntity find( int id );

    static bool exists( int typeId, const QString& name, bool isPublic );

private:
    QExplicitlySharedDataPointer<ViewEntityData> d;

    friend class TypeEntity;
};

class AlertEntity
{
public:
    AlertEntity();
    ~AlertEntity();

    AlertEntity( const AlertEntity& other );
    AlertEntity& operator =( const AlertEntity& other );

public:
    bool isValid() const;

    int id() const;
    int viewId() const;
    AlertEmail alertEmail() const;
    const QString& summaryDays() const;
    const QString& summaryHours() const;
    bool isPublic() const;

    FolderEntity folder() const;
    TypeEntity type() const;
    ViewEntity view() const;

public:
    static AlertEntity find( int id );

private:
    QExplicitlySharedDataPointer<AlertEntityData> d;

    friend class FolderEntity;
    friend class TypeEntity;
};

class UserEntity
{
public:
    UserEntity();
    ~UserEntity();

    UserEntity( const UserEntity& other );
    UserEntity& operator =( const UserEntity& other );

public:
    bool isValid() const;

    int id() const;
    const QString& login() const;
    const QString& name() const;
    Access access() const;

    QList<ProjectEntity> projects() const;

public:
    static UserEntity find( int id );

    static QList<UserEntity> list();

    static bool exists( const QString& login, const QString& name );

private:
    QExplicitlySharedDataPointer<UserEntityData> d;

    friend class ProjectEntity;
};

class MemberEntity
{
public:
    MemberEntity();
    ~MemberEntity();

    MemberEntity( const MemberEntity& other );
    MemberEntity& operator =( const MemberEntity& other );

public:
    bool isValid() const;

    int id() const;
    int projectId() const;
    const QString& login() const;
    const QString& name() const;
    Access access() const;

public:
    static MemberEntity find( int projectId, int userId );

    static QList<MemberEntity> list( int userId );

private:
    QExplicitlySharedDataPointer<MemberEntityData> d;
};

class IssueEntity
{
public:
    IssueEntity();
    ~IssueEntity();

    IssueEntity( const IssueEntity& other );
    IssueEntity& operator =( const IssueEntity& other );

public:
    bool isValid() const;

    int id() const;
    const QString& name() const;

    int stampId() const;
    int readId() const;
    int subscriptionId() const;

    int folderId() const;

    FolderEntity folder() const;

    const QDateTime& createdDate() const;
    const QString& createdUser() const;

    const QDateTime& modifiedDate() const;
    const QString& modifiedUser() const;

    QList<ValueEntity> values() const;
    QList<ValueEntity> nonEmptyValues() const;

    DescriptionEntity description() const;

    QList<ChangeEntity> changes( Qt::SortOrder order ) const;
    QList<ChangeEntity> commentsAndFiles( Qt::SortOrder order ) const;
    QList<ChangeEntity> comments( Qt::SortOrder order ) const;
    QList<ChangeEntity> files( Qt::SortOrder order ) const;

public:
    static IssueEntity find( int id );

    static bool isOwner( int id );
    static bool isAdmin( int id );

    static int findItem( int itemId );

private:
    QExplicitlySharedDataPointer<IssueEntityData> d;

    friend class FolderEntity;
};

class ValueEntity
{
public:
    ValueEntity();
    ~ValueEntity();

    ValueEntity( const ValueEntity& other );
    ValueEntity& operator =( const ValueEntity& other );

public:
    bool isValid() const;

    int id() const;
    const QString& value() const;

    QString name() const;
    DefinitionInfo definition() const;

public:
    static ValueEntity find( int issueId, int attributeId );

private:
    QExplicitlySharedDataPointer<ValueEntityData> d;

    friend class IssueEntity;
    friend class AttributeEntity;
};

class DescriptionEntity
{
public:
    DescriptionEntity();
    ~DescriptionEntity();

    DescriptionEntity( const DescriptionEntity& other );
    DescriptionEntity& operator =( const DescriptionEntity& other );

public:
    bool isValid() const;

    int id() const;
    const QString& text() const;
    TextFormat format() const;

    const QDateTime& modifiedDate() const;
    const QString& modifiedUser() const;

private:
    QExplicitlySharedDataPointer<DescriptionEntityData> d;

    friend class ProjectEntity;
    friend class IssueEntity;
};

class CommentEntity
{
public:
    CommentEntity();
    ~CommentEntity();

    CommentEntity( const CommentEntity& other );
    CommentEntity& operator =( const CommentEntity& other );

public:
    bool isValid() const;

    int id() const;
    const QString& text() const;
    TextFormat format() const;

private:
    QExplicitlySharedDataPointer<CommentEntityData> d;

    friend class ChangeEntity;
};

class FileEntity
{
public:
    FileEntity();
    ~FileEntity();

    FileEntity( const FileEntity& other );
    FileEntity& operator =( const FileEntity& other );

public:
    bool isValid() const;

    int id() const;
    const QString& name() const;

    int size() const;
    const QString& description() const;

private:
    QExplicitlySharedDataPointer<FileEntityData> d;

    friend class ChangeEntity;
};

class ChangeEntity
{
public:
    ChangeEntity();
    ~ChangeEntity();

    ChangeEntity( const ChangeEntity& other );
    ChangeEntity& operator =( const ChangeEntity& other );

public:
    bool isValid() const;

    int id() const;
    int issueId() const;
    int stampId() const;
    ChangeType type() const;

    const QDateTime& createdDate() const;
    const QString& createdUser() const;
    int createdUserId() const;

    const QDateTime& modifiedDate() const;
    const QString& modifiedUser() const;

    int attributeId() const;
    const QString& oldValue() const;
    const QString& newValue() const;

    QString name() const;
    DefinitionInfo definition() const;

    const QString& fromFolder() const;
    const QString& toFolder() const;

    CommentEntity comment() const;
    FileEntity file() const;

public:
    static ChangeEntity findComment( int id );
    static ChangeEntity findFile( int id );

private:
    QExplicitlySharedDataPointer<ChangeEntityData> d;

    friend class IssueEntity;
    friend class IssueEntityData;
};

class PreferenceEntity
{
public:
    PreferenceEntity();
    ~PreferenceEntity();

    PreferenceEntity( const PreferenceEntity& other );
    PreferenceEntity& operator =( const PreferenceEntity& other );

public:
    const QString& key() const;
    const QString& value() const;

public:
    static QList<PreferenceEntity> list( int userId );

private:
    QExplicitlySharedDataPointer<PreferenceEntityData> d;
};

class LanguageEntity
{
public:
    LanguageEntity();
    ~LanguageEntity();

    LanguageEntity( const LanguageEntity& other );
    LanguageEntity& operator =( const LanguageEntity& other );

public:
    const QString& code() const;
    const QString& name() const;

public:
    static QList<LanguageEntity> list();

    static LanguageEntity find( const QString& code );

private:
    QExplicitlySharedDataPointer<LanguageEntityData> d;
};

class TimeZoneEntity
{
public:
    TimeZoneEntity();
    ~TimeZoneEntity();

    TimeZoneEntity( const TimeZoneEntity& other );
    TimeZoneEntity& operator =( const TimeZoneEntity& other );

public:
    const QString& name() const;
    int offset() const;

public:
    static QList<TimeZoneEntity> list();

private:
    QExplicitlySharedDataPointer<TimeZoneEntityData> d;
};

Q_DECLARE_TYPEINFO( ProjectEntity, Q_MOVABLE_TYPE );
Q_DECLARE_TYPEINFO( TypeEntity, Q_MOVABLE_TYPE );
Q_DECLARE_TYPEINFO( FolderEntity, Q_MOVABLE_TYPE );
Q_DECLARE_TYPEINFO( AttributeEntity, Q_MOVABLE_TYPE );
Q_DECLARE_TYPEINFO( ViewEntity, Q_MOVABLE_TYPE );
Q_DECLARE_TYPEINFO( AlertEntity, Q_MOVABLE_TYPE );
Q_DECLARE_TYPEINFO( UserEntity, Q_MOVABLE_TYPE );
Q_DECLARE_TYPEINFO( MemberEntity, Q_MOVABLE_TYPE );
Q_DECLARE_TYPEINFO( IssueEntity, Q_MOVABLE_TYPE );
Q_DECLARE_TYPEINFO( ValueEntity, Q_MOVABLE_TYPE );
Q_DECLARE_TYPEINFO( DescriptionEntity, Q_MOVABLE_TYPE );
Q_DECLARE_TYPEINFO( CommentEntity, Q_MOVABLE_TYPE );
Q_DECLARE_TYPEINFO( FileEntity, Q_MOVABLE_TYPE );
Q_DECLARE_TYPEINFO( ChangeEntity, Q_MOVABLE_TYPE );
Q_DECLARE_TYPEINFO( PreferenceEntity, Q_MOVABLE_TYPE );
Q_DECLARE_TYPEINFO( LanguageEntity, Q_MOVABLE_TYPE );
Q_DECLARE_TYPEINFO( TimeZoneEntity, Q_MOVABLE_TYPE );

#endif
