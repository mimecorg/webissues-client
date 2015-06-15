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

#include "ssldialogs.h"

#include "utils/datetimehelper.h"
#include "utils/iconloader.h"
#include "widgets/propertypanel.h"

#include <QLayout>
#include <QLabel>
#include <QGroupBox>
#include <QPushButton>
#include <QListWidget>
#include <QCheckBox>
#include <QDialogButtonBox>

SslCertificatesDialog::SslCertificatesDialog( QWidget* parent ) : InformationDialog( parent )
{
    setWindowTitle( tr( "Certificate Details" ) );
    setPromptPixmap( IconLoader::pixmap( "edit-password", 22 ) );
    setPrompt( tr( "Information about the certificate of the server:" ) );

    QVBoxLayout* layout = new QVBoxLayout();

    QGroupBox* listBox = new QGroupBox( tr( "Certification Path" ), this );
    QVBoxLayout* listLayout = new QVBoxLayout( listBox );

    m_list = new QListWidget( listBox );

    connect( m_list, SIGNAL( currentRowChanged( int ) ), this, SLOT( showCertificate( int ) ) );

    listLayout->addWidget( m_list );
    layout->addWidget( listBox );

    QGroupBox* subjectBox = new QGroupBox( tr( "Issued To" ), this );
    QVBoxLayout* subjectLayout = new QVBoxLayout( subjectBox );
    subjectLayout->setMargin( 0 );

    m_subjectPanel = new PropertyPanel( subjectBox );
    m_subjectPanel->addProperty( "CN", tr( "Common Name" ) );
    m_subjectPanel->addProperty( "O", tr( "Organization" ) );
    m_subjectPanel->addProperty( "OU", tr( "Organizational Unit" ) );

    subjectLayout->addWidget( m_subjectPanel );
    layout->addWidget( subjectBox );

    QGroupBox* issuerBox = new QGroupBox( tr( "Issued By" ), this );
    QVBoxLayout* issuerLayout = new QVBoxLayout( issuerBox );
    issuerLayout->setMargin( 0 );

    m_issuerPanel = new PropertyPanel( issuerBox );
    m_issuerPanel->addProperty( "CN", tr( "Common Name" ) );
    m_issuerPanel->addProperty( "O", tr( "Organization" ) );
    m_issuerPanel->addProperty( "OU", tr( "Organizational Unit" ) );

    issuerLayout->addWidget( m_issuerPanel );
    layout->addWidget( issuerBox );

    QGroupBox* detailsBox = new QGroupBox( tr( "Certificate Details" ), this );
    QVBoxLayout* detailsLayout = new QVBoxLayout( detailsBox );
    detailsLayout->setMargin( 0 );

    m_detailsPanel = new PropertyPanel( detailsBox );
    m_detailsPanel->addProperty( "effectiveDate", tr( "Issued On" ) );
    m_detailsPanel->addProperty( "expiryDate", tr( "Expires On" ) );
    m_detailsPanel->addProperty( "SHA1", tr( "SHA1 Fingerprint" ) );
    m_detailsPanel->addProperty( "MD5", tr( "MD5 Fingerprint" ) );

    detailsLayout->addWidget( m_detailsPanel );
    layout->addWidget( detailsBox );

    setContentLayout( layout, false );

    resize( 450, 500 );
}

SslCertificatesDialog::~SslCertificatesDialog()
{
}

void SslCertificatesDialog::setCertificates( const QList<QSslCertificate>& certificates )
{
    m_certificates = certificates;

    m_list->clear();

    for ( int i = 0; i < certificates.count(); i++ ) {
        QListWidgetItem* item = new QListWidgetItem( m_list );
        item->setText( certificates.at( i ).subjectInfo( QSslCertificate::CommonName ).first() );
    }

    m_list->setCurrentRow( 0 );
}

void SslCertificatesDialog::showCertificate( int index )
{
    if ( index >= 0 && index < m_certificates.count() ) {
        QSslCertificate certificate = m_certificates.at( index );

        m_subjectPanel->setValue( "CN", certificate.subjectInfo( QSslCertificate::CommonName ).join( "; " ) );
        m_subjectPanel->setValue( "O", certificate.subjectInfo( QSslCertificate::Organization ).join( "; " ) );
        m_subjectPanel->setValue( "OU", certificate.subjectInfo( QSslCertificate::OrganizationalUnitName ).join( "; " ) );

        m_issuerPanel->setValue( "CN", certificate.issuerInfo( QSslCertificate::CommonName ).join( "; " ) );
        m_issuerPanel->setValue( "O", certificate.issuerInfo( QSslCertificate::Organization ).join( "; " ) );
        m_issuerPanel->setValue( "OU", certificate.issuerInfo( QSslCertificate::OrganizationalUnitName ).join( "; " ) );

        m_detailsPanel->setValue( "effectiveDate", formatDate( certificate.effectiveDate() ) );
        m_detailsPanel->setValue( "expiryDate", formatDate( certificate.expiryDate() ) );
        m_detailsPanel->setValue( "SHA1", formatBinary( certificate.digest( QCryptographicHash::Sha1 ) ) );
        m_detailsPanel->setValue( "MD5", formatBinary( certificate.digest( QCryptographicHash::Md5 ) ) );
    } else {
        m_subjectPanel->clearValues();
        m_issuerPanel->clearValues();
        m_detailsPanel->clearValues();
    }
}

QString SslCertificatesDialog::formatDate( const QDateTime& dateTime )
{
    if ( !dateTime.isValid() )
        return QString();
    return DateTimeHelper::formatDate( dateTime.date() );
}

QString SslCertificatesDialog::formatBinary( const QByteArray& binary )
{
    QString result;
    for ( int i = 0; i < binary.length(); i++ ) {
        if ( i > 0 )
            result += QLatin1Char( ':' );
        result += QString( "%1" ).arg( (int)(unsigned char)binary.at( i ), 2, 16, QLatin1Char( '0' ) );
    }
    return result;
}

SslErrorsDialog::SslErrorsDialog( QWidget* parent ) : CommandDialog( parent )
{
    setWindowTitle( tr( "Secure Connection Error" ) );
    setPromptPixmap( IconLoader::pixmap( "edit-access", 22 ) );
    setPrompt( tr( "The authenticity of the host you are connecting to cannot be validated:" ) );

    QVBoxLayout* layout = new QVBoxLayout();

    m_list = new QListWidget( this );
    m_list->setSelectionMode( QListWidget::NoSelection );
    m_list->setFixedHeight( 100 );
    layout->addWidget( m_list );

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    layout->addLayout( buttonLayout );

    m_acceptCheckBox = new QCheckBox( tr( "&Accept this certificate permanently" ), this );
    m_acceptCheckBox->setEnabled( false );
    buttonLayout->addWidget( m_acceptCheckBox );
    buttonLayout->addStretch( 1 );

    m_certificatesButton = new QPushButton( tr( "&View Certificate..." ), this );
    m_certificatesButton->setIcon( IconLoader::icon( "edit-password" ) );
    m_certificatesButton->setIconSize( QSize( 16, 16 ) );
    m_certificatesButton->setEnabled( false );
    buttonLayout->addWidget( m_certificatesButton );

    connect( m_certificatesButton, SIGNAL( clicked() ), this, SLOT( showCertificates() ) );

    buttonBox()->button( QDialogButtonBox::Ok )->setText( tr( "&Ignore" ) );

    setContentLayout( layout, true );

    showWarning( tr( "Review the certificate errors." ) );
}

SslErrorsDialog::~SslErrorsDialog()
{
}

void SslErrorsDialog::setErrors( const QList<QSslError>& errors )
{
    m_list->clear();

    for ( int i = 0; i < errors.count(); i++ ) {
        if ( errors.at( i ).error() != QSslError::NoError ) {
            QListWidgetItem* item = new QListWidgetItem( m_list );
            item->setText( errors.at( i ).errorString() );
        }
    }
}

void SslErrorsDialog::setCertificates( const QList<QSslCertificate>& certificates )
{
    m_certificates = certificates;

    m_certificatesButton->setEnabled( !certificates.isEmpty() );
    m_acceptCheckBox->setEnabled( !certificates.isEmpty() );
}

bool SslErrorsDialog::acceptPermanently() const
{
    return m_acceptCheckBox->isChecked();
}

void SslErrorsDialog::showCertificates()
{
    SslCertificatesDialog dialog( this );
    dialog.setCertificates( m_certificates );
    dialog.exec();
}
