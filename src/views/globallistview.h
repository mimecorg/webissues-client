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

#ifndef GLOBALLISTVIEW_H
#define GLOBALLISTVIEW_H

#include "views/listview.h"

/**
* View for displaying all issues of a given type.
*/
class GlobalListView : public ListView
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent The parent object.
    * @param parentWidget The parent widget of the view's main widget.
    */
    GlobalListView( QObject* parent, QWidget* parentWidget );

    /**
    * Destructor.
    */
    ~GlobalListView();

protected: // overrides
    void updateEvent( UpdateEvent* e );

    void updateFolder();
    void addIssue();

    void markAllAsRead();
    void markAllAsUnread();

    void issueAdded( int issueId, int folderId );

    void initializeList();

    void updateSelection();
    void updateCaption();

    void initializeReport( ReportDialog* dialog );

private:
    Access checkDataAccess();

    void initialUpdateFolder();
    void cascadeUpdateFolder();
};

#endif
