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

#ifndef BOOKMARK_H
#define BOOKMARK_H

#include <QString>
#include <QMetaType>

class QDataStream;

/**
* Structure storing information about a bookmark.
*/
class Bookmark
{
public:
    /**
    * Default constructor.
    */
    Bookmark();

    /**
    * Constructor.
    */
    Bookmark( const QString& serverName, const QString& serverUuid, const QString& url );

    /**
    * Destructor.
    */
    ~Bookmark();

public:
    /**
    * Return the name of the server.
    */
    const QString& serverName() const { return m_serverName; }

    /**
    * Return the unique identifier of the server.
    */
    const QString& serverUuid() const { return m_serverUuid; }

    /**
    * Return the URL of the server.
    */
    const QString& url() const { return m_url; }

public:
    friend QDataStream& operator <<( QDataStream& stream, const Bookmark& bookmark );
    friend QDataStream& operator >>( QDataStream& stream, Bookmark& bookmark );

private:
    QString m_serverName;
    QString m_serverUuid;
    QString m_url;
};

Q_DECLARE_METATYPE( Bookmark )

#endif
