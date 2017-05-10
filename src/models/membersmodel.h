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

#ifndef MEMBERSMODEL_H
#define MEMBERSMODEL_H

#include "models/basemodel.h"

/**
* Model for a list of members of a project.
*/
class MembersModel : public BaseModel
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param projectId Identifier of the project.
    * @param parent The parent object.
    */
    MembersModel( int projectId, QObject* parent );

    /**
    * Destructor.
    */
    ~MembersModel();

public: // overrides
    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;

protected: // overrides
    void updateQueries();

    void updateEvent( UpdateEvent* e );

private:
    void refresh();

private:
    int m_projectId;

    QString m_order;
};

#endif
