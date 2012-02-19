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

#ifndef PROJECTDIALOGS_H
#define PROJECTDIALOGS_H

#include "dialogs/commanddialog.h"

class InputLineEdit;
class QComboBox;

/**
* Dialog for executing the <tt>ADD PROJECT</tt> command.
*/
class AddProjectDialog : public CommandDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent The parent widget.
    */
    AddProjectDialog( QWidget* parent );

    /**
    * Destructor.
    */
    ~AddProjectDialog();

public: // overrides
    void accept();

private:
    InputLineEdit* m_nameEdit;
};

/**
* Dialog for executing the <tt>RENAME PROJECT</tt> command.
*/
class RenameProjectDialog : public CommandDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param projectId Identifier of the project to rename.
    * @param parent The parent widget.
    */
    RenameProjectDialog( int projectId, QWidget* parent );

    /**
    * Destructor.
    */
    ~RenameProjectDialog();

public: // overrides
    void accept();

private:
    int m_projectId;
    QString m_oldName;

    InputLineEdit* m_nameEdit;
};

/**
* Dialog for executing the <tt>DELETE PROJECT</tt> command.
*/
class DeleteProjectDialog : public CommandDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param projectId Identifier of the project to delete.
    * @param parent The parent widget.
    */
    DeleteProjectDialog( int projectId, QWidget* parent );

    /**
    * Destructor.
    */
    ~DeleteProjectDialog();

public: // overrides
    void accept();

protected: // overrides
    bool batchFailed( AbstractBatch* batch );

private:
    int m_projectId;

    bool m_force;
};

/**
* Dialog for executing the <tt>ADD FOLDER</tt> command.
*/
class AddFolderDialog : public CommandDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param projectId Identifier of the project containing the folder.
    * @param parent The parent widget.
    */
    AddFolderDialog( int projectId, QWidget* parent );

    /**
    * Destructor.
    */
    ~AddFolderDialog();

public: // overrides
    void accept();

private:
    int m_projectId;

    InputLineEdit* m_nameEdit;
    QComboBox* m_typeCombo;
};

/**
* Dialog for executing the <tt>RENAME FOLDER</tt> command.
*/
class RenameFolderDialog : public CommandDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param folderId Identifier of the folder to rename.
    * @param parent The parent widget.
    */
    RenameFolderDialog( int folderId, QWidget* parent );

    /**
    * Destructor.
    */
    ~RenameFolderDialog();

public: // overrides
    void accept();

private:
    int m_folderId;
    QString m_oldName;

    InputLineEdit* m_nameEdit;
};

/**
* Dialog for executing the <tt>MOVE FOLDER</tt> command.
*/
class MoveFolderDialog : public CommandDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param folderId Identifier of the folder to move.
    * @param parent The parent widget.
    */
    MoveFolderDialog( int folderId, QWidget* parent );

    /**
    * Destructor.
    */
    ~MoveFolderDialog();

public: // overrides
    void accept();

private:
    int m_folderId;

    int m_oldProjectId;

    QComboBox* m_projectCombo;
};

/**
* Dialog for executing the <tt>DELETE FOLDER</tt> command.
*/
class DeleteFolderDialog : public CommandDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param folderId Identifier of the folder to delete.
    * @param parent The parent widget.
    */
    DeleteFolderDialog( int folderId, QWidget* parent );

    /**
    * Destructor.
    */
    ~DeleteFolderDialog();

public: // overrides
    void accept();

protected: // overrides
    bool batchFailed( AbstractBatch* batch );

private:
    int m_folderId;

    bool m_force;
};

#endif
