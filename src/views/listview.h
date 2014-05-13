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

#ifndef LISTVIEW_H
#define LISTVIEW_H

#include "views/view.h"

class FolderModel;
class IssueRowFilter;
class SearchEditBox;
class SeparatorComboBox;
class ReportDialog;

class QTreeView;
class QModelIndex;
class QMenu;
class QActionGroup;

/**
* Abstract view displaying a list of issues.
*/
class ListView : public View
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent The parent object.
    * @param parentWidget The parent widget of the view's main widget.
    */
    ListView( QObject* parent, QWidget* parentWidget );

    /**
    * Destructor.
    */
    ~ListView();

public:
    /**
    * Select issue with given identifier.
    */
    void setSelectedIssueId( int issueId );

    /**
    * Activate view with given identifier.
    */
    void setCurrentViewId( int viewId );

    /**
    * Navigate to the given issue, comment or attachment.
    */
    void gotoIssue( int issueId, int itemId );

public: // overrides
    void initialUpdate();

    bool eventFilter( QObject* obj, QEvent* e );

signals:
    /**
    * Emitted when the selected issue is changed.
    */
    void selectedIssueChanged( int issueId );

    /**
    * Emitted when the current view is changed.
    */
    void currentViewChanged( int viewId );

    /**
    * Emitted when a link to another issue is clicked.
    */
    void issueActivated( int issueId, int itemId );

    /**
    * Emitted when given item should be activated.
    */
    void itemActivated( int itemId );

protected: // overrides
    void enableView();
    void disableView();
    void updateAccess( Access access );

    void updateEvent( UpdateEvent* e );

protected slots:
    virtual void updateFolder() = 0;
    virtual void addIssue() = 0;

    virtual void markAllAsRead() = 0;
    virtual void markAllAsUnread() = 0;

    virtual void issueAdded( int issueId, int folderId ) = 0;

protected:
    virtual void initializeList() = 0;

    virtual void updateSelection() = 0;
    virtual void updateCaption() = 0;

    virtual void initializeReport( ReportDialog* dialog ) = 0;

private slots:
    void updateActions();
    void updateSummary();

    void openIssue();
    void editIssue();
    void cloneIssue();
    void moveIssue();
    void deleteIssue();

    void markAsRead();
    void subscribe();

    void printReport();
    void exportCsv();
    void exportHtml();
    void exportPdf();

    void manageViews();
    void addView();
    void cloneView();
    void modifyView();

    void headerContextMenu( const QPoint& pos );
    void listContextMenu( const QPoint& pos );

    void doubleClicked( const QModelIndex& index );

    void quickSearchChanged( const QString& text );
    void searchActionTriggered( QAction* action );

    void viewActivated( int index );

private:
    void cleanUp();

    void updateViews();
    void updateSearchOptions();

    void loadCurrentView( bool resort );

    void executeReport( int mode );

    QList<int> visibleIssues();
    QList<int> visibleColumns();

protected:
    FolderModel* m_model;

    int m_typeId;

    int m_selectedIssueId;
    bool m_isRead;
    bool m_isSubscribed;
    bool m_isAdmin;

    bool m_hasIssues;

private:
    QTreeView* m_list;

    SeparatorComboBox* m_viewCombo;

    SearchEditBox* m_searchBox;

    QMenu* m_searchMenu;
    QActionGroup* m_searchActionGroup;

    int m_currentViewId;

    int m_searchColumn;
};

#endif
