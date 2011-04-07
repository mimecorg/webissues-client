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

#ifndef DATAROWS_H
#define DATAROWS_H

#include <QString>
#include <QDateTime>

/**
* Access level for user or member.
*/
enum Access
{
    /**
    * Data is unavailable and the access level is not known.
    */
    UnknownAccess = -1,
    /**
    * User has no access to the system or project.
    */
    NoAccess = 0,
    /**
    * User has normal access to the system or project.
    */
    NormalAccess = 1,
    /**
    * User is a system or project administrator.
    */
    AdminAccess = 2
};

/**
* Type of a change.
*/
enum ChangeType
{
    /**
    * The issue was created with the given name.
    */
    IssueCreated = 0,
    /**
    * The issue was renamed.
    */
    IssueRenamed = 1,
    /**
    * The value of an attribute was changed.
    */
    ValueChanged = 2,
    /**
    * A comment was added.
    */
    CommentAdded = 3,
    /**
    * A file was added.
    */
    FileAdded = 4,
    /**
    * The issue was moved to another folder.
    */
    IssueMoved = 5
};

/**
* Email setting for an alert.
*/
enum AlertEmail
{
    /**
    * No emails are sent for the alert.
    */
    NoEmail = 0,
    /**
    * Immediate notifications are sent for the alert.
    */
    ImmediateNotificationEmail = 1,
    /**
    * Summary notifications are sent for the alert.
    */
    SummaryNotificationEmail = 2,
    /**
    * Summary reports are sent for the alert.
    */
    SummaryReportEmail = 3
};

/**
* A row of the users table.
*/
class UserRow
{
public:
    /**
    * Constructor.
    */
    UserRow( int userId, const QString& login, const QString& name, Access access );

    /**
    * Destructor.
    */
    ~UserRow();

public:
    /**
    * Return the identifier of the user.
    */
    int userId() const { return m_userId; }

    /**
    * Return the login of the user.
    */
    const QString& login() const { return m_login; }

    /**
    * Return the display name of the user.
    */
    const QString& name() const { return m_name; }

    /**
    * Return the global access level of the user.
    */
    Access access() const { return m_access; }

private:
    int m_userId;
    QString m_login;
    QString m_name;
    Access m_access;
};

/**
* A row of the members table.
*/
class MemberRow
{
public:
    /**
    * Constructor.
    */
    MemberRow( int userId, int projectId, Access access );

    /**
    * Destructor.
    */
    ~MemberRow();

public:
    /**
    * Return the idenifier of the user.
    */
    int userId() const { return m_userId; }

    /**
    * Return the identifier of the project.
    */
    int projectId() const { return m_projectId; }

    /**
    * Return the user's access level to the project.
    */
    Access access() const { return m_access; }

private:
    int m_userId;
    int m_projectId;
    Access m_access;
};

/**
* A row of the issue types table.
*/
class TypeRow
{
public:
    /**
    * Constructor.
    */
    TypeRow( int typeId, const QString& name );

    /**
    * Destructor.
    */
    ~TypeRow();

public:
    /**
    * Return the identifier of the issue type.
    */
    int typeId() const { return m_typeId; }

    /**
    * Return the name of the issue type.
    */
    const QString& name() const { return m_name; }

private:
    int m_typeId;
    QString m_name;
};

/**
* A row of the attribute definitions table.
*/
class AttributeRow
{
public:
    /**
    * Constructor.
    */
    AttributeRow( int attributeId, int typeId, const QString& name, const QString& definition );

    /**
    * Destructor.
    */
    ~AttributeRow();

public:
    /**
    * Return the identifier of the attribute.
    */
    int attributeId() const { return m_attributeId; }

    /**
    * Return the identifier of the issue type.
    */
    int typeId() const { return m_typeId; }

    /**
    * Return the name of the attribute.
    */
    const QString& name() const { return m_name; }

    /**
    * Return the definition of the attribute.
    */
    const QString& definition() const { return m_definition; }

private:
    int m_attributeId;
    int m_typeId;
    QString m_name;
    QString m_definition;
};

/**
* A row of the projects table.
*/
class ProjectRow
{
public:
    /**
    * Constructor.
    */
    ProjectRow( int projectId, const QString& name );

    /**
    * Destructor.
    */
    ~ProjectRow();

public:
    /**
    * Return the identifier of the project.
    */
    int projectId() const { return m_projectId; }

    /**
    * Return the name of the project.
    */
    const QString& name() const { return m_name; }

private:
    int m_projectId;
    QString m_name;
};

/**
* A row of the folders table.
*/
class FolderRow
{
public:
    /**
    * Constructor.
    */
    FolderRow( int folderId, int projectId, const QString& name, int typeId, int stamp );

    /**
    * Destructor.
    */
    ~FolderRow();

public:
    /**
    * Return the identifier of the folder.
    */
    int folderId() const { return m_folderId; }

    /**
    * Return the identifier of the folder's project.
    */
    int projectId() const { return m_projectId; }

    /**
    * Return the folder name.
    */
    const QString& name() const { return m_name; }

    /**
    * Return the identifier of the issue type of the folder.
    */
    int typeId() const { return m_typeId; }

    /**
    * Return the stamp of the last modification in the folder.
    */
    int stamp() const { return m_stamp; }

private:
    int m_folderId;
    int m_projectId;
    QString m_name;
    int m_typeId;
    int m_stamp;
};

/**
* A row of the issues table.
*/
class IssueRow
{
public:
    /**
    * Constructor.
    */
    IssueRow( int issueId, int folderId, const QString& name, int stamp, const QDateTime& createdDate,
        int createdUser, const QDateTime& modifiedDate, int modifiedUser );

    /**
    * Destructor.
    */
    ~IssueRow();

public:
    /**
    * Return the identifier of the issue.
    */
    int issueId() const { return m_issueId; }

    /**
    * Return the identifier of the issue's folder.
    */
    int folderId() const { return m_folderId; }

    /**
    * Return the issue name.
    */
    const QString& name() const { return m_name; }

    /**
    * Return the stamp of the last modification of the issue.
    */
    int stamp() const { return m_stamp; }

    /**
    * Return the creation date.
    */
    const QDateTime& createdDate() const { return m_createdDate; }

    /**
    * Return the identifier of the user who created the issue.
    */
    int createdUser() const { return m_createdUser; }

    /**
    * Return the last modification date.
    */
    const QDateTime& modifiedDate() const { return m_modifiedDate; }

    /**
    * Return the identifier of the user who last modified the issue.
    */
    int modifiedUser() const { return m_modifiedUser; }

private:
    int m_issueId;
    int m_folderId;
    QString m_name;
    int m_stamp;
    QDateTime m_createdDate;
    int m_createdUser;
    QDateTime m_modifiedDate;
    int m_modifiedUser;
};

/**
* A row of the attribute values table.
*/
class ValueRow
{
public:
    /**
    * Constructor.
    */
    ValueRow( int attributeId, int issueId, const QString& value );

    /**
    * Destructor.
    */
    ~ValueRow();

public:
    /**
    * Return the identifier of the attribute.
    */
    int attributeId() const { return m_attributeId; }

    /**
    * Return the identifier of the issue.
    */
    int issueId() const { return m_issueId; }

    /**
    * Return the attribute value.
    */
    const QString& value() const { return m_value; }

private:
    int m_attributeId;
    int m_issueId;
    QString m_value;
};

/**
* A row of the changes table.
*/
class ChangeRow
{
public:
    /**
    * Constructor.
    */
    ChangeRow( int changeId, int issueId, ChangeType changeType, int stamp, const QDateTime& createdDate, int createdUser,
        const QDateTime& modifiedDate, int modifiedUser, int attributeId, const QString& oldValue, const QString& newValue,
        int fromFolder, int toFolder );

    /**
    * Destructor.
    */
    ~ChangeRow();

public:
    /**
    * Return the identifier of the change.
    */
    int changeId() const { return m_changeId; }

    /**
    * Return the identifier of the issue.
    */
    int issueId() const { return m_issueId; }

    /**
    * Return the type of the change.
    */
    ChangeType changeType() const { return m_changeType; }

    /**
    * Return the stamp of the last modification of the change.
    */
    int stamp() const { return m_stamp; }

    /**
    * Return the date the change was made.
    */
    const QDateTime& createdDate() const { return m_createdDate; }

    /**
    * Return the identifier of the user who made the change.
    */
    int createdUser() const { return m_createdUser; }

    /**
    * Return the date the change was last modified.
    */
    const QDateTime& modifiedDate() const { return m_modifiedDate; }

    /**
    * Return the identifier of the user who last modified the change.
    */
    int modifiedUser() const { return m_modifiedUser; }

    /**
    * Return the identifier of the changed attribute.
    */
    int attributeId() const { return m_attributeId; }

    /**
    * Return the old value of the changed attribute.
    */
    const QString& oldValue() const { return m_oldValue; }

    /**
    * Return the new value of the changed attribute.
    */
    const QString& newValue() const { return m_newValue; }

    /**
    * Return the identifier of the source folder.
    */
    int fromFolder() const { return m_fromFolder; }

    /**
    * Return the identifier of the target folder.
    */
    int toFolder() const { return m_toFolder; }

private:
    int m_changeId;
    int m_issueId;
    ChangeType m_changeType;
    int m_stamp;
    QDateTime m_createdDate;
    int m_createdUser;
    QDateTime m_modifiedDate;
    int m_modifiedUser;
    int m_attributeId;
    QString m_oldValue;
    QString m_newValue;
    int m_fromFolder;
    int m_toFolder;
};

/**
* A row of the comments table.
*/
class CommentRow
{
public:
    /**
    * Constructor.
    */
    CommentRow( int commentId, const QString& text );

    /**
    * Destructor.
    */
    ~CommentRow();

public:
    /**
    * Return the identifier of the comment.
    */
    int commentId() const { return m_commentId; }

    /**
    * Return the text of the comment.
    */
    const QString& text() const { return m_text; }

private:
    int m_commentId;
    QString m_text;
};

/**
* A row of the files table.
*/
class FileRow
{
public:
    /**
    * Constructor.
    */
    FileRow( int fileId, const QString& name, int size, const QString& description );

    /**
    * Destructor.
    */
    ~FileRow();

public:
    /**
    * Return the identifier of the file.
    */
    int fileId() const { return m_fileId; }

    /**
    * Return the name of the file.
    */
    const QString& name() const { return m_name; }

    /**
    * Return the size of the file.
    */
    int size() const { return m_size; }

    /**
    * Return the description of the file.
    */
    const QString& description() const { return m_description; }

private:
    int m_fileId;
    QString m_name;
    int m_size;
    QString m_description;
};

/**
* A row of the views table.
*/
class ViewRow
{
public:
    /**
    * Constructor.
    */
    ViewRow( int viewId, int typeId, const QString& name, const QString& definition, bool isPublic );

    /**
    * Destructor.
    */
    ~ViewRow();

public:
    /**
    * Return the identifier of the view.
    */
    int viewId() const { return m_viewId; }

    /**
    * Return the identifier of the issue type.
    */
    int typeId() const { return m_typeId; }

    /**
    * Return the name of the view.
    */
    QString name() const { return m_name; }

    /**
    * Return the definiton of the view.
    */
    QString definition() const { return m_definition; }

    /**
    * Return @c true if the view is public.
    */
    bool isPublic() const { return m_isPublic; }

private:
    int m_viewId;
    int m_typeId;
    QString m_name;
    QString m_definition;
    bool m_isPublic;
};

/**
* A row of the alerts table.
*/
class AlertRow
{
public:
    /**
    * Constructor.
    */
    AlertRow( int alertId, int folderId, int viewId, AlertEmail alertEmail );

    /**
    * Destructor.
    */
    ~AlertRow();

public:
    /**
    * Return the identifier of the alert.
    */
    int alertId() const { return m_alertId; }

    /**
    * Return the identifier of the folder.
    */
    int folderId() const { return m_folderId; }

    /**
    * Return the identifier of the view.
    */
    int viewId() const { return m_viewId; }

    /**
    * Return the email type for the alert.
    */
    AlertEmail alertEmail() const { return m_alertEmail; }

private:
    int m_alertId;
    int m_folderId;
    int m_viewId;
    AlertEmail m_alertEmail;
};

#endif
