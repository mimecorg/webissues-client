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

#ifndef FILECACHE_H
#define FILECACHE_H

#include <QObject>
#include <QList>

class QSqlDatabase;

/**
* Class for caching downloaded attachments.
*/
class FileCache : public QObject
{
    Q_OBJECT
public:
    /**
    * Default constructor.
    * @param uuid Unique identifier of the server.
    */
    FileCache( const QString& uuid, const QString& path, QObject* parent );

    /**
    * Destructor.
    */
    ~FileCache();

public:
    /**
    * Locate a file in the cache.
    * @param fileId Identifier of the file.
    * @return Path of the file or empty string if it is not cached.
    */
    QString findFilePath( int fileId );

    /**
    * Generate a unique path in the cache (the file is not created).
    * @param fileId Identifier of the file.
    * @return Path of the new file in the cache.
    */
    QString generateFilePath( const QString& name ) const;

    /**
    * Allocate space in the cache deleting old files if necessary.
    * @param size Amount of space (in bytes) to allocate.
    */
    void allocFileSpace( int size );

    /**
    * Add the file to the cache.
    * @param fileId Identifier of the file.
    * @parm path Path of the file in the cache.
    * @param size Size of the file in bytes.
    */
    void commitFile( int fileId, const QString& path, int size );

private:
    bool installSchema( const QSqlDatabase& database );

    bool allocFileSpace( int allocated, const QSqlDatabase& database );

private:
    QString m_uuid;
};

#endif
