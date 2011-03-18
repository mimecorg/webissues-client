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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include "commanddialog.h"

class SeparatorComboBox;
class InputLineEdit;

class QTabWidget;
class QCheckBox;
class QComboBox;
class QSpinBox;
class QNetworkProxy;

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
    bool apply();

    void dockChanged();
    void proxyChanged();

private:
    void loadLanguages();

private:
    QTabWidget* m_tabWidget;

    QCheckBox* m_dockCheckBox;
    QComboBox* m_showComboBox;
    QComboBox* m_reconnectComboBox;
    SeparatorComboBox* m_languageComboBox;

    QComboBox* m_proxyCombo;
    InputLineEdit* m_proxyHostEdit;
    QSpinBox* m_proxyPortSpin;

    QSpinBox* m_intervalSpin;
    QComboBox* m_actionComboBox;
    QSpinBox* m_cacheSpin;

#if defined( Q_WS_WIN )
    QCheckBox* m_autoStartCheckBox;
#endif
};

#endif
