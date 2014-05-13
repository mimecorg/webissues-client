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

#ifndef CHECKMESSAGEBOX_H
#define CHECKMESSAGEBOX_H

#include "messagebox.h"

class QCheckBox;

/**
* Message box with a checkbox.
*/
class CheckMessageBox : public MessageBox
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent The parent widget.
    */
    CheckMessageBox( QWidget* parent );

    /**
    * Destructor.
    */
    ~CheckMessageBox();

public:
    /**
    * Set the checkbox text.
    * The default text is "Do not show this message again".
    */
    void setCheckBoxText( const QString& text );

    /**
    * Set the state of the checkbox.
    */
    void setChecked( bool checked );

    /**
    * Return the state of the checkbox.
    */
    bool isChecked();

private:
    QCheckBox* m_checkBox;
};

#endif
