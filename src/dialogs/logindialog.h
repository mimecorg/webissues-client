/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2017 WebIssues Team
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

#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include "commanddialog.h"

class Bookmark;
class InputLineEdit;

class QCheckBox;

/**
* Dialog for retrieving login and password.
*/
class LoginDialog : public CommandDialog
{
    Q_OBJECT
public:
    enum DialogMode
    {
        ServerMode,
        HttpMode,
        ProxyMode
    };

public:
    /**
    * Constructor.
    * @param parent The parent widget.
    */
    LoginDialog( QWidget* parent );

    /**
    * Constructor.
    * @param mode The mode in which the dialog works.
    * @param hostName The host name.
    * @param serverName The server name.
    * @param parent The parent widget.
    */
    LoginDialog( DialogMode mode, const QString& hostName, const QString& serverName, QWidget* parent );

    /**
    * Destructor.
    */
    ~LoginDialog();

public:
    /**
    * Show login error message.
    */
    void showLoginError();

    /**
    * Set the login and password.
    */
    void setLoginAndPassword( const QString& login, const QString& password );

    /**
    * Return the entered login.
    */
    QString login() const;

    /**
    * Return the entered password.
    */
    QString password() const;

    /**
    * Return @c true if the remember password checkbox is selected.
    */
    bool rememberPassword() const;

public: // overrides
    void accept();

private:
    void initialize( DialogMode mode, const QString& hostName, const QString& serverName );

private:
    InputLineEdit* m_loginEdit;
    InputLineEdit* m_passwordEdit;

    QCheckBox* m_rememberCheck;
};

/**
* Dialog for setting new password at logon.
*/
class LoginNewDialog : public CommandDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param password The current password.
    * @param parent The parent widget.
    */
    LoginNewDialog( const QString& password, QWidget* parent );

    /**
    * Destructor.
    */
    ~LoginNewDialog();

public:
    /**
    * Return the entered password.
    */
    QString password() const;

public: // overrides
    void accept();

private:
    InputLineEdit* m_passwordEdit;
    InputLineEdit* m_passwordEdit2;

    QString m_oldPassword;
};

#endif
