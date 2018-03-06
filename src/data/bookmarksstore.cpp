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

#include "bookmarksstore.h"

#include "utils/dataserializer.h"

BookmarksStore::BookmarksStore( const QString& path, QObject* parent ) : QObject( parent ),
    m_path( path )
{
    load();
}

BookmarksStore::~BookmarksStore()
{
    save();
}

void BookmarksStore::addBookmark( const Bookmark& bookmark )
{
    deleteByUuid( bookmark.serverUuid() );
    deleteByUrl( bookmark.url() );

    m_bookmarks.prepend( bookmark );
}

void BookmarksStore::deleteBookmark( const Bookmark& bookmark )
{
    deleteByUuid( bookmark.serverUuid() );
}

void BookmarksStore::load()
{
    DataSerializer serializer( m_path );

    if ( !serializer.openForReading() )
        return;

    serializer.stream() >> m_bookmarks;
}

void BookmarksStore::save()
{
    DataSerializer serializer( m_path );

    if ( !serializer.openForWriting() )
        return;

    serializer.stream() << m_bookmarks;
}

void BookmarksStore::deleteByUuid( const QString& uuid )
{
    for ( int i = 0; i < m_bookmarks.count(); i++ ) {
        if ( m_bookmarks.at( i ).serverUuid() == uuid ) {
            m_bookmarks.removeAt( i );
            break;
        }
    }
}

void BookmarksStore::deleteByUrl( const QString& url )
{
    for ( int i = 0; i < m_bookmarks.count(); i++ ) {
        if ( QString::compare( m_bookmarks.at( i ).url(), url, Qt::CaseInsensitive ) == 0 ) {
            m_bookmarks.removeAt( i );
            break;
        }
    }
}
