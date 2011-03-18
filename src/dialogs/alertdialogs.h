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

#ifndef ALERTDIALOGS_H
#define ALERTDIALOGS_H

#include "commanddialog.h"

class SeparatorComboBox;

class QButtonGroup;

/**
* Dialog for executing the <tt>ADD ALERT</tt> command.
*/
class AddAlertDialog : public CommandDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param folderId Identifier of the folder associated with the alert.
    * @param parent The parent widget.
    */
    AddAlertDialog( int folderId, QWidget* parent );

    /**
    * Destructor.
    */
    ~AddAlertDialog();

public: // overrides
    void accept();

private:
    int m_folderId;

    SeparatorComboBox* m_viewCombo;
    QButtonGroup* m_emailGroup;
};

/**
* Dialog for executing the <tt>MODIFY ALERT</tt> command.
*/
class ModifyAlertDialog : public CommandDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param alertId Identifier of the alert to modify.
    * @param parent The parent widget.
    */
    ModifyAlertDialog( int alertId, QWidget* parent );

    /**
    * Destructor.
    */
    ~ModifyAlertDialog();

public: // overrides
    void accept();

private:
    int m_alertId;
    int m_oldAlertEmail;

    QButtonGroup* m_emailGroup;
};

/**
* Dialog for executing the <tt>DELETE ALERT</tt> command.
*/
class DeleteAlertDialog : public CommandDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param alertId Identifier of the alert to delete.
    * @param parent The parent widget.
    */
    DeleteAlertDialog( int alertId, QWidget* parent );

    /**
    * Destructor.
    */
    ~DeleteAlertDialog();

public: // overrides
    void accept();

private:
    int m_alertId;
};

#endif
