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

#include "userdialogs.h"

#include "data/datamanager.h"
#include "data/entities.h"
#include "commands/usersbatch.h"
#include "utils/errorhelper.h"
#include "utils/iconloader.h"
#include "widgets/inputlineedit.h"

#include <QLayout>
#include <QLabel>
#include <QButtonGroup>
#include <QRadioButton>
#include <QListWidget>
#include <QCheckBox>

AddUserDialog::AddUserDialog( QWidget* parent ) : CommandDialog( parent )
{
    setWindowTitle( tr( "Add User" ) );
    setPrompt( tr( "Create a new user:" ) );
    setPromptPixmap( IconLoader::pixmap( "user-new", 22 ) );

    QGridLayout* layout = new QGridLayout();

    QLabel* loginLabel = new QLabel( tr( "&Login:" ), this );
    layout->addWidget( loginLabel, 0, 0 );

    m_loginEdit = new InputLineEdit( this );
    m_loginEdit->setMaxLength( 40 );
    m_loginEdit->setRequired( true );
    layout->addWidget( m_loginEdit, 0, 1 );

    loginLabel->setBuddy( m_loginEdit );

    QLabel* nameLabel = new QLabel( tr( "&Name:" ), this );
    layout->addWidget( nameLabel, 1, 0 );

    m_nameEdit = new InputLineEdit( this );
    m_nameEdit->setMaxLength( 40 );
    m_nameEdit->setRequired( true );
    layout->addWidget( m_nameEdit, 1, 1 );

    nameLabel->setBuddy( m_nameEdit );

    QLabel* passwordLabel = new QLabel( tr( "&Password:" ), this );
    layout->addWidget( passwordLabel, 2, 0 );

    m_passwordEdit = new InputLineEdit( this );
    m_passwordEdit->setEchoMode( QLineEdit::Password );
    m_passwordEdit->setMaxLength( 40 );
    m_passwordEdit->setRequired( true );
    layout->addWidget( m_passwordEdit, 2, 1 );

    passwordLabel->setBuddy( m_passwordEdit );

    QLabel* password2Label = new QLabel( tr( "Con&firm:" ), this );
    layout->addWidget( password2Label, 3, 0 );

    m_passwordEdit2 = new InputLineEdit( this );
    m_passwordEdit2->setEchoMode( QLineEdit::Password );
    m_passwordEdit2->setMaxLength( 40 );
    m_passwordEdit2->setRequired( true );
    layout->addWidget( m_passwordEdit2, 3, 1 );

    password2Label->setBuddy( m_passwordEdit2 );

    m_tempCheck = new QCheckBox( tr( "&User must change password at next logon" ), this );
    layout->addWidget( m_tempCheck, 4, 1 );

    setContentLayout( layout, true );

    m_loginEdit->setFocus();
}

AddUserDialog::~AddUserDialog()
{
}

void AddUserDialog::accept()
{
    if ( !validate() )
        return;

    QString login = m_loginEdit->inputValue();
    QString name = m_nameEdit->inputValue();

    if ( UserEntity::exists( login, name ) ) {
        showWarning( ErrorHelper::UserAlreadyExists );
        return;
    }

    QString password = m_passwordEdit->inputValue();
    QString password2 = m_passwordEdit2->inputValue();

    if ( password != password2 ) {
        showWarning( ErrorHelper::PasswordNotMatching );
        return;
    }

    UsersBatch* batch = new UsersBatch();
    batch->addUser( login, name, password, m_tempCheck->isChecked() );

    executeBatch( batch );
}

SetPasswordDialog::SetPasswordDialog( int userId, QWidget* parent ) : CommandDialog( parent ),
    m_userId( userId ),
    m_passwordEdit( NULL ),
    m_tempCheck( NULL )
{
    UserEntity user = UserEntity::find( userId );

    setWindowTitle( tr( "Change Password" ) );
    if ( userId == dataManager->currentUserId() )
        setPrompt( tr( "Enter your new password:" ) );
    else
        setPrompt( tr( "Enter the new password for user <b>%1</b>:" ).arg( user.name() ) );
    setPromptPixmap( IconLoader::pixmap( "edit-password", 22 ) );

    QGridLayout* layout = new QGridLayout();

    int row = 0;

    if ( userId == dataManager->currentUserId() ) {
        QLabel* passwordLabel = new QLabel( tr( "Current &Password:" ), this );
        layout->addWidget( passwordLabel, row, 0 );

        m_passwordEdit = new InputLineEdit( this );
        m_passwordEdit->setEchoMode( QLineEdit::Password );
        m_passwordEdit->setMaxLength( 40 );
        m_passwordEdit->setRequired( true );
        layout->addWidget( m_passwordEdit, row++, 1 );

        passwordLabel->setBuddy( m_passwordEdit );
    }

    QLabel* newPasswordLabel = new QLabel( tr( "&New Password:" ), this );
    layout->addWidget( newPasswordLabel, row, 0 );

    m_newPasswordEdit = new InputLineEdit( this );
    m_newPasswordEdit->setEchoMode( QLineEdit::Password );
    m_newPasswordEdit->setMaxLength( 40 );
    m_newPasswordEdit->setRequired( true );
    layout->addWidget( m_newPasswordEdit, row++, 1 );

    newPasswordLabel->setBuddy( m_newPasswordEdit );

    QLabel* newPassword2Label = new QLabel( tr( "Con&firm:" ), this );
    layout->addWidget( newPassword2Label, row, 0 );

    m_newPasswordEdit2 = new InputLineEdit( this );
    m_newPasswordEdit2->setEchoMode( QLineEdit::Password );
    m_newPasswordEdit2->setMaxLength( 40 );
    m_newPasswordEdit2->setRequired( true );
    layout->addWidget( m_newPasswordEdit2, row++, 1 );

    newPassword2Label->setBuddy( m_newPasswordEdit2 );

    if ( userId != dataManager->currentUserId() ) {
        m_tempCheck = new QCheckBox( tr( "&User must change password at next logon" ), this );
        layout->addWidget( m_tempCheck, row++, 1 );
    }

    setContentLayout( layout, true );

    if ( m_passwordEdit )
        m_passwordEdit->setFocus();
    else
        m_newPasswordEdit->setFocus();
}

SetPasswordDialog::~SetPasswordDialog()
{
}

void SetPasswordDialog::accept()
{
    if ( !validate() )
        return;

    QString newPassword = m_newPasswordEdit->inputValue();
    QString newPassword2 = m_newPasswordEdit2->inputValue();

    if ( newPassword != newPassword2 ) {
        showWarning( ErrorHelper::PasswordNotMatching );
        return;
    }

    if ( m_userId == dataManager->currentUserId() ) {
        QString password = m_passwordEdit->inputValue();
    
        if ( password == newPassword ) {
            showWarning( ErrorHelper::CannotReusePassword );
            return;
        }

        UsersBatch* batch = new UsersBatch();
        batch->changePassword( password, newPassword );

        executeBatch( batch );
    } else {
        UsersBatch* batch = new UsersBatch();
        batch->setPassword( m_userId, newPassword, m_tempCheck->isChecked() );

        executeBatch( batch );
    }
}

ChangeUserAccessDialog::ChangeUserAccessDialog( int userId, QWidget* parent ) : CommandDialog( parent ),
    m_userId( userId )
{
    UserEntity user = UserEntity::find( userId );
    m_oldAccess = user.access();

    setWindowTitle( tr( "Global Access" ) );
    setPrompt( tr( "Change global access for user <b>%1</b>:" ).arg( user.name() ) );
    setPromptPixmap( IconLoader::pixmap( "edit-access", 22 ) );

    QHBoxLayout* layout = new QHBoxLayout();

    QLabel* label = new QLabel( tr( "Access:" ), this );
    layout->addWidget( label );

    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    layout->addLayout( buttonsLayout );

    m_accessGroup = new QButtonGroup( this );

    QRadioButton* noneButton = new QRadioButton( tr( "&Disabled" ), this );
    m_accessGroup->addButton( noneButton, NoAccess );
    buttonsLayout->addWidget( noneButton );

    QRadioButton* normalButton = new QRadioButton( tr( "&Regular user" ), this );
    m_accessGroup->addButton( normalButton, NormalAccess );
    buttonsLayout->addWidget( normalButton );

    QRadioButton* adminButton = new QRadioButton( tr( "&System administrator" ), this );
    m_accessGroup->addButton( adminButton, AdminAccess );
    buttonsLayout->addWidget( adminButton );

    buttonsLayout->addStretch( 1 );

    m_accessGroup->button( m_oldAccess )->setChecked( true );

    setContentLayout( layout, true );

    m_accessGroup->checkedButton()->setFocus();
}

ChangeUserAccessDialog::~ChangeUserAccessDialog()
{
}

void ChangeUserAccessDialog::accept()
{
    int access = m_accessGroup->checkedId();

    if ( access == m_oldAccess ) {
        QDialog::accept();
        return;
    }

    UsersBatch* batch = new UsersBatch();
    batch->grantUser( m_userId, (Access)access );

    executeBatch( batch );
}

RenameUserDialog::RenameUserDialog( int userId, QWidget* parent ) : CommandDialog( parent ),
    m_userId( userId )
{
    UserEntity user = UserEntity::find( userId );
    m_oldName = user.name();

    setWindowTitle( tr( "Rename User" ) );
    setPrompt( tr( "Enter the new name of user <b>%1</b>:" ).arg( m_oldName ) );
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

RenameUserDialog::~RenameUserDialog()
{
}

void RenameUserDialog::accept()
{
    if ( !validate() )
        return;

    QString name = m_nameEdit->inputValue();

    if ( name == m_oldName ) {
        QDialog::accept();
        return;
    }

    if ( UserEntity::exists( QString(), name ) ) {
        showWarning( ErrorHelper::UserAlreadyExists );
        return;
    }

    UsersBatch* batch = new UsersBatch();
    batch->renameUser( m_userId, name );

    executeBatch( batch );
}

AddMemberDialog::AddMemberDialog( int projectId, QWidget* parent ) : CommandDialog( parent ),
    m_projectId( projectId ),
    m_list( NULL ),
    m_accessGroup( NULL )
{
    ProjectEntity project = ProjectEntity::find( projectId );

    setWindowTitle( tr( "Add Members" ) );
    setPrompt( tr( "Add new members to project <b>%1</b>:" ).arg( project.name() ) );
    setPromptPixmap( IconLoader::pixmap( "user-new", 22 ) );

    QList<int> members;
    foreach ( const UserEntity& member, project.members() )
        members.append( member.id() );

    QList<UserEntity> available;

    foreach ( const UserEntity& user, UserEntity::list() ) {
        if ( members.contains( user.id() ) )
            continue;
        available.append( user );
    }

    if ( available.empty() ) {
        showWarning( tr( "There are no more available users to add." ) );
        showCloseButton();
        setContentLayout( NULL, true );
        return;
    }

    QGridLayout* layout = new QGridLayout();

    QHBoxLayout* selectLayout = new QHBoxLayout();
    layout->addLayout( selectLayout, 0, 1 );

    selectLayout->addStretch( 1 );

    QLabel* allUsersLabel = new QLabel( "<a href=\"#\">" + tr( "Select All" ) + "</a>", this );
    selectLayout->addWidget( allUsersLabel );

    QLabel* noUsersLabel = new QLabel( "<a href=\"#\">" + tr( "Unselect All" ) + "</a>", this );
    selectLayout->addWidget( noUsersLabel );

    connect( allUsersLabel, SIGNAL( linkActivated( const QString& ) ), this, SLOT( allUsersActivated() ) );
    connect( noUsersLabel, SIGNAL( linkActivated( const QString& ) ), this, SLOT( noUsersActivated() ) );

    QLabel* userLabel = new QLabel( tr( "&Users:" ), this );
    layout->addWidget( userLabel, 1, 0 );

    m_list = new QListWidget( this );
    layout->addWidget( m_list, 1, 1 );

    userLabel->setBuddy( m_list );

    foreach ( const UserEntity& user, available ) {
        QListWidgetItem* item = new QListWidgetItem( m_list );
        item->setText( user.name() );
        item->setData( Qt::UserRole, user.id() );
        item->setCheckState( Qt::Unchecked );
    }

    QLabel* accessLabel = new QLabel( tr( "Access:" ), this );
    layout->addWidget( accessLabel, 2, 0 );

    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    layout->addLayout( buttonsLayout, 2, 1 );

    m_accessGroup = new QButtonGroup( this );

    QRadioButton* normalButton = new QRadioButton( tr( "&Regular member" ), this );
    m_accessGroup->addButton( normalButton, NormalAccess );
    buttonsLayout->addWidget( normalButton );

    QRadioButton* adminButton = new QRadioButton( tr( "&Project administrator" ), this );
    m_accessGroup->addButton( adminButton, AdminAccess );
    buttonsLayout->addWidget( adminButton );

    buttonsLayout->addStretch( 1 );

    m_accessGroup->button( NormalAccess )->setChecked( true );

    setContentLayout( layout, false );

    m_list->setFocus();
}

AddMemberDialog::~AddMemberDialog()
{
}

void AddMemberDialog::accept()
{
    QList<int> users;

    for ( int i = 0; i < m_list->count(); i++ ) {
        if ( m_list->item( i )->checkState() == Qt::Checked ) {
            int userId = m_list->item( i )->data( Qt::UserRole ).toInt();
            users.append( userId );
        }
    }

    if ( users.isEmpty() ) {
        showWarning( tr( "No user selected." ) );
        return;
    }

    int access = m_accessGroup->checkedId();

    UsersBatch* batch = new UsersBatch();
    for ( int i = 0; i < users.count(); i++ )
        batch->grantMember( users.at( i ), m_projectId, (Access)access );

    executeBatch( batch );
}

void AddMemberDialog::allUsersActivated()
{
    for ( int i = 0; i < m_list->count(); i++ )
        m_list->item( i )->setCheckState( Qt::Checked );
}

void AddMemberDialog::noUsersActivated()
{
    for ( int i = 0; i < m_list->count(); i++ )
        m_list->item( i )->setCheckState( Qt::Unchecked );
}

ChangeMemberAccessDialog::ChangeMemberAccessDialog( const QList<int>& users, int projectId, QWidget* parent ) :
    CommandDialog( parent ),
    m_users( users ),
    m_projectId( projectId )
{
    ProjectEntity project = ProjectEntity::find( projectId );
    MemberEntity member = MemberEntity::find( projectId, users.first() );

    setWindowTitle( tr( "Change Access" ) );
    if ( users.count() == 1 )
        setPrompt( tr( "Set new access level to project <b>%1</b> for user <b>%2</b>:" ).arg( project.name(), member.name() ) );
    else
        setPrompt( tr( "Set new access level to project <b>%1</b> for %2 selected users:" ).arg( project.name() ).arg( users.count() ) );
    setPromptPixmap( IconLoader::pixmap( "edit-access", 22 ) );

    QHBoxLayout* layout = new QHBoxLayout();

    QLabel* label = new QLabel( tr( "Access:" ), this );
    layout->addWidget( label );

    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    layout->addLayout( buttonsLayout );

    m_accessGroup = new QButtonGroup( this );

    QRadioButton* normalButton = new QRadioButton( tr( "&Regular member" ), this );
    m_accessGroup->addButton( normalButton, NormalAccess );
    buttonsLayout->addWidget( normalButton );

    QRadioButton* adminButton = new QRadioButton( tr( "&Project administrator" ), this );
    m_accessGroup->addButton( adminButton, AdminAccess );
    buttonsLayout->addWidget( adminButton );

    buttonsLayout->addStretch( 1 );

    m_accessGroup->button( member.access() )->setChecked( true );

    setContentLayout( layout, true );

    m_accessGroup->checkedButton()->setFocus();
}

ChangeMemberAccessDialog::~ChangeMemberAccessDialog()
{
}

void ChangeMemberAccessDialog::accept()
{
    int access = m_accessGroup->checkedId();

    QList<int> changed;

    for ( int i = 0; i < m_users.count(); i++ ) {    
        int userId = m_users.at( i );
        MemberEntity member = MemberEntity::find( m_projectId, userId );
        if ( member.access() != access )
            changed.append( userId );
    }

    if ( changed.isEmpty() ) {
        QDialog::accept();
        return;
    }

    UsersBatch* batch = new UsersBatch();
    for ( int i = 0; i < changed.count(); i++ ) {
        int userId = changed.at( i );
        batch->grantMember( userId, m_projectId, (Access)access );
    }

    executeBatch( batch );
}

RemoveMemberDialog::RemoveMemberDialog( const QList<int>& users, int projectId, QWidget* parent ) : CommandDialog( parent ),
    m_users( users ),
    m_projectId( projectId )
{
    ProjectEntity project = ProjectEntity::find( projectId );
    MemberEntity member = MemberEntity::find( projectId, users.first() );

    if ( users.count() == 1 ) {
        setWindowTitle( tr( "Remove Member" ) );
        setPrompt( tr( "Do you want to remove user <b>%1</b> from project <b>%2</b>?" ).arg( member.name(), project.name() ) );
    } else {
        setWindowTitle( tr( "Remove Members" ) );
        setPrompt( tr( "Do you want to remove %1 selected users from project <b>%2</b>?" ).arg( users.count() ).arg( project.name() ) );
    }
    setPromptPixmap( IconLoader::pixmap( "edit-delete", 22 ) );

    setContentLayout( NULL, true );
}

RemoveMemberDialog::~RemoveMemberDialog()
{
}

void RemoveMemberDialog::accept()
{
    UsersBatch* batch = new UsersBatch();

    for ( int i = 0; i < m_users.count(); i++ ) {
        int userId = m_users.at( i );
        batch->grantMember( userId, m_projectId, NoAccess );
    }

    executeBatch( batch );
}

AddUserProjectsDialog::AddUserProjectsDialog( int userId, QWidget* parent ) : CommandDialog( parent ),
    m_userId( userId ),
    m_list( NULL ),
    m_accessGroup( NULL )
{
    UserEntity user = UserEntity::find( userId );

    setWindowTitle( tr( "Add Projects" ) );
    setPrompt( tr( "Add user <b>%1</b> to the selected projects:" ).arg( user.name() ) );
    setPromptPixmap( IconLoader::pixmap( "project-new", 22 ) );

    QList<int> projects;
    foreach ( const ProjectEntity& project, user.projects() )
        projects.append( project.id() );

    QList<ProjectEntity> available;

    foreach ( const ProjectEntity& project, ProjectEntity::list() ) {
        if ( projects.contains( project.id() ) )
            continue;
        available.append( project );
    }

    if ( available.empty() ) {
        showWarning( tr( "There are no more available projects to add." ) );
        showCloseButton();
        setContentLayout( NULL, true );
        return;
    }

    QGridLayout* layout = new QGridLayout();

    QHBoxLayout* selectLayout = new QHBoxLayout();
    layout->addLayout( selectLayout, 0, 1 );

    selectLayout->addStretch( 1 );

    QLabel* allProjectsLabel = new QLabel( "<a href=\"#\">" + tr( "Select All" ) + "</a>", this );
    selectLayout->addWidget( allProjectsLabel );

    QLabel* noProjectsLabel = new QLabel( "<a href=\"#\">" + tr( "Unselect All" ) + "</a>", this );
    selectLayout->addWidget( noProjectsLabel );

    connect( allProjectsLabel, SIGNAL( linkActivated( const QString& ) ), this, SLOT( allProjectsActivated() ) );
    connect( noProjectsLabel, SIGNAL( linkActivated( const QString& ) ), this, SLOT( noProjectsActivated() ) );

    QLabel* projectsLabel = new QLabel( tr( "&Projects:" ), this );
    layout->addWidget( projectsLabel, 1, 0 );

    m_list = new QListWidget( this );
    layout->addWidget( m_list, 1, 1 );

    projectsLabel->setBuddy( m_list );

    foreach ( const ProjectEntity& project, available ) {
        QListWidgetItem* item = new QListWidgetItem( m_list );
        item->setText( project.name() );
        item->setData( Qt::UserRole, project.id() );
        item->setCheckState( Qt::Unchecked );
    }

    QLabel* accessLabel = new QLabel( tr( "Access:" ), this );
    layout->addWidget( accessLabel, 2, 0 );

    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    layout->addLayout( buttonsLayout, 2, 1 );

    m_accessGroup = new QButtonGroup( this );

    QRadioButton* normalButton = new QRadioButton( tr( "&Regular member" ), this );
    m_accessGroup->addButton( normalButton, NormalAccess );
    buttonsLayout->addWidget( normalButton );

    QRadioButton* adminButton = new QRadioButton( tr( "&Project administrator" ), this );
    m_accessGroup->addButton( adminButton, AdminAccess );
    buttonsLayout->addWidget( adminButton );

    buttonsLayout->addStretch( 1 );

    m_accessGroup->button( NormalAccess )->setChecked( true );

    setContentLayout( layout, false );

    m_list->setFocus();
}

AddUserProjectsDialog::~AddUserProjectsDialog()
{
}

void AddUserProjectsDialog::accept()
{
    QList<int> projects;

    for ( int i = 0; i < m_list->count(); i++ ) {
        if ( m_list->item( i )->checkState() == Qt::Checked ) {
            int projectId = m_list->item( i )->data( Qt::UserRole ).toInt();
            projects.append( projectId );
        }
    }

    if ( projects.isEmpty() ) {
        showWarning( tr( "No project selected." ) );
        return;
    }

    int access = m_accessGroup->checkedId();

    UsersBatch* batch = new UsersBatch();
    for ( int i = 0; i < projects.count(); i++ )
        batch->grantMember( m_userId, projects.at( i ), (Access)access );

    executeBatch( batch );
}

void AddUserProjectsDialog::allProjectsActivated()
{
    for ( int i = 0; i < m_list->count(); i++ )
        m_list->item( i )->setCheckState( Qt::Checked );
}

void AddUserProjectsDialog::noProjectsActivated()
{
    for ( int i = 0; i < m_list->count(); i++ )
        m_list->item( i )->setCheckState( Qt::Unchecked );
}

ChangeUserProjectsAccessDialog::ChangeUserProjectsAccessDialog( int userId, const QList<int>& projects, QWidget* parent ) :
    CommandDialog( parent ),
    m_userId( userId ),
    m_projects( projects )
{
    UserEntity user = UserEntity::find( userId );
    ProjectEntity project = ProjectEntity::find( projects.first() );
    MemberEntity member = MemberEntity::find( projects.first(), userId );

    setWindowTitle( tr( "Project Access" ) );
    if ( projects.count() == 1 )
        setPrompt( tr( "Change access to project <b>%1</b> for user <b>%2</b>:" ).arg( project.name(), user.name() ) );
    else
        setPrompt( tr( "Change access to %1 selected projects for user <b>%2</b>:" ).arg( projects.count() ).arg( user.name() ) );
    setPromptPixmap( IconLoader::pixmap( "edit-access", 22 ) );

    QHBoxLayout* layout = new QHBoxLayout();

    QLabel* label = new QLabel( tr( "Access:" ), this );
    layout->addWidget( label );

    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    layout->addLayout( buttonsLayout );

    m_accessGroup = new QButtonGroup( this );

    QRadioButton* normalButton = new QRadioButton( tr( "&Regular member" ), this );
    m_accessGroup->addButton( normalButton, NormalAccess );
    buttonsLayout->addWidget( normalButton );

    QRadioButton* adminButton = new QRadioButton( tr( "&Project administrator" ), this );
    m_accessGroup->addButton( adminButton, AdminAccess );
    buttonsLayout->addWidget( adminButton );

    buttonsLayout->addStretch( 1 );

    m_accessGroup->button( member.access() )->setChecked( true );

    setContentLayout( layout, true );

    m_accessGroup->checkedButton()->setFocus();
}

ChangeUserProjectsAccessDialog::~ChangeUserProjectsAccessDialog()
{
}

void ChangeUserProjectsAccessDialog::accept()
{
    int access = m_accessGroup->checkedId();

    QList<int> changed;

    for ( int i = 0; i < m_projects.count(); i++ ) {    
        int projectId = m_projects.at( i );
        MemberEntity member = MemberEntity::find( projectId, m_userId );
        if ( member.access() != access )
            changed.append( projectId );
    }

    if ( changed.isEmpty() ) {
        QDialog::accept();
        return;
    }

    UsersBatch* batch = new UsersBatch();
    for ( int i = 0; i < changed.count(); i++ ) {
        int projectId = changed.at( i );
        batch->grantMember( m_userId, projectId, (Access)access );
    }

    executeBatch( batch );
}

RemoveUserProjectsDialog::RemoveUserProjectsDialog( int userId, const QList<int>& projects, QWidget* parent ) : CommandDialog( parent ),
    m_userId( userId ),
    m_projects( projects )
{
    UserEntity user = UserEntity::find( userId );
    ProjectEntity project = ProjectEntity::find( projects.first() );

    if ( projects.count() == 1 ) {
        setWindowTitle( tr( "Remove Project" ) );
        setPrompt( tr( "Do you want to remove user <b>%1</b> from project <b>%2</b>?" ).arg( user.name(), project.name() ) );
    } else {
        setWindowTitle( tr( "Remove Projects" ) );
        setPrompt( tr( "Do you want to remove user <b>%1</b> from %2 projects?" ).arg( user.name() ).arg( projects.count() ) );
    }
    setPromptPixmap( IconLoader::pixmap( "edit-delete", 22 ) );

    setContentLayout( NULL, true );
}

RemoveUserProjectsDialog::~RemoveUserProjectsDialog()
{
}

void RemoveUserProjectsDialog::accept()
{
    UsersBatch* batch = new UsersBatch();

    for ( int i = 0; i < m_projects.count(); i++ ) {
        int projectId = m_projects.at( i );
        batch->grantMember( m_userId, projectId, NoAccess );
    }

    executeBatch( batch );
}
