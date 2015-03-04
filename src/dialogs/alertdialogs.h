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

#ifndef ALERTDIALOGS_H
#define ALERTDIALOGS_H

#include "dialogs/commanddialog.h"
#include "data/datamanager.h"

class SeparatorComboBox;
class ScheduleWidget;

class QButtonGroup;

/**
* Base class for dialogs for creating and modifying alerts.
*/
class AlertDialog : public CommandDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent The parent widget.
    */
    AlertDialog( QWidget* parent );

    /**
    * Destructor.
    */
    ~AlertDialog();

public:
    enum Flag
    {
        WithView = 1,
        OnlyPublic = 2,
        CheckEmail = 4,
    };

    Q_DECLARE_FLAGS( Flags, Flag )

protected:
    bool initialize( Flags flags = 0, int typeId = 0, const QList<int>& used = QList<int>() );

    int viewId() const;

    void setAlertEmail( AlertEmail email );
    AlertEmail alertEmail() const;

    void setSummaryDays( const QString& days );
    QString summaryDays() const;

    void setSummaryHours( const QString& hours );
    QString summaryHours() const;

    bool validateSummary();

private slots:
    void emailButtonClicked();

private:
    SeparatorComboBox* m_viewCombo;
    QButtonGroup* m_emailGroup;
    ScheduleWidget* m_scheduleWidget;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( AlertDialog::Flags )

/**
* Dialog for executing the <tt>ADD ALERT</tt> command.
*/
class AddAlertDialog : public AlertDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param folderId Identifier of the folder associated with the alert.
    * @param parent The parent widget.
    */
    AddAlertDialog( int folderId, bool isPublic, QWidget* parent );

    /**
    * Destructor.
    */
    ~AddAlertDialog();

public: // overrides
    void accept();

private:
    int m_folderId;
    bool m_isPublic;
};

/**
* Dialog for executing the <tt>ADD GLOBAL ALERT</tt> command.
*/
class AddGlobalAlertDialog : public AlertDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param typeId Identifier of the issue type associated with the alert.
    * @param parent The parent widget.
    */
    AddGlobalAlertDialog( int typeId, bool isPublic, QWidget* parent );

    /**
    * Destructor.
    */
    ~AddGlobalAlertDialog();

public: // overrides
    void accept();

private:
    int m_typeId;
    bool m_isPublic;
};

/**
* Dialog for executing the <tt>MODIFY ALERT</tt> command.
*/
class ModifyAlertDialog : public AlertDialog
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
    AlertEmail m_oldAlertEmail;
    QString m_oldSummaryDays;
    QString m_oldSummaryHours;
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
