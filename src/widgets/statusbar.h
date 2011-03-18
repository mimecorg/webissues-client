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

#ifndef STATUSBAR_H
#define STATUSBAR_H

#include <QStatusBar>

class ElidedLabel;

class QLabel;

/**
* A status bar with status icon and label and an optional summary field.
*/
class StatusBar : public QStatusBar
{
    Q_OBJECT
public:
    /**
    * Conctructor.
    * @param parent The parent widget.
    */
    StatusBar( QWidget* parent );

    /**
    * Destructor.
    */
    ~StatusBar();

public:
    /**
    * Show a message with an information icon.
    */
    void showInfo( const QString& text );

    /**
    * Show a message with a warning icon.
    */
    void showWarning( const QString& text );

    /**
    * Show a message with an error icon.
    */
    void showError( const QString& text );

    /**
    * Show a message with a busy icon.
    */
    void showBusy( const QString& text );

public slots:
    /**
    * Show a status message in the status bar.
    */
    void showStatus( const QPixmap& pixmap, const QString& text, int icon = 0 );

    /**
    * Show a summary field in the status bar.
    * If the summary text is empty, the field is hidden.
    */
    void showSummary( const QPixmap& pixmap, const QString& text );

private:
    QLabel* m_pixmap;
    ElidedLabel* m_label;

    QLabel* m_summaryPixmap;
    QLabel* m_summaryLabel;
};

#endif
