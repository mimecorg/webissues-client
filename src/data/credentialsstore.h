/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2017 WebIssues Team
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

#ifndef CREDENTIALSSTORE_H
#define CREDENTIALSSTORE_H

#include "credential.h"

#include <QObject>
#include <QList>

/**
* Class for storing a list of credentials.
*/
class CredentialsStore : public QObject
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param path The path of the file containing credentials.
    * @param parent The parent object.
    */
    CredentialsStore( const QString& path, QObject* parent );

    /**
    * Destructor.
    */
    ~CredentialsStore();

public:
    /**
    * Return all credentials for given WebIssues server.
    */
    QList<ServerCredential> serverCredentials( const QString& serverUuid ) const;

    /**
    * Return HTTP credential for given host.
    */
    HttpCredential httpCredential( const QString& hostName ) const;

    /**
    * Return proxy credential for given host.
    */
    HttpCredential proxyCredential( const QString& hostName ) const;

    /**
    * Add a server credential to the top of the list.
    */
    void addServerCredential( const ServerCredential& credential );

    /**
    * Set HTTP credential for given host.
    */
    void setHttpCredential( const HttpCredential& credential );

    /**
    * Set proxy credential for given host.
    */
    void setProxyCredential( const HttpCredential& credential );

    /**
    * Load credentials from the file.
    */
    void load();

    /**
    * Save credentials to the file.
    */
    void save();

private:
    void deleteByUuid( const QString& uuid );
    void deleteByUrl( const QString& url );

private:
    QString m_path;

    QList<ServerCredential> m_serverCredentials;
    QList<HttpCredential> m_httpCredentials;
    QList<HttpCredential> m_proxyCredentials;
};

#endif
