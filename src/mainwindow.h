/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2013 WebIssues Team
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "xmlui/client.h"

#include <QMainWindow>
#include <QSystemTrayIcon>

class StartView;
class View;
class ProjectsView;
class SummaryView;
class FolderView;
class IssueView;
class PaneWidget;
class StatusLabel;

class QStackedWidget;

/**
* The main window of the application.
*
* When connected to the server, the window embeds the projects view,
* adds its actions to the menu and toolbar and handles its signals.
*
* If the tray icon is disabled, closing this window quits the application.
* If it's enabled, the window is hidden.
*/
class MainWindow : public QMainWindow, public XmlUi::Client
{
    Q_OBJECT
public:
    /**
    * Constructor.
    */
    MainWindow();

    /**
    * Destructor.
    */
    ~MainWindow();

public:
    /**
    * Restore connection to the last server.
    */
    void reconnect();

public: // overrides
    QMenu* createPopupMenu();

protected: // overrides
    void closeEvent( QCloseEvent* e );

    bool eventFilter( QObject* object, QEvent* e );

private slots:
    void connectionOpened();

    void quit();
    void closeConnection();
    void connectionInfo();

    void showUsers();
    void showTypes();

    void gotoItem();
    void gotoIssue( int issueId, int itemId );
    void gotoItem( int itemId );

    void changePassword();
    void userPreferences();

    void configure();
    void settingsChanged();

    void trayIconActivated( QSystemTrayIcon::ActivationReason reason );

    void captionChanged( const QString& caption );

    void builderReset();

    void folderEnabledChanged( bool enabled );
    void issueEnabledChanged( bool enabled );

    void selectionChanged( int folderId, int viewId );
    void projectSelected( int projectId );
    void selectedIssueChanged( int issueId );
    void currentViewChanged( int viewId );

    void updateSelection();

    void showStatus( const QPixmap& pixmap, const QString& text, int icon = 0 );
    void showSummary( const QPixmap& pixmap, const QString& text );

private:
    void showStartPage();

    void restoreViewState();
    void storeViewState();

    void updateActions( bool connected );

    void setActiveView( View* view );

    void showFromTray( bool toggle );

    bool queryCloseConnection();

private:
    StartView* m_startView;

    ProjectsView* m_view;
    SummaryView* m_summaryView;
    FolderView* m_folderView;
    IssueView* m_issueView;

    View* m_activeView;

    QStackedWidget* m_stackedWidget;

    PaneWidget* m_folderPane;
    PaneWidget* m_issuePane;

    StatusLabel* m_statusLabel;
    StatusLabel* m_summaryLabel;
    StatusLabel* m_encryptionLabel;
    StatusLabel* m_userLabel;

    QSystemTrayIcon* m_trayIcon;

    QTimer* m_selectionTimer;

    int m_selectedProjectId;
    int m_selectedFolderId;
    int m_selectedIssueId;
    int m_currentViewId;

    bool m_supressFilter;
};

#endif
