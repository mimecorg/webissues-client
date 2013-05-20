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

#ifndef STARTVIEW_H
#define STARTVIEW_H

#include "view.h"

#include "data/credential.h"

class AbstractBatch;
class Bookmark;
class InputLineEdit;

class QToolButton;
class QTreeWidget;
class QModelIndex;

/**
* View displayed in the main window in disconnected state.
*/
class StartView : public View
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent The parent object.
    * @param parentWidget The parent widget of the view's main widget.
    */
    StartView( QObject* parent, QWidget* parentWidget );

    /**
    * Destructor.
    */
    ~StartView();

public:
    /**
    * Restore connection to the last server.
    */
    void reconnect();

signals:
    /**
    * Emitted when connection to the server is opened.
    */
    void connectionOpened();

public: // overrides
    void initialUpdate();

private slots:
    void openConnection();
    void openConnectionAs();
    void removeConnection();

    void returnPressed();
    void buttonClicked();

    void contextMenu( const QPoint& pos );
    void doubleClicked( const QModelIndex& index );
    void selectionChanged();

    void loginCompleted( bool successful );

private:
    void openConnection( const QString& url, const ServerCredential& credential = ServerCredential() );
    void cancelConnection();

    void executeBatch( AbstractBatch* batch );

    void showLoginDialog( bool error );
    void showLoginNewDialog();

    Bookmark selectedBookmark();

private:
    InputLineEdit* m_edit;
    QToolButton* m_button;

    QTreeWidget* m_list;

    AbstractBatch* m_batch;

    QString m_login;
    QString m_password;
    bool m_remember;
};

#endif
