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

#include "projectdialogs.h"

#include "commands/projectsbatch.h"
#include "commands/commandmanager.h"
#include "data/datamanager.h"
#include "rdb/utilities.h"
#include "utils/errorhelper.h"
#include "utils/tablemodelshelper.h"
#include "utils/iconloader.h"
#include "widgets/inputlineedit.h"

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

    RDB::IndexConstIterator<ProjectRow> it( dataManager->projects()->index() );
    if ( RDB::findRow( it, &ProjectRow::name, name ) ) {
        showWarning( ErrorHelper::statusMessage( ErrorHelper::ProjectAlreadyExists ) );
        return;
    }

    ProjectsBatch* batch = new ProjectsBatch();
    batch->addProject( name );

    executeBatch( batch );
}

RenameProjectDialog::RenameProjectDialog( int projectId, QWidget* parent ) : CommandDialog( parent ),
    m_projectId( projectId )
{
    const ProjectRow* project = dataManager->projects()->find( projectId );
    m_oldName = project ? project->name() : QString();

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

    RDB::IndexConstIterator<ProjectRow> it( dataManager->projects()->index() );
    if ( RDB::findRow( it, &ProjectRow::name, name ) ) {
        showWarning( ErrorHelper::statusMessage( ErrorHelper::ProjectAlreadyExists ) );
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
    const ProjectRow* project = dataManager->projects()->find( projectId );
    QString name = project ? project->name() : QString();

    setWindowTitle( tr( "Delete Project" ) );
    setPrompt( tr( "Are you sure you want to delete project <b>%1</b>?" ).arg( name ) );
    setPromptPixmap( IconLoader::pixmap( "edit-delete", 22 ) );

    RDB::ForeignConstIterator<FolderRow> it( dataManager->folders()->parentIndex(), projectId );
    if ( it.next() ) {
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
    const ProjectRow* project = dataManager->projects()->find( projectId );
    QString name = project ? project->name() : QString();

    setWindowTitle( tr( "Add Folder" ) );
    setPrompt( tr( "Create a new folder in project <b>%1</b>:" ).arg( name ) );
    setPromptPixmap( IconLoader::pixmap( "folder-new", 22 ) );

    RDB::IndexConstIterator<TypeRow> it( dataManager->types()->index() );
    QList<const TypeRow*> types = localeAwareSortRows( it, &TypeRow::name );

    if ( types.empty() ) {
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
        m_typeCombo->addItem( types.at( i )->name(), types.at( i )->typeId() );

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

    RDB::ForeignConstIterator<FolderRow> it( dataManager->folders()->parentIndex(), m_projectId );
    if ( RDB::findRow( it, &FolderRow::name, name ) ) {
        showWarning( ErrorHelper::statusMessage( ErrorHelper::FolderAlreadyExists ) );
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
    const FolderRow* folder = dataManager->folders()->find( folderId );
    m_oldName = folder ? folder->name() : QString();

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

    const FolderRow* folder = dataManager->folders()->find( m_folderId );
    int projectId = folder ? folder->projectId() : 0;

    RDB::ForeignConstIterator<FolderRow> it( dataManager->folders()->parentIndex(), projectId );
    if ( RDB::findRow( it, &FolderRow::name, name ) ) {
        showWarning( ErrorHelper::statusMessage( ErrorHelper::FolderAlreadyExists ) );
        return;
    }

    ProjectsBatch* batch = new ProjectsBatch();
    batch->renameFolder( m_folderId, name );

    executeBatch( batch );
}

MoveFolderDialog::MoveFolderDialog( int folderId, QWidget* parent ) : CommandDialog( parent ),
    m_folderId( folderId )
{
    const FolderRow* folder = dataManager->folders()->find( folderId );
    QString name = folder ? folder->name() : QString();
    m_oldProjectId = folder ? folder->projectId() : 0;

    setWindowTitle( tr( "Move Folder" ) );
    setPrompt( tr( "Move folder <b>%1</b> to another project:" ).arg( name ) );
    setPromptPixmap( IconLoader::pixmap( "folder-move", 22 ) );

    QGridLayout* layout = new QGridLayout();

    QLabel* projectLabel = new QLabel( tr( "&Project:" ), this );
    layout->addWidget( projectLabel, 0, 0 );

    m_projectCombo = new QComboBox( this );
    layout->addWidget( m_projectCombo, 0, 1 );

    projectLabel->setBuddy( m_projectCombo );

    layout->setColumnStretch( 1, 1 );

    RDB::IndexConstIterator<ProjectRow> it( dataManager->projects()->index() );
    QList<const ProjectRow*> projects = localeAwareSortRows( it, &ProjectRow::name );

    for ( int i = 0; i < projects.count(); i++ ) {
        const ProjectRow* project = projects.at( i );
        if ( !TableModelsHelper::isProjectAdmin( project->projectId() ) )
            continue;
        m_projectCombo->addItem( project->name(), project->projectId() );
        if ( project->projectId() == m_oldProjectId )
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

    const FolderRow* folder = dataManager->folders()->find( m_folderId );
    QString name = folder ? folder->name() : QString();

    RDB::ForeignConstIterator<FolderRow> it( dataManager->folders()->parentIndex(), projectId );
    if ( RDB::findRow( it, &FolderRow::name, name ) ) {
        showWarning( ErrorHelper::statusMessage( ErrorHelper::FolderAlreadyExists ) );
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
    const FolderRow* folder = dataManager->folders()->find( folderId );
    QString name = folder ? folder->name() : QString();

    setWindowTitle( tr( "Delete Folder" ) );
    setPrompt( tr( "Are you sure you want to delete folder <b>%1</b>?" ).arg( name ) );
    setPromptPixmap( IconLoader::pixmap( "edit-delete", 22 ) );

    RDB::ForeignConstIterator<IssueRow> it( dataManager->issues()->parentIndex(), folderId );
    if ( it.next() ) {
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
