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

#include "credential.h"

#include <QDataStream>

#if defined( Q_OS_WIN ) && !defined( Q_CC_MINGW )
#define HAVE_WINCRYPT
#include <qt_windows.h>
#include <wincrypt.h>
#endif

#if !defined( HAVE_WINCRYPT )
static QByteArray obscure( const QByteArray& value )
{
    QByteArray result = value;

    char* data = result.data();
    for ( int i = 0; i < value.length(); i++ )
        data[ i ] ^= (char)( 127 - i * i );

    return result;
}
#endif

static QByteArray encrypt( const QString& value )
{
    if ( value.isEmpty() )
        return QByteArray();

    QByteArray data = value.toUtf8();

#if defined( HAVE_WINCRYPT )
    DATA_BLOB input;
    input.pbData = (BYTE*)data.data();
    input.cbData = data.length();

    DATA_BLOB output;

    if ( !CryptProtectData( &input, L"", NULL, NULL, NULL, 0, &output ) )
        return QByteArray();

    QByteArray result( (char*)output.pbData, output.cbData );

    LocalFree( output.pbData );

    return result;
#else
    return obscure( data );
#endif
}

static QString decrypt( const QByteArray& value )
{
    if ( value.isEmpty() )
        return QString();

#if defined( HAVE_WINCRYPT )
    DATA_BLOB input;
    input.pbData = (BYTE*)value.data();
    input.cbData = value.length();

    DATA_BLOB output;

    if ( !CryptUnprotectData( &input, NULL, NULL, NULL, NULL, 0, &output ) )
        return QString();

    QByteArray data( (char*)output.pbData, output.cbData );

    LocalFree( output.pbData );
#else
    QByteArray data = obscure( value );
#endif

    return QString::fromUtf8( data );
}

ServerCredential::ServerCredential()
{
}

ServerCredential::ServerCredential( const QString& serverUuid, const QString& userName, const QString& login, const QString& password ) :
    m_serverUuid( serverUuid ),
    m_userName( userName ),
    m_login( login ),
    m_password( password )
{
}

ServerCredential::~ServerCredential()
{
}

QDataStream& operator <<( QDataStream& stream, const ServerCredential& credential )
{
    return stream
        << credential.m_serverUuid
        << credential.m_userName
        << credential.m_login
        << encrypt( credential.m_password );
}

QDataStream& operator >>( QDataStream& stream, ServerCredential& credential )
{
    QByteArray password;
    stream
        >> credential.m_serverUuid
        >> credential.m_userName
        >> credential.m_login
        >> password;
    credential.m_password = decrypt( password );
    return stream;
}

HttpCredential::HttpCredential()
{
}

HttpCredential::HttpCredential( const QString& hostName, const QString& login, const QString& password ) :
    m_hostName( hostName ),
    m_login( login ),
    m_password( password )
{
}

HttpCredential::~HttpCredential()
{
}

QDataStream& operator <<( QDataStream& stream, const HttpCredential& credential )
{
    return stream
        << credential.m_hostName
        << credential.m_login
        << encrypt( credential.m_password );
}

QDataStream& operator >>( QDataStream& stream, HttpCredential& credential )
{
    QByteArray password;
    stream
        >> credential.m_hostName
        >> credential.m_login
        >> password;
    credential.m_password = decrypt( password );
    return stream;
}
