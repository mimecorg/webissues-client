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

#include "connectioninfodialog.h"

#if defined( HAVE_OPENSSL )
#include "ssldialogs.h"
#endif

#include "commands/commandmanager.h"
#include "data/datamanager.h"
#include "utils/iconloader.h"
#include "widgets/propertypanel.h"

#include <QLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QDialogButtonBox>

#if defined( HAVE_OPENSSL )
#include <QSslCipher>
#endif

ConnectionInfoDialog::ConnectionInfoDialog( QWidget* parent ) : InformationDialog( parent )
{
    setWindowTitle( tr( "Connection Details" ) );
    setPromptPixmap( IconLoader::pixmap( "status-info", 22 ) );
    setPrompt( tr( "Information about current connection:" ) );

    QVBoxLayout* layout = new QVBoxLayout();

    QGroupBox* serverBox = new QGroupBox( tr( "Server Information" ), this );
    QVBoxLayout* serverLayout = new QVBoxLayout( serverBox );
    layout->addWidget( serverBox );

    m_serverPanel = new PropertyPanel( serverBox );
    m_serverPanel->setInnerMargin( 0 );

    m_serverPanel->addProperty( "url", tr( "URL:" ) );
    m_serverPanel->addProperty( "name", tr( "Name:" ) );
    m_serverPanel->addProperty( "version", tr( "Version:" ) );
    m_serverPanel->addProperty( "encryption", tr( "Encryption:" ) );

    serverLayout->addWidget( m_serverPanel );

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    m_certificatesButton = new QPushButton( tr( "&View Certificate..." ), serverBox );
    m_certificatesButton->setIcon( IconLoader::icon( "edit-password" ) );
    m_certificatesButton->setIconSize( QSize( 16, 16 ) );
    buttonLayout->addWidget( m_certificatesButton );

    serverLayout->addLayout( buttonLayout );

    connect( m_certificatesButton, SIGNAL( clicked() ), this, SLOT( viewCertificates() ) );

    QGroupBox* userBox = new QGroupBox( tr( "User Information" ), this );
    QVBoxLayout* userLayout = new QVBoxLayout( userBox );
    layout->addWidget( userBox );

    m_userPanel = new PropertyPanel( serverBox );
    m_userPanel->setInnerMargin( 0 );

    m_userPanel->addProperty( "login", tr( "Login:" ) );
    m_userPanel->addProperty( "name", tr( "Name:" ) );
    m_userPanel->addProperty( "access", tr( "Access:" ) );

    userLayout->addWidget( m_userPanel );

    updateInformation();

    setContentLayout( layout, true );
}

ConnectionInfoDialog::~ConnectionInfoDialog()
{
}

void ConnectionInfoDialog::updateInformation()
{
    m_serverPanel->setValue( "url", commandManager->serverUrl().toString() );

#if defined( HAVE_OPENSSL )
    m_certificatesButton->setEnabled( !commandManager->sslConfiguration().peerCertificateChain().isEmpty() );

    QSslCipher cipher = commandManager->sslConfiguration().sessionCipher();
    if ( !cipher.isNull() )
        m_serverPanel->setValue( "encryption", cipher.name() );
    else
        m_serverPanel->setValue( "encryption", tr( "None" ) );
#else
    m_certificatesButton->setEnabled( false );
    m_serverPanel->setValue( "encrypted", tr( "No" ) );
#endif

    m_serverPanel->setValue( "name", dataManager->serverName() );
    m_serverPanel->setValue( "version", dataManager->serverVersion() );

    m_userPanel->setValue( "login", dataManager->currentUserLogin() );
    m_userPanel->setValue( "name", dataManager->currentUserName() );

    Access access = dataManager->currentUserAccess();
    m_userPanel->setValue( "access", ( access == AdminAccess ) ? tr( "Administrator" ) : tr( "Regular" ) );
}

void ConnectionInfoDialog::viewCertificates()
{
#if defined( HAVE_OPENSSL )
    SslCertificatesDialog dialog( this );
    dialog.setCertificates( commandManager->sslConfiguration().peerCertificateChain() );
    dialog.exec();
#endif
}
