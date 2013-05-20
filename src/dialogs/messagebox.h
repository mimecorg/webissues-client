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

#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include "informationdialog.h"

#include <QMessageBox>

/**
* Simple message box.
*/
class MessageBox : public InformationDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent The parent widget.
    */
    MessageBox( QWidget* parent );

    /**
    * Destructor.
    */
    ~MessageBox();

public:
    /**
    * Set the prompt text.
    * The prompt text is displayed in the top part of the dialog. HTML tags
    * can be used.
    */
    void setPrompt( const QString& text );

    /**
    * Set the prompt pixmap.
    */
    void setPromptPixmap( const QPixmap& pixmap );

    /**
    * Set the buttons of the message box.
    * By default only the OK button is displayed.
    */
    void setStandardButtons( QMessageBox::StandardButtons buttons );

    /**
    * Return the given button.
    */
    QPushButton* button( QMessageBox::StandardButton button );

public:
    /**
    * Display a warning message box with given parameters.
    */
    static QMessageBox::StandardButton warning( QWidget* parent, const QString& title, const QString& prompt, QMessageBox::StandardButtons buttons = QMessageBox::Ok );

public: // overrides
    void reject();

private slots:
    void buttonClicked( QAbstractButton* button );
};

#endif
