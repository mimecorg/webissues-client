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

#include "issuedialogs.h"

#include "application.h"
#include "commands/issuebatch.h"
#include "data/datamanager.h"
#include "data/localsettings.h"
#include "rdb/utilities.h"
#include "utils/definitioninfo.h"
#include "utils/tablemodelshelper.h"
#include "utils/viewsettingshelper.h"
#include "utils/attributehelper.h"
#include "utils/iconloader.h"
#include "widgets/inputlineedit.h"
#include "widgets/abstractvalueeditor.h"
#include "widgets/valueeditorfactory.h"
#include "widgets/separatorcombobox.h"

#include <QLayout>
#include <QLabel>
#include <QRadioButton>
#include <QGroupBox>
#include <QScrollArea>
#include <QButtonGroup>
#include <QLineEdit>
#include <QFileInfo>
#include <QRegExp>
#include <QMessageBox>

IssueDialog::IssueDialog( QWidget* parent ) : CommandDialog( parent ),
    m_nameEdit( NULL )
{
}

IssueDialog::~IssueDialog()
{
    application->applicationSettings()->setValue( "IssueDialogSize", size() );
}

void IssueDialog::initialize( int typeId, int projectId )
{
    QGridLayout* layout = new QGridLayout();

    QLabel* nameLabel = new QLabel( tr( "&Name:" ), this );
    layout->addWidget( nameLabel, 0, 0 );

    m_nameEdit = new InputLineEdit( this );
    m_nameEdit->setMaxLength( 80 );
    m_nameEdit->setRequired( true );
    layout->addWidget( m_nameEdit, 0, 1 );

    nameLabel->setBuddy( m_nameEdit );

    QList<int> attributes = ViewSettingsHelper::attributeOrder( typeId, dataManager->viewSetting( typeId, "attribute_order" ) );

    if ( !attributes.isEmpty() ) {
        QGroupBox* attributeGroup = new QGroupBox( tr( "Attributes" ), this );
        QVBoxLayout* attributeLayout = new QVBoxLayout( attributeGroup );
        layout->addWidget( attributeGroup, 1, 0, 1, 2 );

        QScrollArea* attributeScroll = new QScrollArea( attributeGroup );
        attributeScroll->setWidgetResizable( true );
        attributeLayout->addWidget( attributeScroll );

        QPalette scrollPalette = palette();
        scrollPalette.setColor( QPalette::Window, QColor::fromRgb( 255, 255, 255 ) );
        attributeScroll->setPalette( scrollPalette );

        QWidget* attributePanel = new QWidget( attributeScroll );
        attributeScroll->setWidget( attributePanel );

        QGridLayout* valuesLayout = new QGridLayout( attributePanel );

        valuesLayout->setColumnMinimumWidth( 0, 120 );

        QLabel* nameLabel = new QLabel( tr( "Name" ), attributePanel );
        valuesLayout->addWidget( nameLabel, 0, 0 );

        QLabel* valueLabel = new QLabel( tr( "Value" ), attributePanel );
        valuesLayout->addWidget( valueLabel, 0, 1 );

        QFrame* separator = new QFrame( attributePanel );
        separator->setFrameStyle( QFrame::HLine | QFrame::Sunken );
        valuesLayout->addWidget( separator, 1, 0, 1, 2 );

        for ( int i = 0; i < attributes.count(); i++ ) {
            int attributeId = attributes.at( i );
            const AttributeRow* attribute = dataManager->attributes()->find( attributeId );

            QLabel* headerLabel = new QLabel( attribute->name(), attributePanel );
            valuesLayout->addWidget( headerLabel, i + 2, 0 );

            DefinitionInfo info = DefinitionInfo::fromString( attribute->definition() );

            AbstractValueEditor* editor = ValueEditorFactory::createValueEditor( info, projectId, this, attributePanel );
            valuesLayout->addWidget( editor->widget(), i + 2, 1 );

            m_editors.insert( attributeId, editor );
        }

        valuesLayout->setRowStretch( attributes.count() + 2, 1 );

        setContentLayout( layout, false );
    } else {
        setContentLayout( layout, true );
    }

    m_nameEdit->setFocus();

    resize( application->applicationSettings()->value( "IssueDialogSize", QSize( 500, 500 ) ).toSize() );
}

void IssueDialog::setIssueName( const QString& name )
{
    m_nameEdit->setInputValue( name );
}

QString IssueDialog::issueName() const
{
    return m_nameEdit->inputValue();
}

void IssueDialog::setAttributeValue( int attributeId, const QString& value )
{
    AbstractValueEditor* editor = m_editors.value( attributeId );
    if ( editor )
        editor->setInputValue( value );
}

QString IssueDialog::attributeValue( int attributeId ) const
{
    AbstractValueEditor* editor = m_editors.value( attributeId );
    if ( editor )
        return editor->inputValue();
    return QString();
}

QList<int> IssueDialog::attributeIds() const
{
    return m_editors.keys();
}

AddIssueDialog::AddIssueDialog( int folderId, QWidget* parent ) : IssueDialog( parent ),
    m_folderId( folderId )
{
    const FolderRow* folder = dataManager->folders()->find( folderId );
    QString name = folder ? folder->name() : QString();
    int typeId = folder ? folder->typeId() : 0;
    int projectId = folder ? folder->projectId() : 0;

    setWindowTitle( tr( "Add Issue" ) );
    setPrompt( tr( "Create a new issue in folder <b>%1</b>:" ).arg( name ) );
    setPromptPixmap( IconLoader::pixmap( "issue-new", 22 ) );

    initialize( typeId, projectId );

    QList<int> attributes = attributeIds();

    for ( int i = 0; i < attributes.count(); i++ ) {
        int attributeId = attributes.at( i );
        const AttributeRow* attribute = dataManager->attributes()->find( attributeId );

        DefinitionInfo info = DefinitionInfo::fromString( attribute->definition() );
        QString value = info.metadata( "default" ).toString();

        value = AttributeHelper::convertInitialValue( info, value );

        setAttributeValue( attributeId, value );
        m_values.insert( attributeId, value );
    }
}

AddIssueDialog::~AddIssueDialog()
{
}

void AddIssueDialog::accept()
{
    if ( !validate() )
        return;

    QString name = issueName();
    IssueBatch* batch = new IssueBatch( m_folderId, name );

    for ( QMap<int, QString>::const_iterator it = m_values.begin(); it != m_values.end(); ++it ) {
        QString value = attributeValue( it.key() );
        if ( value != it.value() )
            batch->setValue( it.key(), value );
    }

    executeBatch( batch );
}

bool AddIssueDialog::batchSuccessful( AbstractBatch* batch )
{
    m_issueId = ( (IssueBatch*)batch )->issueId();

    return true;
}

EditIssueDialog::EditIssueDialog( int issueId, QWidget* parent ) : IssueDialog( parent ),
    m_issueId( issueId )
{
    const IssueRow* issue = dataManager->issues()->find( issueId );
    m_oldName = issue ? issue->name() : QString();
    int folderId = issue ? issue->folderId() : 0;

    const FolderRow* folder = dataManager->folders()->find( folderId );
    int typeId = folder ? folder->typeId() : 0;
    int projectId = folder ? folder->projectId() : 0;

    setWindowTitle( tr( "Edit Attributes" ) );
    setPrompt( tr( "Edit attributes of issue <b>%1</b>:" ).arg( m_oldName ) );
    setPromptPixmap( IconLoader::pixmap( "edit-modify", 22 ) );

    initialize( typeId, projectId );

    setIssueName( m_oldName );

    QList<int> attributes = attributeIds();

    for ( int i = 0; i < attributes.count(); i++ ) {
        int attributeId = attributes.at( i );
        const ValueRow* row = dataManager->values()->find( attributeId, issueId );
        QString value = row ? row->value() : QString();

        setAttributeValue( attributeId, value );
        m_values.insert( attributeId, value );
    }
}

EditIssueDialog::~EditIssueDialog()
{
}

void EditIssueDialog::accept()
{
    if ( !validate() )
        return;

    IssueBatch* batch = NULL;

    QString name = issueName();
    if ( name != m_oldName ) {
        batch = new IssueBatch( m_issueId );
        batch->renameIssue( name );
    }

    for ( QMap<int, QString>::const_iterator it = m_values.begin(); it != m_values.end(); ++it ) {
        QString value = attributeValue( it.key() );
        if ( value != it.value() ) {
            if ( !batch )
                batch = new IssueBatch( m_issueId );
            batch->setValue( it.key(), value );
        }
    }

    if ( batch )
        executeBatch( batch );
    else
        QDialog::accept();
}

MoveIssueDialog::MoveIssueDialog( int issueId, QWidget* parent ) : CommandDialog( parent ),
    m_issueId( issueId )
{
    const IssueRow* issue = dataManager->issues()->find( issueId );
    QString name = issue ? issue->name() : QString();
    m_oldFolderId = issue ? issue->folderId() : 0;
    const FolderRow* folder = dataManager->folders()->find( m_oldFolderId );
    int typeId = folder ? folder->typeId() : 0;

    setWindowTitle( tr( "Move Issue" ) );
    setPrompt( tr( "Move issue <b>%1</b> to another folder of the same type:" ).arg( name ) );
    setPromptPixmap( IconLoader::pixmap( "issue-move", 22 ) );

    QGridLayout* layout = new QGridLayout();

    QLabel* folderLabel = new QLabel( tr( "&Folder:" ), this );
    layout->addWidget( folderLabel, 0, 0 );

    m_folderCombo = new SeparatorComboBox( this );
    layout->addWidget( m_folderCombo, 0, 1 );

    folderLabel->setBuddy( m_folderCombo );

    layout->setColumnStretch( 1, 1 );

    RDB::IndexConstIterator<ProjectRow> itp( dataManager->projects()->index() );
    QList<const ProjectRow*> projects = localeAwareSortRows( itp, &ProjectRow::name );

    for ( int i = 0; i < projects.count(); i++ ) {
        const ProjectRow* project = projects.at( i );

        if ( !TableModelsHelper::isProjectAdmin( project->projectId() ) )
            continue;

        RDB::ForeignConstIterator<FolderRow> itf( dataManager->folders()->parentIndex(), project->projectId() );
        QList<const FolderRow*> folders = localeAwareSortRows( itf, &FolderRow::name );

        bool projectAdded = false;
        for ( int j = 0; j < folders.count(); j++ ) {
            const FolderRow* folder = folders.at( j );
            if ( folder->typeId() != typeId )
                continue;
            if ( !projectAdded ) {
                m_folderCombo->addParentItem( project->name() );
                projectAdded = true;
            }
            m_folderCombo->addChildItem( folder->name(), folder->folderId() );
            if ( folder->folderId() == m_oldFolderId )
                m_folderCombo->setCurrentIndex( m_folderCombo->count() - 1 );
        }
    }

    setContentLayout( layout, true );

    m_folderCombo->setFocus();
}

MoveIssueDialog::~MoveIssueDialog()
{
}

void MoveIssueDialog::accept()
{
    if ( !validate() )
        return;

    int folderId = m_folderCombo->itemData( m_folderCombo->currentIndex() ).toInt();

    if ( folderId == m_oldFolderId ) {
        QDialog::accept();
        return;
    }

    IssueBatch* batch = new IssueBatch( m_issueId );
    batch->moveIssue( folderId );

    executeBatch( batch );
}

DeleteIssueDialog::DeleteIssueDialog( int issueId, QWidget* parent ) : CommandDialog( parent ),
    m_issueId( issueId )
{
    const IssueRow* issue = dataManager->issues()->find( issueId );
    QString name = issue ? issue->name() : QString();

    setWindowTitle( tr( "Delete Issue" ) );
    setPrompt( tr( "Are you sure you want to delete issue <b>%1</b>?" ).arg( name ) );
    setPromptPixmap( IconLoader::pixmap( "edit-delete", 22 ) );

    showWarning( tr( "The entire issue history will be permanently deleted." ) );

    setContentLayout( NULL, true );
}

DeleteIssueDialog::~DeleteIssueDialog()
{
}

void DeleteIssueDialog::accept()
{
    IssueBatch* batch = new IssueBatch( m_issueId );
    batch->deleteIssue();

    executeBatch( batch );
}

DeleteCommentDialog::DeleteCommentDialog( int commentId, QWidget* parent ) : CommandDialog( parent ),
    m_commentId( commentId )
{
    QString id = TableModelsHelper::formatId( commentId );

    setWindowTitle( tr( "Delete Comment" ) );
    setPrompt( tr( "Are you sure you want to delete comment <b>%1</b>?" ).arg( id ) );
    setPromptPixmap( IconLoader::pixmap( "edit-delete", 22 ) );

    setContentLayout( NULL, true );
}

DeleteCommentDialog::~DeleteCommentDialog()
{
}

void DeleteCommentDialog::accept()
{
    const ChangeRow* change = dataManager->changes()->find( m_commentId );
    int issueId = change ? change->issueId() : 0;

    IssueBatch* batch = new IssueBatch( issueId );
    batch->deleteComment( m_commentId );

    executeBatch( batch );
}

AddAttachmentDialog::AddAttachmentDialog( int issueId, const QString& path, const QString& url, QWidget* parent ) : CommandDialog( parent ),
    m_issueId( issueId ),
    m_path( path )
{
    QFileInfo info( url );
    QString baseName = info.completeBaseName();
    QString suffix = info.suffix();

    QString fileName;
    if ( suffix.isEmpty() ) {
        baseName.truncate( 40 );
        fileName = baseName.trimmed();
    } else {
        baseName.truncate( 40 - suffix.length() - 1 );
        fileName = baseName.trimmed() + '.' + suffix;
    }

    if ( fileName != info.fileName() ) {
        QMessageBox::warning( parent, tr( "Warning" ),
            tr( "The name of the selected file is longer than %1 characters and will be truncated." ).arg( 40 ) );
    }

    const IssueRow* issue = dataManager->issues()->find( issueId );
    QString name = issue ? issue->name() : QString();

    setWindowTitle( tr( "Add Attachment" ) );
    setPrompt( tr( "Add an attachment to issue <b>%1</b>:" ).arg( name ) );
    setPromptPixmap( IconLoader::pixmap( "file-attach", 22 ) );

    QGridLayout* layout = new QGridLayout();

    QLabel* fileLabel = new QLabel( tr( "&File name:" ), this );
    layout->addWidget( fileLabel, 0, 0 );

    m_fileEdit = new InputLineEdit( this );
    m_fileEdit->setMaxLength( 40 );
    m_fileEdit->setFormat( InputLineEdit::FileNameFormat );
    m_fileEdit->setInputValue( fileName );
    layout->addWidget( m_fileEdit, 0, 1 );

    fileLabel->setBuddy( m_fileEdit );

    QLabel* descriptionLabel = new QLabel( tr( "&Description:" ), this );
    layout->addWidget( descriptionLabel, 1, 0 );

    m_descriptionEdit = new InputLineEdit( this );
    m_descriptionEdit->setMaxLength( 80 );
    layout->addWidget( m_descriptionEdit, 1, 1 );

    descriptionLabel->setBuddy( m_descriptionEdit );

    setContentLayout( layout, true );

    QFileInfo fileInfo( path );
    m_size = fileInfo.size();

    createProgressPanel( m_size, tr( "Size: %1" ).arg( TableModelsHelper::formatSize( m_size ) ) );

    m_descriptionEdit->setFocus();
}

AddAttachmentDialog::~AddAttachmentDialog()
{
}

void AddAttachmentDialog::accept()
{
    if ( !validate() )
        return;

    QString fileName = m_fileEdit->inputValue();
    QString description = m_descriptionEdit->inputValue();

    IssueBatch* batch = new IssueBatch( m_issueId );
    batch->addAttachment( fileName, description, m_path );

    connect( batch, SIGNAL( uploadProgress( int ) ), this, SLOT( uploadProgress( int ) ) );

    uploadProgress( 0 );

    executeBatch( batch );
}

void AddAttachmentDialog::uploadProgress( int done )
{
    QString uploaded = TableModelsHelper::formatSize( done );
    QString total = TableModelsHelper::formatSize( m_size );

    setProgress( done, tr( "Uploaded: %1 of %2" ).arg( uploaded, total ) );
}

bool AddAttachmentDialog::batchSuccessful( AbstractBatch* batch )
{
    int error = ( (IssueBatch*)batch )->fileError();

    if ( error != QFile::NoError ) {
        showError( tr( "File could not be read." ) );
        return false;
    }

    return true;
}

GetAttachmentDialog::GetAttachmentDialog( int fileId, const QString& path, const QString& url, QWidget* parent ) : CommandDialog( parent ),
    m_fileId( fileId ),
    m_path( path )
{
    const FileRow* file = dataManager->files()->find( fileId );
    QString name = file ? file->name() : QString();
    QString description = file ? file->description() : QString();
    m_size = file ? file->size() : 0;

    setWindowTitle( tr( "Download" ) );
    if ( !url.isEmpty() ) {
        setPrompt( tr( "Download attachment <b>%1</b>:" ).arg( name ) );
        setPromptPixmap( IconLoader::pixmap( "file-save-as", 22 ) );
    } else {
        setPrompt( tr( "Open attachment <b>%1</b>:" ).arg( name ) );
        setPromptPixmap( IconLoader::pixmap( "file-open", 22 ) );
    }

    QGridLayout* layout = new QGridLayout();

    QLabel* fileLabel = new QLabel( tr( "Destination:" ), this );
    layout->addWidget( fileLabel, 0, 0 );

    InputLineEdit* fileEdit = new InputLineEdit( this );
    fileEdit->setReadOnly( true );
    if ( url.isEmpty() )
        fileEdit->setInputValue( path );
    else
        fileEdit->setInputValue( url );
    layout->addWidget( fileEdit, 0, 1 );

    QLabel* descriptionLabel = new QLabel( tr( "Description:" ), this );
    layout->addWidget( descriptionLabel, 1, 0 );

    InputLineEdit* descriptionEdit = new InputLineEdit( this );
    descriptionEdit->setReadOnly( true );
    descriptionEdit->setInputValue( description );
    layout->addWidget( descriptionEdit, 1, 1 );

    setContentLayout( layout, true );

    createProgressPanel( m_size, tr( "Size: %1" ).arg( TableModelsHelper::formatSize( m_size ) ) );
}

GetAttachmentDialog::~GetAttachmentDialog()
{
}

void GetAttachmentDialog::download()
{
    accept();
}

void GetAttachmentDialog::accept()
{
    const ChangeRow* change = dataManager->changes()->find( m_fileId );
    int issueId = change ? change->issueId() : 0;

    IssueBatch* batch = new IssueBatch( issueId );
    batch->getAttachment( m_fileId, m_path );

    connect( batch, SIGNAL( downloadProgress( int ) ), this, SLOT( downloadProgress( int ) ) );

    downloadProgress( 0 );

    executeBatch( batch );
}

void GetAttachmentDialog::downloadProgress( int done )
{
    QString downloaded = TableModelsHelper::formatSize( done );
    QString total = TableModelsHelper::formatSize( m_size );

    setProgress( done, tr( "Downloaded: %1 of %2" ).arg( downloaded, total ) );
}

bool GetAttachmentDialog::batchSuccessful( AbstractBatch* batch )
{
    int error = ( (IssueBatch*)batch )->fileError();

    if ( error != QFile::NoError ) {
        showError( tr( "File could not be saved." ) );
        return false;
    }

    return true;
}

EditAttachmentDialog::EditAttachmentDialog( int fileId, QWidget* parent ) : CommandDialog( parent ),
    m_fileId( fileId )
{
    const FileRow* file = dataManager->files()->find( fileId );
    m_oldName = file ? file->name() : QString();
    m_oldDescription = file ? file->description() : QString();

    setWindowTitle( tr( "Edit Attachment" ) );
    setPrompt( tr( "Edit attachment <b>%1</b>:" ).arg( m_oldName ) );
    setPromptPixmap( IconLoader::pixmap( "edit-modify", 22 ) );

    QGridLayout* layout = new QGridLayout();

    QLabel* fileLabel = new QLabel( tr( "&File name:" ), this );
    layout->addWidget( fileLabel, 0, 0 );

    m_fileEdit = new InputLineEdit( this );
    m_fileEdit->setMaxLength( 40 );
    m_fileEdit->setFormat( InputLineEdit::FileNameFormat );
    m_fileEdit->setInputValue( m_oldName );
    layout->addWidget( m_fileEdit, 0, 1 );

    fileLabel->setBuddy( m_fileEdit );

    QLabel* descriptionLabel = new QLabel( tr( "&Description:" ), this );
    layout->addWidget( descriptionLabel, 1, 0 );

    m_descriptionEdit = new InputLineEdit( this );
    m_descriptionEdit->setMaxLength( 80 );
    m_descriptionEdit->setInputValue( m_oldDescription );
    layout->addWidget( m_descriptionEdit, 1, 1 );

    descriptionLabel->setBuddy( m_descriptionEdit );

    setContentLayout( layout, true );

    m_fileEdit->setFocus();
}

EditAttachmentDialog::~EditAttachmentDialog()
{
}

void EditAttachmentDialog::accept()
{
    if ( !validate() )
        return;

    QString fileName = m_fileEdit->inputValue();
    QString description = m_descriptionEdit->inputValue();

    if ( fileName == m_oldName && description == m_oldDescription ) {
        QDialog::accept();
        return;
    }

    const ChangeRow* change = dataManager->changes()->find( m_fileId );
    int issueId = change ? change->issueId() : 0;

    IssueBatch* batch = new IssueBatch( issueId );
    batch->editAttachment( m_fileId, fileName, description );

    executeBatch( batch );
}

DeleteAttachmentDialog::DeleteAttachmentDialog( int fileId, QWidget* parent ) : CommandDialog( parent ),
    m_fileId( fileId )
{
    const FileRow* file = dataManager->files()->find( fileId );
    QString name = file ? file->name() : QString();

    setWindowTitle( tr( "Delete Attachment" ) );
    setPrompt( tr( "Are you sure you want to delete attachment <b>%1</b>?" ).arg( name ) );
    setPromptPixmap( IconLoader::pixmap( "edit-delete", 22 ) );

    setContentLayout( NULL, true );
}

DeleteAttachmentDialog::~DeleteAttachmentDialog()
{
}

void DeleteAttachmentDialog::accept()
{
    const ChangeRow* change = dataManager->changes()->find( m_fileId );
    int issueId = change ? change->issueId() : 0;

    IssueBatch* batch = new IssueBatch( issueId );
    batch->deleteAttachment( m_fileId );

    executeBatch( batch );
}
