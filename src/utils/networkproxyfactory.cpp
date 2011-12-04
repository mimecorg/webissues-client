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

#include "networkproxyfactory.h"

#if !defined( NO_PROXY_FACTORY )

#include "application.h"
#include "data/localsettings.h"

#include <QRegExp>
#include <QStringList>

NetworkProxyFactory::NetworkProxyFactory()
{
}

NetworkProxyFactory::~NetworkProxyFactory()
{
}

QList<QNetworkProxy> NetworkProxyFactory::queryProxy( const QNetworkProxyQuery& query )
{
    LocalSettings* settings = application->applicationSettings();

    QNetworkProxy::ProxyType type = (QNetworkProxy::ProxyType)settings->value( "ProxyType" ).toInt();

    if ( type == QNetworkProxy::NoProxy || type == QNetworkProxy::DefaultProxy )
        return QNetworkProxyFactory::systemProxyForQuery( query );

    QStringList excludedPatterns = settings->value( "ProxyExclude" ).toStringList();

    foreach ( const QString& pattern, excludedPatterns ) {
        QRegExp regExp( pattern, Qt::CaseInsensitive, QRegExp::Wildcard );
        if ( regExp.exactMatch( query.peerHostName() ) )
            return QList<QNetworkProxy>() << QNetworkProxy::NoProxy;
    }

    QString hostName = settings->value( "ProxyHost" ).toString();
    quint16 port = (quint16)settings->value( "ProxyPort" ).toInt();

    return QList<QNetworkProxy>() << QNetworkProxy( type, hostName, port );
}

#endif // !defined( NO_PROXY_FACTORY )
