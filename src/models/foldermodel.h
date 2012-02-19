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

#ifndef FOLDERMODEL_H
#define FOLDERMODEL_H

#include "basemodel.h"

#include <QStringList>

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

    /**
    * The value added to attribute identifier to create user defined columns.
    */
    Column_UserDefined = 1000
};

/**
* Model for a list of issues in a folder.
*/
class FolderModel : public BaseModel
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param folderId Identifier of the folder.
    * @param parent The parent object.
    */
    FolderModel( int folderId, QObject* parent );

    /**
    * Destructor.
    */
    ~FolderModel();

public:
    /**
    * Fill the model based on the view with given identifier or 0 if
    * the default view should be used.
    */
    void setView( int viewId, bool resort );

    /**
    * Fill the model based on the given list of columns.
    */
    void setColumns( const QList<int>& columns );

    /**
    * Return the current list of columns.
    */
    const QList<int>& columns() const { return m_columns; }

    /**
    * Set the quick search text for the list.
    */
    void setSearchText( int column, const QString& text );

public: // overrides
    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;

protected: // overrides
    void updateQueries();

    void updateEvent( UpdateEvent* e );

private:
    void generateQueries( bool resort );
    void refresh();

private:
    int m_folderId;
    int m_viewId;
    int m_typeId;

    bool m_forceColumns;

    int m_searchColumn;
    QString m_searchText;

    QString m_query;
    QString m_order;

    QList<QVariant> m_arguments;

    QList<int> m_columns;
    QStringList m_sortColumns;
};

#endif
