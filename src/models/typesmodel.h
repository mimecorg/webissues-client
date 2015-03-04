/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2015 WebIssues Team
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

#ifndef TYPESMODEL_H
#define TYPESMODEL_H

#include "basemodel.h"

/**
* Model for the tree of issue types and attributes.
*/
class TypesModel : public BaseModel
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent The parent object.
    */
    TypesModel( QObject* parent );

    /**
    * Destructor.
    */
    ~TypesModel();

public: // overrides
    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;

protected: // overrides
    void updateEvent( UpdateEvent* e );

private:
    void refresh();
};

#endif
