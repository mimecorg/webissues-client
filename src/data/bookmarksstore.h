/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2012 WebIssues Team
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

#ifndef BOOKMARKSTORE_H
#define BOOKMARKSTORE_H

#include "bookmark.h"

#include <QObject>
#include <QList>

/**
* Class for storing a list of bookmarks.
*/
class BookmarksStore : public QObject
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param path The path of the file containing bookmarks.
    * @param parent The parent object.
    */
    BookmarksStore( const QString& path, QObject* parent );

    /**
    * Destructor.
    */
    ~BookmarksStore();

public:
    /**
    * Return all existing bookmarks.
    */
    const QList<Bookmark>& bookmarks() const { return m_bookmarks; }

    /**
    * Add a bookmark to the top of the list.
    */
    void addBookmark( const Bookmark& bookmark );

    /**
    * Delete the given bookmark.
    */
    void deleteBookmark( const Bookmark& bookmark );

    /**
    * Load bookmarks from the file.
    */
    void load();

    /**
    * Save bookmarks to the file.
    */
    void save();

private:
    void deleteByUuid( const QString& uuid );
    void deleteByUrl( const QString& url );

private:
    QString m_path;

    QList<Bookmark> m_bookmarks;
};

#endif
