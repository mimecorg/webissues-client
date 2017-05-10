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

#ifndef ENTITIES_P_H
#define ENTITIES_P_H

#include <QDateTime>

class Query;

class ProjectEntityData : public QSharedData
{
public:
    ProjectEntityData();
    ~ProjectEntityData();

public:
    void read( const Query& query );

public:
    int m_id;
    QString m_name;
    bool m_isPublic;
};

class FolderEntityData : public QSharedData
{
public:
    FolderEntityData();
    ~FolderEntityData();

public:
    void read( const Query& query );

public:
    int m_id;
    int m_projectId;
    int m_typeId;
    QString m_name;
    int m_stampId;
};

class TypeEntityData : public QSharedData
{
public:
    TypeEntityData();
    ~TypeEntityData();

public:
    void read( const Query& query );

public:
    int m_id;
    QString m_name;
};

class AttributeEntityData : public QSharedData
{
public:
    AttributeEntityData();
    ~AttributeEntityData();

public:
    void read( const Query& query );

public:
    int m_id;
    int m_typeId;
    QString m_name;
};

class ViewEntityData : public QSharedData
{
public:
    ViewEntityData();
    ~ViewEntityData();

public:
    void read( const Query& query );

public:
    int m_id;
    int m_typeId;
    QString m_name;
    DefinitionInfo m_definition;
    bool m_isPublic;
};

class AlertEntityData : public QSharedData
{
public:
    AlertEntityData();
    ~AlertEntityData();

public:
    void read( const Query& query );

public:
    int m_id;
    int m_folderId;
    int m_typeId;
    int m_viewId;
    AlertEmail m_alertEmail;
    QString m_summaryDays;
    QString m_summaryHours;
    bool m_isPublic;
};

class UserEntityData : public QSharedData
{
public:
    UserEntityData();
    ~UserEntityData();

public:
    void read( const Query& query );

public:
    int m_id;
    QString m_login;
    QString m_name;
    Access m_access;
};

class MemberEntityData : public QSharedData
{
public:
    MemberEntityData();
    ~MemberEntityData();

public:
    void read( const Query& query );

public:
    int m_id;
    int m_projectId;
    QString m_login;
    QString m_name;
    Access m_access;
};

class IssueEntityData : public QSharedData
{
public:
    IssueEntityData();
    ~IssueEntityData();

public:
    void read( const Query& query );

    QList<ChangeEntity> changes( bool all, Qt::SortOrder order ) const;

public:
    int m_id;
    int m_stampId;
    int m_readId;
    int m_subscriptionId;
    int m_folderId;
    int m_typeId;
    QString m_name;
    QDateTime m_createdDate;
    QString m_createdUser;
    int m_createdUserId;
    QDateTime m_modifiedDate;
    QString m_modifiedUser;
};

class ValueEntityData : public QSharedData
{
public:
    ValueEntityData();
    ~ValueEntityData();

public:
    void read( const Query& query );

public:
    int m_id;
    int m_typeId;
    QString m_value;
};

class DescriptionEntityData : public QSharedData
{
public:
    DescriptionEntityData();
    ~DescriptionEntityData();

public:
    void read( const Query& query );

public:
    int m_id;
    QString m_text;
    TextFormat m_format;
    QDateTime m_modifiedDate;
    QString m_modifiedUser;
    int m_modifiedUserId;
};

class CommentEntityData : public QSharedData
{
public:
    CommentEntityData();
    ~CommentEntityData();

public:
    void read( const Query& query );

public:
    int m_id;
    QString m_text;
    TextFormat m_format;
};

class FileEntityData : public QSharedData
{
public:
    FileEntityData();
    ~FileEntityData();

public:
    void read( const Query& query );

public:
    int m_id;
    QString m_name;

    int m_size;
    QString m_description;
};

class ChangeEntityData : public QSharedData
{
public:
    ChangeEntityData();
    ~ChangeEntityData();

public:
    void read( const Query& query );
    void readComment( const Query& query );
    void readFile( const Query& query );

public:
    int m_id;
    int m_issueId;
    int m_stampId;
    ChangeType m_type;

    QDateTime m_createdDate;
    QString m_createdUser;
    int m_createdUserId;

    QDateTime m_modifiedDate;
    QString m_modifiedUser;
    int m_modifiedUserId;

    QString m_oldValue;
    QString m_newValue;

    int m_typeId;
    int m_attributeId;

    QString m_fromProject;
    QString m_fromFolder;
    QString m_toProject;
    QString m_toFolder;

    QString m_commentText;
    TextFormat m_commentFormat;

    QString m_fileName;
    int m_fileSize;
    QString m_fileDescription;
};

class PreferenceEntityData : public QSharedData
{
public:
    PreferenceEntityData();
    ~PreferenceEntityData();

public:
    void read( const Query& query );

public:
    QString m_key;
    QString m_value;
};

class LanguageEntityData : public QSharedData
{
public:
    LanguageEntityData();
    ~LanguageEntityData();

public:
    void read( const Query& query );

public:
    QString m_code;
    QString m_name;
};

class TimeZoneEntityData : public QSharedData
{
public:
    TimeZoneEntityData();
    ~TimeZoneEntityData();

public:
    void read( const Query& query );

public:
    QString m_name;
    int m_offset;
};

#endif
