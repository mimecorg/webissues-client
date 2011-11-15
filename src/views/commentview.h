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

#ifndef COMMENTVIEW_H
#define COMMENTVIEW_H

#include "view.h"

class InputTextEdit;

/**
* View for adding comments to issues.
*/
class CommentView : public View
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent The parent object.
    * @param parentWidget The parent widget of the view's main widget.
    */
    CommentView( QObject* parent, QWidget* parentWidget );

    /**
    * Destructor.
    */
    ~CommentView();

public: // overrides
    void initialUpdate();

    bool queryClose();

protected: // overrides
    void enableView();
    void disableView();

    void updateEvent( UpdateEvent* e );

private slots:
    void updateActions();
    void updateClipboard();

    void sendComment();

    void sendCompleted( bool successful );

    void contextMenu( const QPoint& pos );

    void settingsChanged();

private:
    Access checkDataAccess();

    void updateCaption();

private:
    InputTextEdit* m_edit;

    bool m_sending;

    QString m_oldText;

    int m_issueId;
    int m_folderId;
};

#endif
