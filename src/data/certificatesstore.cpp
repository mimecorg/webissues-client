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

#include "certificatesstore.h"

#include <QFile>

CertificatesStore::CertificatesStore( const QString& path, QObject* parent ) : QObject( parent ),
    m_path( path )
{
    load();
}

CertificatesStore::~CertificatesStore()
{
    save();
}

bool CertificatesStore::containsCertificate( const QSslCertificate& certificate )
{
    return m_certificates.contains( certificate );
}

void CertificatesStore::addCertificate( const QSslCertificate& certificate )
{
    m_certificates.prepend( certificate );
}

void CertificatesStore::load()
{
    QFile file( m_path );

    if ( !file.open( QIODevice::ReadOnly ) )
        return;

    m_certificates = QSslCertificate::fromDevice( &file, QSsl::Pem );
}

void CertificatesStore::save()
{
    if ( m_certificates.isEmpty() )
        return;

    QFile file( m_path );

    if ( !file.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
        return;

    for ( int i = 0; i < m_certificates.count(); i++ )
        file.write( m_certificates.at( i ).toPem() );
}
