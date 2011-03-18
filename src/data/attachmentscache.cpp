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

#include "attachmentscache.h"
#include "localsettings.h"

#include "application.h"
#include "utils/dataserializer.h"

#include <QFile>
#include <QFileInfo>
#include <QDataStream>

AttachmentsCache::AttachmentsCache( const QString& path, QObject* parent ) : QObject( parent ),
    m_path( path )
{
    openCache();

    flushCache( 0 );

    connect( application->applicationSettings(), SIGNAL( settingsChanged() ), this, SLOT( settingsChanged() ) );
}

AttachmentsCache::~AttachmentsCache()
{
    flushCache( 0 );
}

QString AttachmentsCache::findAttachment( int fileId )
{
    for ( int i = 0; i < m_cachedItems.count(); i++ ) {
        if ( m_cachedItems.at( i ).m_fileId == fileId ) {
            if ( i > 0 ) {
                Item item = m_cachedItems.at( i );
                m_cachedItems.removeAt( i );
                m_cachedItems.prepend( item );
            
                saveCache();
            }
            return application->locateTempFile( m_cachedItems.at( 0 ).m_name );
        }
    }
    return QString();
}

QString AttachmentsCache::allocAttachment( int fileId, const QString& name, int size )
{
    m_allocatedItem.m_fileId = fileId;
    m_allocatedItem.m_name = generateUniqueName( name );
    m_allocatedItem.m_size = size;

    flushCache( size );

    return application->locateTempFile( m_allocatedItem.m_name );
}

void AttachmentsCache::commitAttachment()
{
    m_cachedItems.prepend( m_allocatedItem );

    saveCache();
}

void AttachmentsCache::openCache()
{
    DataSerializer serializer( m_path );

    if ( !serializer.openForReading() )
        return;

    serializer.stream() >> m_cachedItems;
}

void AttachmentsCache::saveCache()
{
    DataSerializer serializer( m_path );

    if ( !serializer.openForWriting() )
        return;

    serializer.stream() << m_cachedItems;
}

void AttachmentsCache::settingsChanged()
{
    flushCache( 0 );
}

void AttachmentsCache::flushCache( int allocated )
{
    LocalSettings* settings = application->applicationSettings();
    int limit = settings->value( "AttachmentsCacheSize" ).toInt() * 1024 * 1024;
    int occupied = ( allocated + 4095 ) & ~4096;

    for ( int i = 0; i < m_cachedItems.count(); i++ )
        occupied += ( m_cachedItems.at( i ).m_size + 4095 ) & ~4096;

    bool modified = false;
    int index = m_cachedItems.count() - 1;

    while ( index >= 0 && occupied > limit ) {
        QString path = application->locateTempFile( m_cachedItems.at( index ).m_name );
        if ( !QFile::exists( path ) || QFile::remove( path ) ) {
            occupied -= ( m_cachedItems.at( index ).m_size + 4095 ) & ~4096;
            m_cachedItems.removeAt( index );
            modified = true;
        }
        index--;
    }

    if ( modified )
        saveCache();
}

QString AttachmentsCache::generateUniqueName( const QString& name )
{
    QString path = application->locateTempFile( name );

    QFileInfo info( path );

    if ( !info.exists() )
        return name;

    QString baseName = info.baseName();
    QString suffix = info.completeSuffix();
    if ( !suffix.isEmpty() )
        suffix.prepend( '.' );

    for ( int number = 2; ; number++ ) {
        QString generatedName = QString( "%1(%2)%3" ).arg( baseName ).arg( number ).arg( suffix );
        path = application->locateTempFile( generatedName );
        if ( !QFile::exists( path ) )
            return generatedName;
    }
}

QDataStream& operator <<( QDataStream& stream, const AttachmentsCache::Item& item )
{
    return stream << item.m_fileId << item.m_name << item.m_size;
}

QDataStream& operator >>( QDataStream& stream, AttachmentsCache::Item& item )
{
    return stream >> item.m_fileId >> item.m_name >> item.m_size;
}
