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

#ifndef ATTACHMENTSCACHE_H
#define ATTACHMENTSCACHE_H

#include <QObject>
#include <QList>

/**
* Class for caching downloaded attachments.
*/
class AttachmentsCache : public QObject
{
    Q_OBJECT
public:
    /**
    * Constructor.
    */
    AttachmentsCache( const QString& path, QObject* parent );

    /**
    * Destructor.
    */
    ~AttachmentsCache();

public:
    /**
    * Find an attachment in the cache.
    * @param fileId Identifier of the attachment.
    * @return The path of the file or an empty string.
    */
    QString findAttachment( int fileId );

    /**
    * Allocate a path for a new attachment.
    * @param fileId Identifier of the attachment.
    * @param name Original name of the attachment.
    * @param size Size of the attachment.
    * @return The path of the file.
    */
    QString allocAttachment( int fileId, const QString& name, int size );

    /**
    * Commit the allocated attachment into the cache.
    */
    void commitAttachment();

private:
    void openCache();
    void saveCache();

    void flushCache( int allocated );

    QString generateUniqueName( const QString& name );

private slots:
    void settingsChanged();

private:
    struct Item
    {
        int m_fileId;
        QString m_name;
        int m_size;
    };

    friend QDataStream& operator <<( QDataStream& stream, const AttachmentsCache::Item& item );
    friend QDataStream& operator >>( QDataStream& stream, AttachmentsCache::Item& item );

private:
    QString m_path;

    QList<Item> m_cachedItems;

    Item m_allocatedItem;
};

#endif
