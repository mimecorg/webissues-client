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

#include "projectdialogs.h"

#include "commands/projectsbatch.h"
#include "commands/commandmanager.h"
#include "data/entities.h"
#include "utils/errorhelper.h"
#include "utils/iconloader.h"
#include "widgets/inputlineedit.h"
#include "widgets/markuptextedit.h"

#include <QLayout>
#include <QLabel>
#include <QComboBox>

AddProjectDialog::AddProjectDialog( QWidget* parent ) : CommandDialog( parent )
{
    setWindowTitle( tr( "Add Project" ) );
    setPrompt( tr( "Create a new project:" ) );
    setPromptPixmap( IconLoader::pixmap( "project-new", 22 ) );

    QHBoxLayout* layout = new QHBoxLayout();

    QLabel* label = new QLabel( tr( "&Name:" ), this );
    layout->addWidget( label, 0 );

    m_nameEdit = new InputLineEdit( this );
    m_nameEdit->setMaxLength( 40 );
    m_nameEdit->setRequired( true );
    layout->addWidget( m_nameEdit, 1 );

    label->setBuddy( m_nameEdit );

    setContentLayout( layout, true );

    m_nameEdit->setFocus();
}

AddProjectDialog::~AddProjectDialog()
{
}

void AddProjectDialog::accept()
{
    if ( !validate() )
        return;

    QString name = m_nameEdit->inputValue();

    if ( ProjectEntity::exists( name ) ) {
        showWarning( ErrorHelper::ProjectAlreadyExists );
        return;
    }

    ProjectsBatch* batch = new ProjectsBatch();
    batch->addProject( name );

    executeBatch( batch );
}

RenameProjectDialog::RenameProjectDialog( int projectId, QWidget* parent ) : CommandDialog( parent ),
    m_projectId( projectId )
{
    ProjectEntity project = ProjectEntity::find( projectId );
    m_oldName = project.name();

    setWindowTitle( tr( "Rename Project" ) );
    setPrompt( tr( "Enter the new name of project <b>%1</b>:" ).arg( m_oldName ) );
    setPromptPixmap( IconLoader::pixmap( "edit-rename", 22 ) );

    QHBoxLayout* layout = new QHBoxLayout();

    QLabel* label = new QLabel( tr( "&Name:" ), this );
    layout->addWidget( label, 0 );

    m_nameEdit = new InputLineEdit( this );
    m_nameEdit->setMaxLength( 40 );
    m_nameEdit->setRequired( true );
    m_nameEdit->setInputValue( m_oldName );
    layout->addWidget( m_nameEdit, 1 );

    label->setBuddy( m_nameEdit );

    setContentLayout( layout, true );

    m_nameEdit->setFocus();
}

RenameProjectDialog::~RenameProjectDialog()
{
}

void RenameProjectDialog::accept()
{
    if ( !validate() )
        return;

    QString name = m_nameEdit->inputValue();

    if ( name == m_oldName ) {
        QDialog::accept();
        return;
    }

    if ( ProjectEntity::exists( name ) ) {
        showWarning( ErrorHelper::ProjectAlreadyExists );
        return;
    }

    ProjectsBatch* batch = new ProjectsBatch();
    batch->renameProject( m_projectId, name );

    executeBatch( batch );
}

DeleteProjectDialog::DeleteProjectDialog( int projectId, QWidget* parent ) : CommandDialog( parent ),
    m_projectId( projectId ),
    m_force( false )
{
    ProjectEntity project = ProjectEntity::find( projectId );

    setWindowTitle( tr( "Delete Project" ) );
    setPrompt( tr( "Are you sure you want to delete project <b>%1</b>?" ).arg( project.name() ) );
    setPromptPixmap( IconLoader::pixmap( "edit-delete", 22 ) );

    if ( !project.folders().isEmpty() ) {
        showWarning( tr( "All folders and issues in this project will be permanently deleted." ) );
        m_force = true;
    }

    setContentLayout( NULL, true );
}

DeleteProjectDialog::~DeleteProjectDialog()
{
}

void DeleteProjectDialog::accept()
{
    ProjectsBatch* batch = new ProjectsBatch();
    batch->deleteProject( m_projectId, m_force );

    executeBatch( batch );
}

bool DeleteProjectDialog::batchFailed( AbstractBatch* /*batch*/ )
{
    if ( commandManager->error() == CommandManager::WebIssuesError && commandManager->errorCode() == ErrorHelper::CannotDeleteProject ) {
        showWarning( tr( "All folders and issues in this project will be permanently deleted." ) );
        m_force = true;
    }

    return true;
}

AddFolderDialog::AddFolderDialog( int projectId, QWidget* parent ) : CommandDialog( parent ),
    m_projectId( projectId ),
    m_nameEdit( NULL ),
    m_typeCombo( NULL )
{
    ProjectEntity project = ProjectEntity::find( projectId );

    setWindowTitle( tr( "Add Folder" ) );
    setPrompt( tr( "Create a new folder in project <b>%1</b>:" ).arg( project.name() ) );
    setPromptPixmap( IconLoader::pixmap( "folder-new", 22 ) );

    QList<TypeEntity> types = TypeEntity::list();

    if ( types.isEmpty() ) {
        showWarning( tr( "There are no available issue types to use." ) );
        showCloseButton();
        setContentLayout( NULL, true );
        return;
    }

    QGridLayout* layout = new QGridLayout();

    QLabel* nameLabel = new QLabel( tr( "&Name:" ), this );
    layout->addWidget( nameLabel, 0, 0 );

    m_nameEdit = new InputLineEdit( this );
    m_nameEdit->setMaxLength( 40 );
    m_nameEdit->setRequired( true );
    layout->addWidget( m_nameEdit, 0, 1 );

    nameLabel->setBuddy( m_nameEdit );

    QLabel* typeLabel = new QLabel( tr( "&Type:" ), this );
    layout->addWidget( typeLabel, 1, 0 );

    m_typeCombo = new QComboBox( this );
    layout->addWidget( m_typeCombo, 1, 1 );

    typeLabel->setBuddy( m_typeCombo );

    for ( int i = 0; i < types.count(); i++ )
        m_typeCombo->addItem( types.at( i ).name(), types.at( i ).id() );

    setContentLayout( layout, true );

    m_nameEdit->setFocus();
}

AddFolderDialog::~AddFolderDialog()
{
}

void AddFolderDialog::accept()
{
    if ( !validate() )
        return;

    QString name = m_nameEdit->inputValue();

    if ( FolderEntity::exists( m_projectId, name ) ) {
        showWarning( ErrorHelper::FolderAlreadyExists );
        return;
    }

    int typeId = m_typeCombo->itemData( m_typeCombo->currentIndex() ).toInt();

    ProjectsBatch* batch = new ProjectsBatch();
    batch->addFolder( m_projectId, typeId, name );

    executeBatch( batch );
}

RenameFolderDialog::RenameFolderDialog( int folderId, QWidget* parent ) : CommandDialog( parent ),
    m_folderId( folderId )
{
    FolderEntity folder = FolderEntity::find( folderId );
    m_oldName = folder.name();

    setWindowTitle( tr( "Rename Folder" ) );
    setPrompt( tr( "Enter the new name of folder <b>%1</b>:" ).arg( m_oldName ) );
    setPromptPixmap( IconLoader::pixmap( "edit-rename", 22 ) );

    QHBoxLayout* layout = new QHBoxLayout();

    QLabel* label = new QLabel( tr( "&Name:" ), this );
    layout->addWidget( label, 0 );

    m_nameEdit = new InputLineEdit( this );
    m_nameEdit->setMaxLength( 40 );
    m_nameEdit->setRequired( true );
    m_nameEdit->setInputValue( m_oldName );
    layout->addWidget( m_nameEdit, 1 );

    label->setBuddy( m_nameEdit );

    setContentLayout( layout, true );

    m_nameEdit->setFocus();
}

RenameFolderDialog::~RenameFolderDialog()
{
}

void RenameFolderDialog::accept()
{
    if ( !validate() )
        return;

    QString name = m_nameEdit->inputValue();

    if ( name == m_oldName ) {
        QDialog::accept();
        return;
    }

    FolderEntity folder = FolderEntity::find( m_folderId );

    if ( FolderEntity::exists( folder.projectId(), name ) ) {
        showWarning( ErrorHelper::FolderAlreadyExists );
        return;
    }

    ProjectsBatch* batch = new ProjectsBatch();
    batch->renameFolder( m_folderId, name );

    executeBatch( batch );
}

MoveFolderDialog::MoveFolderDialog( int folderId, QWidget* parent ) : CommandDialog( parent ),
    m_folderId( folderId )
{
    FolderEntity folder = FolderEntity::find( folderId );
    m_oldProjectId = folder.projectId();

    setWindowTitle( tr( "Move Folder" ) );
    setPrompt( tr( "Move folder <b>%1</b> to another project:" ).arg( folder.name() ) );
    setPromptPixmap( IconLoader::pixmap( "folder-move", 22 ) );

    QList<ProjectEntity> projects;
    bool available = false;

    foreach ( const ProjectEntity& project, ProjectEntity::list() ) {
        if ( !ProjectEntity::isAdmin( project.id() ) )
            continue;
        projects.append( project );
        if ( project.id() != m_oldProjectId )
            available = true;
    }

    if ( !available ) {
        showWarning( tr( "There are no available destination projects." ) );
        showCloseButton();
        setContentLayout( NULL, true );
        return;
    }

    QGridLayout* layout = new QGridLayout();

    QLabel* projectLabel = new QLabel( tr( "&Project:" ), this );
    layout->addWidget( projectLabel, 0, 0 );

    m_projectCombo = new QComboBox( this );
    layout->addWidget( m_projectCombo, 0, 1 );

    projectLabel->setBuddy( m_projectCombo );

    layout->setColumnStretch( 1, 1 );

    foreach ( const ProjectEntity& project, projects ) {
        m_projectCombo->addItem( project.name(), project.id() );
        if ( project.id() == m_oldProjectId )
            m_projectCombo->setCurrentIndex( m_projectCombo->count() - 1 );
    }

    setContentLayout( layout, true );

    m_projectCombo->setFocus();
}

MoveFolderDialog::~MoveFolderDialog()
{
}

void MoveFolderDialog::accept()
{
    if ( !validate() )
        return;

    int projectId = m_projectCombo->itemData( m_projectCombo->currentIndex() ).toInt();

    if ( projectId == m_oldProjectId ) {
        QDialog::accept();
        return;
    }

    FolderEntity folder = FolderEntity::find( m_folderId );

    if ( FolderEntity::exists( projectId, folder.name() ) ) {
        showWarning( ErrorHelper::FolderAlreadyExists );
        return;
    }

    ProjectsBatch* batch = new ProjectsBatch();
    batch->moveFolder( m_folderId, projectId );

    executeBatch( batch );
}

DeleteFolderDialog::DeleteFolderDialog( int folderId, QWidget* parent ) : CommandDialog( parent ),
    m_folderId( folderId ),
    m_force( false )
{
    FolderEntity folder = FolderEntity::find( folderId );

    setWindowTitle( tr( "Delete Folder" ) );
    setPrompt( tr( "Are you sure you want to delete folder <b>%1</b>?" ).arg( folder.name() ) );
    setPromptPixmap( IconLoader::pixmap( "edit-delete", 22 ) );

    if ( !folder.issues().isEmpty() ) {
        showWarning( tr( "All issues in this folder will be permanently deleted." ) );
        m_force = true;
    }

    setContentLayout( NULL, true );
}

DeleteFolderDialog::~DeleteFolderDialog()
{
}

void DeleteFolderDialog::accept()
{
    ProjectsBatch* batch = new ProjectsBatch();
    batch->deleteFolder( m_folderId, m_force );

    executeBatch( batch );
}

bool DeleteFolderDialog::batchFailed( AbstractBatch* /*batch*/ )
{
    if ( commandManager->error() == CommandManager::WebIssuesError && commandManager->errorCode() == ErrorHelper::CannotDeleteFolder ) {
        showWarning( tr( "All issues in this folder will be permanently deleted." ) );
        m_force = true;
    }

    return true;
}

AddProjectDescriptionDialog::AddProjectDescriptionDialog( int projectId, QWidget* parent ) : CommandDialog( parent ),
    m_projectId( projectId )
{
    ProjectEntity project = ProjectEntity::find( projectId );

    setWindowTitle( tr( "Add Description" ) );
    setPrompt( tr( "Add description to project <b>%1</b>:" ).arg( project.name() ) );
    setPromptPixmap( IconLoader::pixmap( "description-new", 22 ) );

    QVBoxLayout* layout = new QVBoxLayout();

    m_descriptionEdit = new MarkupTextEdit( this );
    layout->addWidget( m_descriptionEdit );

    setContentLayout( layout, false );

    m_descriptionEdit->setFocus();
}

AddProjectDescriptionDialog::~AddProjectDescriptionDialog()
{
}

void AddProjectDescriptionDialog::accept()
{
    if ( !validate() )
        return;

    ProjectsBatch* batch = new ProjectsBatch();
    batch->addProjectDescription( m_projectId, m_descriptionEdit->inputValue(), m_descriptionEdit->textFormat() );

    executeBatch( batch );
}

EditProjectDescriptionDialog::EditProjectDescriptionDialog( int projectId, QWidget* parent ) : CommandDialog( parent ),
    m_projectId( projectId )
{
    ProjectEntity project = ProjectEntity::find( projectId );
    DescriptionEntity description = project.description();
    m_oldText = description.text();
    m_oldFormat = description.format();

    setWindowTitle( tr( "Edit Description" ) );
    setPrompt( tr( "Edit description of project <b>%1</b>:" ).arg( project.name() ) );
    setPromptPixmap( IconLoader::pixmap( "edit-modify", 22 ) );

    QVBoxLayout* layout = new QVBoxLayout();

    m_descriptionEdit = new MarkupTextEdit( this );
    layout->addWidget( m_descriptionEdit );

    setContentLayout( layout, false );

    m_descriptionEdit->setInputValue( m_oldText );
    m_descriptionEdit->setTextFormat( m_oldFormat );

    m_descriptionEdit->setFocus();
}

EditProjectDescriptionDialog::~EditProjectDescriptionDialog()
{
}

void EditProjectDescriptionDialog::accept()
{
    if ( !validate() )
        return;

    QString text = m_descriptionEdit->inputValue();
    TextFormat format = m_descriptionEdit->textFormat();

    if ( text == m_oldText && format == m_oldFormat ) {
        QDialog::accept();
        return;
    }

    ProjectsBatch* batch = new ProjectsBatch();
    batch->editProjectDescription( m_projectId, text, format );

    executeBatch( batch );
}

DeleteProjectDescriptionDialog::DeleteProjectDescriptionDialog( int projectId, QWidget* parent ) : CommandDialog( parent ),
    m_projectId( projectId )
{
    ProjectEntity project = ProjectEntity::find( projectId );

    setWindowTitle( tr( "Delete Description" ) );
    setPrompt( tr( "Are you sure you want to delete description of project <b>%1</b>?" ).arg( project.name() ) );
    setPromptPixmap( IconLoader::pixmap( "edit-delete", 22 ) );

    setContentLayout( NULL, true );
}

DeleteProjectDescriptionDialog::~DeleteProjectDescriptionDialog()
{
}

void DeleteProjectDescriptionDialog::accept()
{
    ProjectsBatch* batch = new ProjectsBatch();
    batch->deleteProjectDescription( m_projectId );

    executeBatch( batch );
}
