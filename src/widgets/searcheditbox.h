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

#ifndef SEARCHEDITBOX_H
#define SEARCHEDITBOX_H

#include <QLineEdit>

class EditToolButton;

/**
* An edit box with a clear button.
*
* The clear button is only enabled if the text is not empty.
*/
class SearchEditBox : public QLineEdit
{
    Q_OBJECT
public:
    /**
    * Conctructor.
    * @param parent The parent widget.
    */
    SearchEditBox( QWidget* parent );

    /**
    * Destructor.
    */
    ~SearchEditBox();

public:
    /**
    * Set the menu displayed when user clicks the options button.
    */
    void setOptionsMenu( QMenu* menu );

signals:
    /**
    * Emitted when user presses ESC or Enter key.
    */
    void deactivate();

protected:
    void resizeEvent( QResizeEvent* e );

    void keyPressEvent ( QKeyEvent * e );

private slots:
    void updateClearButton( const QString& text );

private:
    EditToolButton* m_optionsButton;
    EditToolButton* m_clearButton;
};

#endif
