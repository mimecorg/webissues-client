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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include "dialogs/commanddialog.h"

#include "ui_settingsdialog.h"

/**
* Dialog for modifying application settings.
*/
class SettingsDialog : public CommandDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent The parent widget.
    */
    SettingsDialog( QWidget* parent );

    /**
    * Destructor.
    */
    ~SettingsDialog();

public: // overrides
    void accept();

private slots:
    void on_dockCheckBox_toggled( bool on );

    bool apply();

private:
    QTabWidget* m_tabWidget;

    Ui::SettingsDialog m_ui;
};

#endif
