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

#ifndef USERDIALOGS_H
#define USERDIALOGS_H

#include "dialogs/commanddialog.h"

class InputLineEdit;

class QCheckBox;
class QButtonGroup;
class QListWidget;

/**
* Dialog for executing the <tt>ADD USER</tt> command.
*/
class AddUserDialog : public CommandDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent The parent widget.
    */
    AddUserDialog( QWidget* parent );

    /**
    * Destructor.
    */
    ~AddUserDialog();

public: // overrides
    void accept();

private:
    InputLineEdit* m_loginEdit;
    InputLineEdit* m_nameEdit;
    InputLineEdit* m_passwordEdit;
    InputLineEdit* m_passwordEdit2;
    QCheckBox* m_tempCheck;
};

/**
* Dialog for executing the <tt>SET PASSWORD</tt> command.
*/
class SetPasswordDialog : public CommandDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param userId Identifier of the user.
    * @param parent The parent widget.
    */
    SetPasswordDialog( int userId, QWidget* parent );

    /**
    * Destructor.
    */
    ~SetPasswordDialog();

public: // overrides
    void accept();

private:
    int m_userId;

    InputLineEdit* m_passwordEdit;
    InputLineEdit* m_newPasswordEdit;
    InputLineEdit* m_newPasswordEdit2;
    QCheckBox* m_tempCheck;
};

/**
* Dialog for executing the <tt>GRANT USER</tt> command.
*/
class ChangeUserAccessDialog : public CommandDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param userId Identifier of the user.
    * @param parent The parent widget.
    */
    ChangeUserAccessDialog( int userId, QWidget* parent );

    /**
    * Destructor.
    */
    ~ChangeUserAccessDialog();

public: // overrides
    void accept();

private:
    int m_userId;
    int m_oldAccess;

    QButtonGroup* m_accessGroup;
};

/**
* Dialog for executing the <tt>RENAME USER</tt> command.
*/
class RenameUserDialog : public CommandDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param userId Identifier of the user to rename.
    * @param parent The parent widget.
    */
    RenameUserDialog( int userId, QWidget* parent );

    /**
    * Destructor.
    */
    ~RenameUserDialog();

public: // overrides
    void accept();

private:
    int m_userId;
    QString m_oldName;

    InputLineEdit* m_nameEdit;
};

/**
* Dialog for executing the <tt>GRANT MEMBER</tt> command.
*
* This dialog allows to select new members of the project.
*/
class AddMemberDialog : public CommandDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param projectId Identifier of the project.
    * @param parent The parent widget.
    */
    AddMemberDialog( int projectId, QWidget* parent );

    /**
    * Destructor.
    */
    ~AddMemberDialog();

public: // overrides
    void accept();

private slots:
    void allUsersActivated();
    void noUsersActivated();

private:
    int m_projectId;

    QListWidget* m_list;
    QButtonGroup* m_accessGroup;
};

/**
* Dialog for executing the <tt>GRANT MEMBER</tt> command.
*
* This dialog allows to change the access level for existing members.
*/
class ChangeMemberAccessDialog : public CommandDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param users Identifiers of the users.
    * @param projectId Identifier of the project.
    * @param parent The parent widget.
    */
    ChangeMemberAccessDialog( const QList<int>& users, int projectId, QWidget* parent );

    /**
    * Destructor.
    */
    ~ChangeMemberAccessDialog();

public: // overrides
    void accept();

private:
    QList<int> m_users;
    int m_projectId;

    QButtonGroup* m_accessGroup;
};

/**
* Dialog for executing the <tt>GRANT MEMBER</tt> command.
*
* This dialog allows to remove project membership from existing members.
*/
class RemoveMemberDialog : public CommandDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param users Identifiers of the users.
    * @param projectId Identifier of the project.
    * @param parent The parent widget.
    */
    RemoveMemberDialog( const QList<int>& users, int projectId, QWidget* parent );

    /**
    * Destructor.
    */
    ~RemoveMemberDialog();

public: // overrides
    void accept();

private:
    QList<int> m_users;
    int m_projectId;
};

/**
* Dialog for executing the <tt>GRANT MEMBER</tt> command.
*
* This dialog allows to select new projects for a user.
*/
class AddUserProjectsDialog : public CommandDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param userId Identifier of the user.
    * @param parent The parent widget.
    */
    AddUserProjectsDialog( int userId, QWidget* parent );

    /**
    * Destructor.
    */
    ~AddUserProjectsDialog();

public: // overrides
    void accept();

private slots:
    void allProjectsActivated();
    void noProjectsActivated();

private:
    int m_userId;

    QListWidget* m_list;
    QButtonGroup* m_accessGroup;
};

/**
* Dialog for executing the <tt>GRANT MEMBER</tt> command.
*
* This dialog allows to change the access level for existing projects.
*/
class ChangeUserProjectsAccessDialog : public CommandDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param userId Identifier of the user.
    * @param projects Identifiers of the projects.
    * @param parent The parent widget.
    */
    ChangeUserProjectsAccessDialog( int userId, const QList<int>& projects, QWidget* parent );

    /**
    * Destructor.
    */
    ~ChangeUserProjectsAccessDialog();

public: // overrides
    void accept();

private:
    int m_userId;
    QList<int> m_projects;

    QButtonGroup* m_accessGroup;
};

/**
* Dialog for executing the <tt>GRANT MEMBER</tt> command.
*
* This dialog allows to remove membership from projects.
*/
class RemoveUserProjectsDialog : public CommandDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param userId Identifier of the user.
    * @param projects Identifiers of the projects.
    * @param parent The parent widget.
    */
    RemoveUserProjectsDialog( int userId, const QList<int>& projects, QWidget* parent );

    /**
    * Destructor.
    */
    ~RemoveUserProjectsDialog();

public: // overrides
    void accept();

private:
    int m_userId;
    QList<int> m_projects;
};

#endif
