/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2012 WebIssues Team
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

#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include "dialogs/commanddialog.h"

#include <QMap>

class SeparatorComboBox;
class InputLineEdit;

class QTabWidget;
class QCheckBox;
class QButtonGroup;

/**
* Dialog for modifying user preferences using <tt>SET PREFERENCE</tt> commands.
*/
class PreferencesDialog : public CommandDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param userId Identifier of the user to change preferences for.
    * @param parent The parent widget.
    */
    PreferencesDialog( int userId, QWidget* parent );

    /**
    * Destructor.
    */
    ~PreferencesDialog();

public: // overrides
    void accept();

protected: // overrides
    bool batchSuccessful( AbstractBatch* batch );
    bool batchFailed( AbstractBatch* batch );

private slots:
    void fixGeometry();

    void allDaysActivated();
    void noDaysActivated();
    void allHoursActivated();
    void noHoursActivated();

private:
    void initialize();

private:
    int m_userId;

    bool m_initialized;
    QMap<QString, QString> m_preferences;

    QTabWidget* m_tabWidget;

    SeparatorComboBox* m_languageComboBox;
    SeparatorComboBox* m_timeZoneComboBox;

    SeparatorComboBox* m_numberComboBox;
    SeparatorComboBox* m_dateComboBox;
    SeparatorComboBox* m_timeComboBox;
    SeparatorComboBox* m_firstDayComboBox;

    SeparatorComboBox* m_folderPageComboBox;
    SeparatorComboBox* m_historyPageComboBox;

    InputLineEdit* m_emailEdit;
    QCheckBox* m_detailsCheckBox;
    QCheckBox* m_noReadCheckBox;
    QButtonGroup* m_daysGroup;
    QButtonGroup* m_hoursGroup;
};

#endif
