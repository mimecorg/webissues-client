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

#ifndef DIALOGMANAGER_H
#define DIALOGMANAGER_H

#include <QObject>
#include <QMap>

class QDialog;

/**
* Class managing the modeless dialogs of the application.
*
* This class provides methods for managing dialogs of various types.
*
* The instance of this class is available using the dialogManager global variable.
* It is created and owned by the Application.
*/
class DialogManager : public QObject
{
    Q_OBJECT
public:
    /**
    * Constructor.
    */
    DialogManager();

    /**
    * Destructor.
    */
    ~DialogManager();

public:
    void addDialog( QDialog* dialog, int id = 0 );

    bool activateDialog( const char* className, int id = 0 );

    bool queryCloseDialogs();

    void closeAllDialogs();

protected: // overrides
    bool eventFilter( QObject* object, QEvent* e );

private slots:
    void dialogDestroyed( QObject* dialog );

private:
    void storeGeometry( QDialog* dialog, bool offset );
    void restoreGeometry( QDialog* dialog );

private:
    QMap<QDialog*, int> m_dialogs;
};

/**
* Global pointer used to access the DialogManager.
*/
extern DialogManager* dialogManager;

#endif
