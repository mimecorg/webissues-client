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

#include "filecache.h"

#include "application.h"
#include "data/localsettings.h"
#include "utils/dataserializer.h"

#include <QFileInfo>
#include <QFile>

FileCache::FileCache( const QString& uuid, const QString& path, QObject* parent ) : QObject( parent ),
    m_uuid( uuid ),
    m_path( path )
{
    load();
}

FileCache::~FileCache()
{
}

QString FileCache::findFilePath( int fileId )
{
    int index = -1;

    for ( int i = 0; i < m_items.count(); i++ ) {
        if ( m_items.at( i ).m_uuid == m_uuid && m_items.at( i ).m_fileId == fileId ) {
            index = i;
            break;
        }
    }

    if ( index < 0 )
        return QString();

    QString path = m_items.at( index ).m_path;

    if ( !QFile::exists( path ) ) {
        m_items.removeAt( index );
        save();
        return QString();
    }

    if ( index > 0 ) {
        Item item = m_items.at( index );
        m_items.removeAt( index );
        m_items.prepend( item );
        save();
    }

    return path;
}

QString FileCache::generateFilePath( const QString& name ) const
{
    QString path = application->locateTempFile( name );

    QFileInfo info( path );

    if ( !info.exists() )
        return path;

    QString baseName = info.baseName();
    QString suffix = info.completeSuffix();
    if ( !suffix.isEmpty() )
        suffix.prepend( '.' );

    for ( int number = 2; ; number++ ) {
        QString generatedName = QString( "%1(%2)%3" ).arg( baseName ).arg( number ).arg( suffix );
        path = application->locateTempFile( generatedName );
        if ( !QFile::exists( path ) )
            return path;
    }
}

void FileCache::allocFileSpace( int size )
{
    flush( size );
}

void FileCache::commitFile( int fileId, const QString& path, int size )
{
    Item item = { m_uuid, fileId, path, size };
    m_items.prepend( item );
    save();
}

void FileCache::flush()
{
    flush( 0 );
}

void FileCache::flush( int allocated )
{
    LocalSettings* settings = application->applicationSettings();
    int limit = settings->value( "AttachmentsCacheSize" ).toInt() * 1024 * 1024;

    int occupied = ( allocated + 4095 ) & ~4095;

    for ( int i = 0; i < m_items.count(); i++ )
        occupied += ( m_items.at( i ).m_size + 4095 ) & ~4095;

    bool modified = false;

    for ( int i = m_items.count() - 1; i >= 0; i-- ) {
        QString path = m_items.at( i ).m_path;
        if ( !QFile::exists( path ) || ( ( occupied > limit ) && QFile::remove( path ) ) ) {
            occupied -= ( m_items.at( i ).m_size + 4095 ) & ~4095;
            m_items.removeAt( i );
            modified = true;
        }
    }

    if ( modified )
        save();
}

QDataStream& operator <<( QDataStream& stream, const FileCache::Item& item )
{
    return stream
        << item.m_uuid
        << item.m_fileId
        << item.m_path
        << item.m_size;
}

QDataStream& operator >>( QDataStream& stream, FileCache::Item& item )
{
    return stream
        >> item.m_uuid
        >> item.m_fileId
        >> item.m_path
        >> item.m_size;
}

void FileCache::load()
{
    DataSerializer serializer( m_path );

    if ( !serializer.openForReading() )
        return;

    serializer.stream() >> m_items;
}

void FileCache::save()
{
    DataSerializer serializer( m_path );

    if ( !serializer.openForWriting() )
        return;

    serializer.stream() << m_items;
}
