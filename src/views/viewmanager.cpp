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

#include "viewmanager.h"
#include "view.h"
#include "issueview.h"
#include "folderview.h"
#include "viewfactory.h"
#include "viewerwindow.h"

#include <QApplication>

ViewManager* viewManager = NULL;

ViewManager::ViewManager()
{
}

ViewManager::~ViewManager()
{
    closeAllViews();
}

void ViewManager::openTypesView()
{
    openView( "TypesView", 0 );
}

void ViewManager::openUsersView()
{
    openView( "UsersView", 0 );
}

void ViewManager::openSummaryView( int projectId )
{
    openView( "SummaryView", projectId );
}

void ViewManager::openFolderView( int folderId )
{
    openView( "FolderView", folderId );
}

void ViewManager::openGlobalListView( int typeId )
{
    openView( "GlobalListView", typeId );
}

void ViewManager::openIssueView( int issueId, int itemId )
{
    IssueView* view = (IssueView*)openView( "IssueView", issueId );
    if ( itemId != 0 )
        view->gotoItem( itemId );
}

bool ViewManager::queryCloseViews()
{
    QList<View*> views = m_views.keys();

    for ( int i = 0; i < views.count(); i++ ) {
        View* view = views.at( i );
        if ( !view->queryClose() )
            return false;
    }

    return true;
}

void ViewManager::closeView( View* view )
{
    ViewerWindow* window = m_views.value( view, NULL );
    if ( !window )
        return;

    window->deleteLater();
}

void ViewManager::closeAllViews()
{
    QList<ViewerWindow*> windows = m_views.values();

    for ( int i = 0; i < windows.count(); i++ ) {
        ViewerWindow* window = windows.at( i );
        delete window;
    }

    m_views.clear();
}

View* ViewManager::openView( const char* className, int id )
{
    View* view = NULL;
    ViewerWindow* window = NULL;

    QList<View*> views = m_views.keys();

    for ( int i = 0; i < views.count(); i++ ) {
        view = views.at( i );
        if ( view->inherits( className ) && ( id == 0 || view->id() == id ) ) {
            window = m_views.value( view, NULL );
            if ( window ) {
                window->raise();
                window->activateWindow();
                return view;
            }
        }
    }

    window = new ViewerWindow();
    view = ViewFactory::createView( className, window, window );

    m_views.insert( view, window );
    connect( view, SIGNAL( destroyed( QObject* ) ), this, SLOT( viewDestroyed( QObject* ) ) );

    view->setId( id );
    window->setView( view );

    view->initialUpdate();
    window->show();

    return view;
}

void ViewManager::addView( View* view )
{
    m_views.insert( view, NULL );
    connect( view, SIGNAL( destroyed( QObject* ) ), this, SLOT( viewDestroyed( QObject* ) ) );
}

void ViewManager::viewDestroyed( QObject* view )
{
    m_views.remove( (View*)view );
}

bool ViewManager::isStandAlone( View* view )
{
    return m_views.value( view, NULL ) != NULL;
}
