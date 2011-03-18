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

#ifndef TABLEMODELS_H
#define TABLEMODELS_H

#include "rdb/abstracttablemodel.h"

class UpdateEvent;

/**
* Column type.
*/
enum Column
{
    Column_Name,
    Column_ID,
    Column_CreatedDate,
    Column_CreatedBy,
    Column_ModifiedDate,
    Column_ModifiedBy,
    Column_Size,
    Column_Description,
    Column_Login,
    Column_Access,
    Column_Required,
    Column_Details,
    Column_DefaultValue,
    Column_OldValue,
    Column_NewValue,
    Column_Type,
    Column_Columns,
    Column_SortBy,
    Column_Filter,
    Column_Address,
    Column_Status,
    Column_EmailType,

    /**
    * The value added to attribute identifier to create user defined columns.
    */
    Column_UserDefined = 1000
};

/**
* Base class for all table models.
*/
class BaseTableModel : public RDB::AbstractTableModel
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent The parent object.
    */
    BaseTableModel( QObject* parent );

    /**
    * Destructor.
    */
    ~BaseTableModel();

public: // overrides
    QString columnName( int column ) const;

protected:
    /**
    * Handle the event indicating that some data was updated.
    */
    virtual void updateEvent( UpdateEvent* e );

protected: // overrides
    void customEvent( QEvent* e );
};

/**
* Model providing information for the rows of the users table.
*/
class UsersTableModel : public BaseTableModel
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent The parent object.
    */
    UsersTableModel( QObject* parent );

    /**
    * Destructor.
    */
    ~UsersTableModel();

public: // overrides
    QString text( int id, int column ) const;

    QPixmap icon( int id, int column ) const;

protected: // overrides
    void updateEvent( UpdateEvent* e );
};

/**
* Model providing information for the rows of the members table.
*/
class MembersTableModel : public BaseTableModel
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent The parent object.
    * @param projectId Identifier of the project containing the members.
    */
    MembersTableModel( int projectId, QObject* parent );

    /**
    * Destructor.
    */
    ~MembersTableModel();

public: // overrides
    QString text( int id, int column ) const;

    QPixmap icon( int id, int column ) const;

protected: // overrides
    void updateEvent( UpdateEvent* e );

private:
    int m_projectId;
};

/**
* Model providing information for the rows of the issue types table.
*/
class TypesTableModel : public BaseTableModel
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent The parent object.
    */
    TypesTableModel( QObject* parent );

    /**
    * Destructor.
    */
    ~TypesTableModel();

public: // overrides
    QString text( int id, int column ) const;

    QPixmap icon( int id, int column ) const;

protected: // overrides
    void updateEvent( UpdateEvent* e );
};

/**
* Model providing information for the rows of the attributes table.
*/
class AttributesTableModel : public BaseTableModel
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent The parent object.
    */
    AttributesTableModel( QObject* parent );

    /**
    * Destructor.
    */
    ~AttributesTableModel();

public: // overrides
    QString text( int id, int column ) const;

    QPixmap icon( int id, int column ) const;

protected: // overrides
    void updateEvent( UpdateEvent* e );
};

/**
* Model providing information for the rows of the projects table.
*/
class ProjectsTableModel : public BaseTableModel
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent The parent object.
    */
    ProjectsTableModel( QObject* parent );

    /**
    * Destructor.
    */
    ~ProjectsTableModel();

public: // overrides
    QString text( int id, int column ) const;

    QPixmap icon( int id, int column ) const;

protected: // overrides
    void updateEvent( UpdateEvent* e );
};

/**
* Model providing information for the rows of the folders table.
*/
class FoldersTableModel : public BaseTableModel
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent The parent object.
    */
    FoldersTableModel( QObject* parent );

    /**
    * Destructor.
    */
    ~FoldersTableModel();

public: // overrides
    QString text( int id, int column ) const;

    QPixmap icon( int id, int column ) const;

protected: // overrides
    void updateEvent( UpdateEvent* e );
};

/**
* Model providing information for the rows of the issues table.
*/
class IssuesTableModel : public BaseTableModel
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent The parent object.
    * @param folderId Identifier of the folder containing the issues.
    */
    IssuesTableModel( int folderId, QObject* parent );

    /**
    * Destructor.
    */
    ~IssuesTableModel();

public: // overrides
    QString text( int id, int column ) const;

    QPixmap icon( int id, int column ) const;

    QString toolTip( int id, int column ) const;

    int compare( int id1, int id2, int column ) const;

protected: // overrides
    void updateEvent( UpdateEvent* e );

private:
    int m_folderId;
};

/**
* Model providing information for the rows of the views table.
*/
class ViewsTableModel : public BaseTableModel
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent The parent object.
    * @param typeId Identifier of the issue type related to the views.
    */
    ViewsTableModel( int typeId, QObject* parent );

    /**
    * Destructor.
    */
    ~ViewsTableModel();

public: // overrides
    QString text( int id, int column ) const;

    QPixmap icon( int id, int column ) const;

protected: // overrides
    void updateEvent( UpdateEvent* e );

private:
    int m_typeId;
};

/**
* Model providing information for the rows of the alerts table.
*/
class AlertsTableModel : public BaseTableModel
{
    Q_OBJECT
public:
    /**
    * Constructor.
    */
    AlertsTableModel( bool treeMode, QObject* parent );

    /**
    * Destructor.
    */
    ~AlertsTableModel();

public: // overrides
    QString text( int id, int column ) const;

    QPixmap icon( int id, int column ) const;

    int compare( int id1, int id2, int column ) const;

protected: // overrides
    void updateEvent( UpdateEvent* e );

private:
    bool m_treeMode;
};

#endif
