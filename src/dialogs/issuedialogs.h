/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2013 WebIssues Team
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

#ifndef ISSUEDIALOGS_H
#define ISSUEDIALOGS_H

#include "dialogs/commanddialog.h"

#include <QMap>

class InputLineEdit;
class AbstractValueEditor;
class SeparatorComboBox;

class QButtonGroup;
class QLineEdit;

/**
* Base class for dialogs for adding or editing issues.
*/
class IssueDialog : public CommandDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent The parent widget.
    */
    IssueDialog( QWidget* parent );

    /**
    * Destructor.
    */
    ~IssueDialog();

protected:
    bool initialize( int typeId, int projectId );

    void setIssueName( const QString& name );
    QString issueName() const;

    void setAttributeValue( int attributeId, const QString& value );
    QString attributeValue( int attributeId ) const;

    QList<int> attributeIds() const;

private:
    InputLineEdit* m_nameEdit;
    QMap<int, AbstractValueEditor*> m_editors;
};

/**
* Dialog for adding a new issue.
*/
class AddIssueDialog : public IssueDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param folderId The identifier of the folder containing the issue.
    * @param cloneIssueId Optional identifier of the issue to clone.
    * @param parent The parent widget.
    */
    AddIssueDialog( int folderId, int cloneIssueId, QWidget* parent );

    /**
    * Destructor.
    */
    ~AddIssueDialog();

public:
    /**
    * Return the identifier of the created issue.
    */
    int issueId() const { return m_issueId; }

public: // overrides
    void accept();

protected: // overrides
    bool batchSuccessful( AbstractBatch* batch );

private:
    int m_folderId;
    int m_issueId;

    QMap<int, QString> m_values;
};

/**
* Dialog for editing an issue.
*/
class EditIssueDialog : public IssueDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param issueId Identifier of the issue to edit.
    * @param parent The parent widget.
    */
    EditIssueDialog( int issueId, QWidget* parent );

    /**
    * Destructor.
    */
    ~EditIssueDialog();

public:
    /**
    * If @c true, update folder instead of issue details after making the changes.
    */
    void setUpdateFolder( bool update );

public: // overrides
    void accept();

private:
    int m_issueId;

    bool m_updateFolder;

    QString m_oldName;
    QMap<int, QString> m_values;
};

/**
* Base class for dialogs for moving and cloning issues.
*/
class TransferIssueDialog : public CommandDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent The parent widget.
    */
    TransferIssueDialog( QWidget* parent );

    /**
    * Destructor.
    */
    ~TransferIssueDialog();

public:
    int folderId() const;

protected:
    void initialize( int typeId, int folderId, bool requireAdministrator );

private:
    SeparatorComboBox* m_folderCombo;
};

/**
* Dialog for selecting destination folder for cloned issue.
*/
class CloneIssueDialog : public TransferIssueDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param issueId Identifier of the isssue to clone.
    * @param parent The parent widget.
    */
    CloneIssueDialog( int issueId, QWidget* parent );

    /**
    * Destructor.
    */
    ~CloneIssueDialog();
};

/**
* Dialog for executing the <tt>MOVE ISSUE</tt> command.
*/
class MoveIssueDialog : public TransferIssueDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param issueId Identifier of the isssue to move.
    * @param parent The parent widget.
    */
    MoveIssueDialog( int issueId, QWidget* parent );

    /**
    * Destructor.
    */
    ~MoveIssueDialog();

public:
    /**
    * If @c true, update folder instead of issue details after making the changes.
    */
    void setUpdateFolder( bool update );

public: // overrides
    void accept();

private:
    int m_issueId;

    bool m_updateFolder;

    int m_oldFolderId;
};

/**
* Dialog for executing the <tt>DELETE ISSUE</tt> command.
*/
class DeleteIssueDialog : public CommandDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param issueId Identifier of the issue to delete.
    * @param parent The parent widget.
    */
    DeleteIssueDialog( int issueId, QWidget* parent );

    /**
    * Destructor.
    */
    ~DeleteIssueDialog();

public: // overrides
    void accept();

private:
    int m_issueId;
};

/**
* Dialog for executing the <tt>DELETE COMMENT</tt> command.
*/
class DeleteCommentDialog : public CommandDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param commentId Identifier of the comment to delete.
    * @param parent The parent widget.
    */
    DeleteCommentDialog( int commentId, QWidget* parent );

    /**
    * Destructor.
    */
    ~DeleteCommentDialog();

public: // overrides
    void accept();

private:
    int m_commentId;
};

/**
* Dialog for executing the <tt>ADD ATTACHMENT</tt> command.
*/
class AddAttachmentDialog : public CommandDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param issueId Identifier of the issue containing the attachment.
    * @param path Physical path of the file to attach.
    * @param url User visible URL of the file to attach.
    * @param parent The parent widget.
    */
    AddAttachmentDialog( int issueId, const QString& path, const QString& url, QWidget* parent );

    /**
    * Destructor.
    */
    ~AddAttachmentDialog();

public: // overrides
    void accept();

protected: // overrides
    bool batchSuccessful( AbstractBatch* batch );

private slots:
    void uploadProgress( int done );

private:
    int m_issueId;

    QString m_path;
    int m_size;

    InputLineEdit* m_fileEdit;
    InputLineEdit* m_descriptionEdit;
};

/**
* Dialog for executing the <tt>GET ATTACHMENT</tt> command.
*/
class GetAttachmentDialog : public CommandDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param fileId Identifier of the file to download.
    * @param path Physical path of the file to save.
    * @param url User visible URL of the file to save.
    * @param parent The parent widget.
    */
    GetAttachmentDialog( int fileId, const QString& path, const QString& url, QWidget* parent );

    /**
    * Destructor.
    */
    ~GetAttachmentDialog();

public:
    /**
    * Start the download automatically.
    */
    void download();

public: // overrides
    void accept();

protected: // overrides
    bool batchSuccessful( AbstractBatch* batch );

private slots:
    void downloadProgress( int done );

private:
    int m_fileId;

    QString m_path;
    int m_size;
};

/**
* Dialog for executing the <tt>EDIT ATTACHMENT</tt> command.
*/
class EditAttachmentDialog : public CommandDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param fileId Identifier of the file to edit.
    * @param parent The parent widget.
    */
    EditAttachmentDialog( int fileId, QWidget* parent );

    /**
    * Destructor.
    */
    ~EditAttachmentDialog();

public: // overrides
    void accept();

private:
    int m_fileId;

    QString m_oldName;
    QString m_oldDescription;

    InputLineEdit* m_fileEdit;
    InputLineEdit* m_descriptionEdit;
};

/**
* Dialog for executing the <tt>DELETE ATTACHMENT</tt> command.
*/
class DeleteAttachmentDialog : public CommandDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param fileId Identifier of the file to delete.
    * @param parent The parent widget.
    */
    DeleteAttachmentDialog( int fileId, QWidget* parent );

    /**
    * Destructor.
    */
    ~DeleteAttachmentDialog();

public: // overrides
    void accept();

private:
    int m_fileId;
};

#endif
