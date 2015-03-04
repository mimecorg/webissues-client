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

#ifndef VIEWERWINDOW_H
#define VIEWERWINDOW_H

#include "xmlui/client.h"

#include <QMainWindow>

class View;
class StatusLabel;

namespace XmlUi
{
class Builder;
}

/**
* Container window which can contain any kind of view.
*
* The window adds the view's actions to its menu and toolbar,
* handles the view's signals and provides session management
* for the view.
*
* The viewer window is automatically created by the view manager
* when a view is opened.
*/
class ViewerWindow : public QMainWindow, public XmlUi::Client
{
    Q_OBJECT
public:
    /**
    * Constructor.
    */
    ViewerWindow();

    /**
    * Destructor.
    */
    ~ViewerWindow();

public:
    /**
    * Embed the given view in the window.
    */
    void setView( View* view );

    /**
    * Return the view contained in the window.
    */
    View* view() const { return m_view; }

public: // overrides
    QMenu* createPopupMenu();

protected: // overrides
    void closeEvent( QCloseEvent* e );
    void showEvent( QShowEvent* e );

private slots:
    void configure();

    void captionChanged( const QString& caption );

    void enabledChanged( bool enabled );

    void showStatus( const QPixmap& pixmap, const QString& text, int icon = 0 );
    void showSummary( const QPixmap& pixmap, const QString& text );

private:
    void storeGeometry( bool offset );

private:
    View* m_view;

    StatusLabel* m_statusLabel;
    StatusLabel* m_summaryLabel;
};

#endif
