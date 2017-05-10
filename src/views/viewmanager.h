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

#ifndef VIEWMANAGER_H
#define VIEWMANAGER_H

#include <QObject>
#include <QMap>

class View;
class ViewerWindow;

/**
* Class managing the views of the application.
*
* This class provides methods for opening views of various types,
* except the project view which is embedded in the main window.
*
* If a view of a given type with the same identifier already exists,
* it is activated and another view is not created.
*
* All views are embedded in their own ViewerWindow.
*
* The view manager also provides a method for notifying views when
* some configuration is changed and for closing all views querying
* them before closing.
*
* The instance of this class is available using the dataManager global variable.
* It is created and owned by the ConnectionManager.
*/
class ViewManager : public QObject
{
    Q_OBJECT
public:
    /**
    * Constructor.
    */
    ViewManager();

    /**
    * Destructor.
    */
    ~ViewManager();

public:
    /**
    * Open the types view.
    */
    void openTypesView();

    /**
    * Open the users view.
    */
    void openUsersView();

    /**
    * Open the project summary view for the given project.
    */
    void openSummaryView( int projectId );

    /**
    * Open the folder view for the given folder.
    */
    void openFolderView( int folderId );

    /**
    * Open the global list view of the given type.
    */
    void openGlobalListView( int typeId );

    /**
    * Open the issue details view for the given issue.
    */
    void openIssueView( int issueId, int itemId = 0 );

    /**
    * Add the view to the view manager.
    */
    void addView( View* view );

    /**
    * Query all views to close.
    * @return @c true if views can be closed, @c false otherwise.
    */
    bool queryCloseViews();

    /**
    * Close the given view.
    * The view's window is destroyed.
    * @param view The view to close.
    */
    void closeView( View* view );

    /**
    * Close all viewer windows.
    */
    void closeAllViews();

    /**
    * Return @c true is the view is stand-alone.
    * A view is stand-alone when it's contained in a ViewerWindow.
    */
    bool isStandAlone( View* view );

private:
    View* openView( const char* className, int id );

private slots:
    void viewDestroyed( QObject* view );

private:
    QMap<View*, ViewerWindow*> m_views;
};

/**
* Global pointer used to access the ViewManager.
*/
extern ViewManager* viewManager;

#endif
