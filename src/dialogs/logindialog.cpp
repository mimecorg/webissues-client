/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2014 WebIssues Team
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

#include "logindialog.h"

#include "commands/commandmanager.h"
#include "data/datamanager.h"
#include "utils/errorhelper.h"
#include "utils/iconloader.h"
#include "widgets/inputlineedit.h"

#include <QLayout>
#include <QLabel>
#include <QCheckBox>

LoginDialog::LoginDialog( QWidget* parent ) : CommandDialog( parent )
{
    initialize( ServerMode, commandManager->serverUrl().host(), dataManager->serverName() );
}

LoginDialog::LoginDialog( DialogMode mode, const QString& hostName, const QString& serverName, QWidget* parent ) : CommandDialog( parent )
{
    initialize( mode, hostName, serverName );
}

void LoginDialog::initialize( DialogMode mode, const QString& hostName, const QString& serverName )
{
    setWindowTitle( tr( "Login" ) );

    switch ( mode ) {
        case ServerMode:
            setPrompt( tr( "Enter credentials for the WebIssues Server:" ) );
            break;
        case HttpMode:
            setPrompt( tr( "Enter credentials for the HTTP connection:" ) );
            break;
        case ProxyMode:
            setPrompt( tr( "Enter credentials for the proxy connection:" ) );
            break;
    }

    setPromptPixmap( IconLoader::pixmap( "edit-password", 22 ) );

    showInfo( tr( "Enter login and password." ) );

    QGridLayout* layout = new QGridLayout();

    QLabel* hostLabel = new QLabel( tr( "Host name:" ), this );
    layout->addWidget( hostLabel, 0, 0 );

    QLabel* hostValueLabel = new QLabel( hostName, this );
    layout->addWidget( hostValueLabel, 0, 1 );

    QLabel* nameLabel = new QLabel( tr( "Server name:" ), this );
    layout->addWidget( nameLabel, 1, 0 );

    QLabel* nameValueLabel = new QLabel( serverName, this );
    layout->addWidget( nameValueLabel, 1, 1 );

    QFrame* separator = new QFrame( this );
    separator->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    layout->addWidget( separator, 2, 0, 1, 2 );

    QLabel* loginLabel = new QLabel( tr( "&Login:" ), this );
    layout->addWidget( loginLabel, 3, 0 );

    m_loginEdit = new InputLineEdit( this );
    m_loginEdit->setMaxLength( 40 );
    m_loginEdit->setRequired( true );
    layout->addWidget( m_loginEdit, 3, 1 );

    loginLabel->setBuddy( m_loginEdit );

    QLabel* passwordLabel = new QLabel( tr( "&Password:" ), this );
    layout->addWidget( passwordLabel, 4, 0 );

    m_passwordEdit = new InputLineEdit( this );
    m_passwordEdit->setEchoMode( QLineEdit::Password );
    m_passwordEdit->setMaxLength( 40 );
    m_passwordEdit->setRequired( true );
    layout->addWidget( m_passwordEdit, 4, 1 );

    passwordLabel->setBuddy( m_passwordEdit );

    m_rememberCheck = new QCheckBox( tr( "&Remember password" ), this );
    layout->addWidget( m_rememberCheck, 5, 1 );

    setContentLayout( layout, true );

    m_loginEdit->setFocus();
}

LoginDialog::~LoginDialog()
{
}

void LoginDialog::showLoginError()
{
    showWarning( ErrorHelper::IncorrectLogin );
}

void LoginDialog::accept()
{
    if ( !validate() )
        return;

    CommandDialog::accept();
}

void LoginDialog::setLoginAndPassword( const QString& login, const QString& password )
{
    m_loginEdit->setInputValue( login );
    m_passwordEdit->setInputValue( password );

    m_rememberCheck->setChecked( !password.isEmpty() );

    if ( !login.isEmpty() && password.isEmpty() )
        m_passwordEdit->setFocus();
}

QString LoginDialog::login() const
{
    return m_loginEdit->inputValue();
}

QString LoginDialog::password() const
{
    return m_passwordEdit->inputValue();
}

bool LoginDialog::rememberPassword() const
{
    return m_rememberCheck->isChecked();
}

LoginNewDialog::LoginNewDialog( const QString& password, QWidget* parent ) : CommandDialog( parent ),
    m_oldPassword( password )
{
    setWindowTitle( tr( "Change Password" ) );
    setPrompt( tr( "You have to enter a new password in order to log in:" ) );
    setPromptPixmap( IconLoader::pixmap( "edit-password", 22 ) );

    showInfo( tr( "Enter your new password." ) );

    QGridLayout* layout = new QGridLayout();

    QLabel* passwordLabel = new QLabel( tr( "&Password:" ), this );
    layout->addWidget( passwordLabel, 0, 0 );

    m_passwordEdit = new InputLineEdit( this );
    m_passwordEdit->setEchoMode( QLineEdit::Password );
    m_passwordEdit->setMaxLength( 40 );
    m_passwordEdit->setRequired( true );
    layout->addWidget( m_passwordEdit, 0, 1 );

    passwordLabel->setBuddy( m_passwordEdit );

    QLabel* password2Label = new QLabel( tr( "Con&firm:" ), this );
    layout->addWidget( password2Label, 1, 0 );

    m_passwordEdit2 = new InputLineEdit( this );
    m_passwordEdit2->setEchoMode( QLineEdit::Password );
    m_passwordEdit2->setMaxLength( 40 );
    m_passwordEdit2->setRequired( true );
    layout->addWidget( m_passwordEdit2, 1, 1 );

    password2Label->setBuddy( m_passwordEdit2 );

    setContentLayout( layout, true );

    m_passwordEdit->setFocus();
}

LoginNewDialog::~LoginNewDialog()
{
}

void LoginNewDialog::accept()
{
    if ( !validate() )
        return;

    QString password = m_passwordEdit->inputValue();
    QString password2 = m_passwordEdit2->inputValue();

    if ( password != password2 ) {
        showWarning( ErrorHelper::PasswordNotMatching );
        return;
    }

    if ( password == m_oldPassword ) {
        showWarning( ErrorHelper::CannotReusePassword );
        return;
    }

    CommandDialog::accept();
}

QString LoginNewDialog::password() const
{
    return m_passwordEdit->inputValue();
}
