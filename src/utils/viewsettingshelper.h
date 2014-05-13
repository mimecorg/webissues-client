/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2014 WebIssues Team
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

#ifndef VIEWSETTINGSHELPER_H
#define VIEWSETTINGSHELPER_H

#include <QObject>
#include <QPair>

class DefinitionInfo;

/**
* Helper object providing information about view settings.
*/
class ViewSettingsHelper : public QObject
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param typeId Identifier of the issue type.
    */
    ViewSettingsHelper( int typeId );

    /**
    * Destructor.
    */
    ~ViewSettingsHelper();

public:
    /**
    * Return names of multiple attributes.
    */
    QString attributeNames( const QList<int>& attributes ) const;

    /**
    * Return name of a column.
    */
    QString columnName( int column ) const;

    /**
    * Return names of multiple columns.
    */
    QString columnNames( const QList<int>& columns ) const;

    /**
    * Return information about sort order.
    */
    QString sortOrderInfo(const QPair<int, Qt::SortOrder>& order ) const;

    /**
    * Return information about filter conditions.
    */
    QString filtersInfo( const QList<DefinitionInfo>& filters ) const;

    /**
    * Return the user-friendly name of the operator.
    */
    QString operatorName( const QString& type ) const;

    /**
    * Return the name of the view or All Issues.
    */
    QString viewName( int viewId ) const;

private:
    int m_typeId;
};

#endif
