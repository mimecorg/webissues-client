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

#ifndef VIEWSMODEL_H
#define VIEWSMODEL_H

#include "basemodel.h"

/**
* Model for a list of views.
*/
class ViewsModel : public BaseModel
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param folderId Identifier of the issue type.
    * @param isPublic @c true if public views are displayed.
    * @param parent The parent object.
    */
    ViewsModel( int typeId, bool isPublic, QObject* parent );

    /**
    * Destructor.
    */
    ~ViewsModel();

public: // overrides
    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;

protected: // overrides
    void updateEvent( UpdateEvent* e );

private:
    void refresh();

private:
    int m_typeId;
    bool m_isPublic;
};

#endif
