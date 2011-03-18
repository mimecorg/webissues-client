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

#include "credentialsstore.h"

#include "utils/dataserializer.h"

CredentialsStore::CredentialsStore( const QString& path, QObject* parent ) : QObject( parent ),
    m_path( path )
{
    load();
}

CredentialsStore::~CredentialsStore()
{
    save();
}

QList<ServerCredential> CredentialsStore::serverCredentials( const QString& serverUuid ) const
{
    QList<ServerCredential> result;

    for ( int i = 0; i < m_serverCredentials.count(); i++ ) {
        if ( m_serverCredentials.at( i ).serverUuid() == serverUuid )
            result.append( m_serverCredentials.at( i ) );
    }

    return result;
}

HttpCredential CredentialsStore::httpCredential( const QString& hostName ) const
{
    for ( int i = 0; i < m_httpCredentials.count(); i++ ) {
        if ( QString::compare( m_httpCredentials.at( i ).hostName(), hostName, Qt::CaseInsensitive ) == 0 )
            return m_httpCredentials.at( i );
    }
    return HttpCredential();
}

HttpCredential CredentialsStore::proxyCredential( const QString& hostName ) const
{
    for ( int i = 0; i < m_proxyCredentials.count(); i++ ) {
        if ( QString::compare( m_proxyCredentials.at( i ).hostName(), hostName, Qt::CaseInsensitive ) == 0 )
            return m_proxyCredentials.at( i );
    }
    return HttpCredential();
}

void CredentialsStore::addServerCredential( const ServerCredential& credential )
{
    for ( int i = 0; i < m_serverCredentials.count(); i++ ) {
        if ( m_serverCredentials.at( i ).serverUuid() == credential.serverUuid()
             && m_serverCredentials.at( i ).login() == credential.login() ) {
            m_serverCredentials.removeAt( i );
            break;
        }
    }

    m_serverCredentials.prepend( credential );
}

void CredentialsStore::setHttpCredential( const HttpCredential& credential )
{
    for ( int i = 0; i < m_httpCredentials.count(); i++ ) {
        if ( QString::compare( m_httpCredentials.at( i ).hostName(), credential.hostName(), Qt::CaseInsensitive ) == 0 ) {
            m_httpCredentials.removeAt( i );
            break;
        }
    }

    m_httpCredentials.prepend( credential );
}

void CredentialsStore::setProxyCredential( const HttpCredential& credential )
{
    for ( int i = 0; i < m_proxyCredentials.count(); i++ ) {
        if ( QString::compare( m_proxyCredentials.at( i ).hostName(), credential.hostName(), Qt::CaseInsensitive ) == 0 ) {
            m_proxyCredentials.removeAt( i );
            break;
        }
    }

    m_proxyCredentials.prepend( credential );
}

void CredentialsStore::load()
{
    DataSerializer serializer( m_path );

    if ( !serializer.openForReading() )
        return;

    serializer.stream()
        >> m_serverCredentials
        >> m_httpCredentials
        >> m_proxyCredentials;
}

void CredentialsStore::save()
{
    DataSerializer serializer( m_path );

    if ( !serializer.openForWriting() )
        return;

    serializer.stream()
        << m_serverCredentials
        << m_httpCredentials
        << m_proxyCredentials;
}
