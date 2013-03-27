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

#ifndef SUMMARYVIEW_H
#define SUMMARYVIEW_H

#include "views/view.h"

class FindBar;

/**
* View for displaying summary of a project.
*/
class SummaryView : public View
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent The parent object.
    * @param parentWidget The parent widget of the view's main widget.
    */
    SummaryView( QObject* parent, QWidget* parentWidget );

    /**
    * Destructor.
    */
    ~SummaryView();

public: // overrides
    void initialUpdate();

    bool eventFilter( QObject* obj, QEvent* e );

signals:
    /**
    * Emitted when a link to an issue is clicked.
    */
    void issueActivated( int issueId, int itemId );

protected: // overrides
    void enableView();
    void disableView();
    void updateAccess( Access access );

    void updateEvent( UpdateEvent* e );

private slots:
    void updateCaption();
    void updateActions();

    void updateProject();
    void addDescription();

    void copy();
    void selectAll();

    void find();
    void findNext();
    void findPrevious();

    void findText( const QString& text );

    void openLink();
    void copyLink();

    void summaryContextMenu( const QPoint& pos );

    void linkClicked( const QUrl& url );

    void populateSummary();

private:
    Access checkDataAccess();

    void initialUpdateProject();
    void cascadeUpdateProject();

    void populateSummaryDelayed();

    void findItem( int itemId );

    void findText( const QString& text, int flags );

    bool linkContextMenu( const QUrl& link, const QPoint& pos );

    void handleCommand( const QString& command );

private:
    QWebView* m_browser;

    FindBar* m_findBar;

    QUrl m_actionLink;

    bool m_isFindEnabled;

    QTimer* m_populateTimer;
};

#endif
