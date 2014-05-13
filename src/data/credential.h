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

#ifndef CREDENTIAL_H
#define CREDENTIAL_H

#include <QString>
#include <QMetaType>

class QDataStream;

/**
* Structure storing information about login and password for WebIssues server.
*/
class ServerCredential
{
public:
    /**
    * Default constructor.
    */
    ServerCredential();

    /**
    * Constructor.
    */
    ServerCredential( const QString& serverUuid, const QString& userName, const QString& login, const QString& password );

    /**
    * Destructor.
    */
    ~ServerCredential();

public:
    /**
    * Return the unique identifier of the server.
    */
    const QString& serverUuid() const { return m_serverUuid; }

    /**
    * Return the name of the user.
    */
    const QString& userName() const { return m_userName; }

    /**
    * Return the login.
    */
    const QString& login() const { return m_login; }

    /**
    * Return the password if available.
    */
    const QString& password() const { return m_password; }

public:
    friend QDataStream& operator <<( QDataStream& stream, const ServerCredential& credential );
    friend QDataStream& operator >>( QDataStream& stream, ServerCredential& credential );

private:
    QString m_serverUuid;
    QString m_userName;
    QString m_login;
    QString m_password;
};

Q_DECLARE_METATYPE( ServerCredential )

/**
* Structure storing information about login and password for HTTP or proxy connection.
*/
class HttpCredential
{
public:
    /**
    * Default constructor.
    */
    HttpCredential();

    /**
    * Constructor.
    */
    HttpCredential( const QString& hostName, const QString& login, const QString& password );

    /**
    * Destructor.
    */
    ~HttpCredential();

public:
    /**
    * Return the host name.
    */
    const QString& hostName() const { return m_hostName; }

    /**
    * Return the login.
    */
    const QString& login() const { return m_login; }

    /**
    * Return the password if available.
    */
    const QString& password() const { return m_password; }

public:
    friend QDataStream& operator <<( QDataStream& stream, const HttpCredential& credential );
    friend QDataStream& operator >>( QDataStream& stream, HttpCredential& credential );

private:
    QString m_hostName;
    QString m_login;
    QString m_password;
};

#endif
