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

#ifndef USERSMODEL_H
#define USERSMODEL_H

#include "models/basemodel.h"

/**
* Model for a list of users.
*/
class UsersModel : public BaseModel
{
    Q_OBJECT
public:
    enum Filter
    {
        AllUsers,
        Active,
        Disabled
    };

public:
    /**
    * Constructor.
    * @param parent The parent object.
    */
    UsersModel( QObject* parent );

    /**
    * Destructor.
    */
    ~UsersModel();

public:
    void setFilter( Filter filter );

public: // overrides
    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;

protected: // overrides
    void updateQueries();

    void updateEvent( UpdateEvent* e );

private:
    void refresh();

private:
    QString m_order;

    Filter m_filter;
};

#endif
