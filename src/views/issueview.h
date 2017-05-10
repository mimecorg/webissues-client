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

#ifndef ISSUEVIEW_H
#define ISSUEVIEW_H

#include "views/view.h"
#include "data/localsettings.h"
#include "models/issuedetailsgenerator.h"

#include <QUrl>

class PropertyPanel;
class PopEditPanel;
class PopEditBox;
class RowIndex;
class FindBar;

class QTabWidget;
class QTreeView;
class QModelIndex;
class QUrl;
class QFrame;
class QSplitter;
class QWebView;

/**
* View for displaying details of an issue.
*/
class IssueView : public View
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent The parent object.
    * @param parentWidget The parent widget of the view's main widget.
    */
    IssueView( QObject* parent, QWidget* parentWidget );

    /**
    * Destructor.
    */
    ~IssueView();

public:
    /**
    * Navigate to the given comment or file.
    */
    void gotoItem( int itemId );

public: // overrides
    void initialUpdate();

    bool eventFilter( QObject* obj, QEvent* e );

signals:
    /**
    * Emitted when a link to another issue is clicked.
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

    void updateIssue();
    void addComment();
    void addAttachment();
    void editIssue();
    void cloneIssue();
    void moveIssue();
    void deleteIssue();
    void addDescription();
    void markAsRead();
    void subscribe();

    void copy();
    void selectAll();

    void find();
    void findNext();
    void findPrevious();

    void openAttachment();
    void saveAttachment();
    void openLink();
    void copyLink();

    void printReport();
    void exportHtml();
    void exportPdf();

    void historyContextMenu( const QPoint& pos );

    void linkClicked( const QUrl& url );

    void findText( const QString& text );

    void populateDetails();

    void scrollToAnchor();

    void issueAdded( int issueId );

    void settingsChanged();

private:
    void cleanUp();

    Access checkDataAccess();

    void initialUpdateIssue();
    void cascadeUpdateIssue();

    void populateDetailsDelayed();

    void findItem( int itemId );

    void findText( const QString& text, int flags );

    bool linkContextMenu( const QUrl& link, const QPoint& pos );

    void handleAttachment( int fileId );
    void handleAttachment( int fileId, AttachmentAction action );

    void handleCommand( const QString& command, int argument );

    void addComment( const QString& tile, const QString& text );

private:
    QWebView* m_browser;

    FindBar* m_findBar;

    int m_folderId;
    int m_typeId;
    bool m_isRead;
    bool m_isSubscribed;

    IssueDetailsGenerator::History m_history;

    QUrl m_actionLink;

    bool m_isFindEnabled;

    int m_lockedIssueId;

    QTimer* m_populateTimer;

    bool m_loading;
    QString m_scrollAnchor;
};

#endif
