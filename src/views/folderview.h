/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2011 WebIssues Team
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

#ifndef FOLDERVIEW_H
#define FOLDERVIEW_H

#include "views/view.h"

class FolderModel;
class IssueRowFilter;
class SearchEditBox;
class SeparatorComboBox;
class ColumnCondition;

class QTreeView;
class QModelIndex;
class QMenu;
class QActionGroup;

/**
* View for displaying issues in a folder.
*/
class FolderView : public View
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent The parent object.
    * @param parentWidget The parent widget of the view's main widget.
    */
    FolderView( QObject* parent, QWidget* parentWidget );

    /**
    * Destructor.
    */
    ~FolderView();

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
    * Emitted when given item should be activated.
    */
    void gotoItem( int itemId );

protected: // overrides
    void enableView();
    void disableView();
    void updateAccess( Access access );

    void updateEvent( UpdateEvent* e );

private slots:
    void updateActions();
    void updateSummary();

    void updateFolder();
    void openIssue();
    void addIssue();
    void editIssue();
    void moveIssue();
    void deleteIssue();

    void markAsRead();
    void markAllAsRead();
    void markAllAsUnread();

    void printReport();
    void exportCsv();
    void exportHtml();
    void exportPdf();

    void manageViews();
    void addView();
    void modifyView();

    void headerContextMenu( const QPoint& pos );
    void listContextMenu( const QPoint& pos );

    void doubleClicked( const QModelIndex& index );

    void quickSearchChanged( const QString& text );

    void viewActivated( int index );

private:
    void cleanUp();

    Access checkDataAccess();

    void initialUpdateFolder();
    void cascadeUpdateFolder();

    void updateCaption();

    void updateViews();

    void markAllIssues( bool read );

    void loadCurrentView( bool resort );

    QList<int> visibleIssues();

private:
    QTreeView* m_list;
    FolderModel* m_model;

    SeparatorComboBox* m_viewCombo;

    SearchEditBox* m_searchBox;

    int m_currentViewId;

    int m_typeId;
    int m_projectId;

    int m_gotoIssueId;
    int m_gotoItemId;

    int m_selectedIssueId;
    bool m_isRead;
};

#endif
