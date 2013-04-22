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
#include "data/query.h"

#include <QFileInfo>
#include <QFile>

FileCache::FileCache( const QString& uuid, const QString& path, QObject* parent ) : QObject( parent ),
    m_uuid( uuid )
{
    QSqlDatabase database = QSqlDatabase::addDatabase( "SQLITEX", "FileCache" );

	database.setDatabaseName( path );

    if ( !database.open() )
        return;

    database.transaction();

    bool ok = installSchema( database );
    if ( ok )
        ok = database.commit();

    if ( !ok ) {
        database.rollback();
        database.close();
		return;
    }

	allocFileSpace( 0 );
}

FileCache::~FileCache()
{
	allocFileSpace( 0 );

	QSqlDatabase database = QSqlDatabase::database( "FileCache" );
    database.close();
}

bool FileCache::installSchema( const QSqlDatabase& database )
{
	const int schemaVersion = 1;

	Query query( database );

    if ( !query.execQuery( "PRAGMA user_version" ) )
        return false;

    int currentVersion = query.readScalar().toInt();

    if ( currentVersion >= schemaVersion )
        return true;

	if ( !query.execQuery( "CREATE TABLE files_cache ( server_uuid text, file_id integer, file_path text UNIQUE, file_size integer, last_access integer, UNIQUE ( server_uuid, file_id ) )" ) )
		return false;

	QString sql = QString( "PRAGMA user_version = %1" ).arg( schemaVersion );

    if ( !query.execQuery( sql ) )
        return false;

    return true;
}

QString FileCache::findFilePath( int fileId )
{
    QSqlDatabase database = QSqlDatabase::database( "FileCache" );

	Query query( database );

	if ( !query.execQuery( "SELECT file_path FROM files_cache WHERE server_uuid = ? AND file_id = ?", m_uuid, fileId ) )
		return QString();

	QString path = query.readScalar().toString();

    if ( path.isEmpty() )
        return QString();

    if ( !QFile::exists( path ) ) {
        query.execQuery( "DELETE FROM files_cache WHERE file_path = ?", path );

        return QString();
    }

    query.execQuery( "UPDATE files_cache SET last_access = strftime( '%s', 'now' ) WHERE file_path = ?", path );

    return path;
}

QString FileCache::generateFilePath( const QString& name ) const
{
    QString path = application->locateSharedCacheFile( "files/" + name );

    QFileInfo info( path );

    if ( !info.exists() )
        return path;

    QString baseName = info.baseName();
    QString suffix = info.completeSuffix();
    if ( !suffix.isEmpty() )
        suffix.prepend( '.' );

    for ( int number = 2; ; number++ ) {
        QString generatedName = QString( "%1(%2)%3" ).arg( baseName ).arg( number ).arg( suffix );
        path = application->locateSharedCacheFile( "files/" + generatedName );
        if ( !QFile::exists( path ) )
            return path;
    }
}

void FileCache::allocFileSpace( int size )
{
    QSqlDatabase database = QSqlDatabase::database( "FileCache" );
    database.transaction();

    bool ok = allocFileSpace( size, database );
    if ( ok )
        ok = database.commit();

    if ( !ok )
        database.rollback();
}

bool FileCache::allocFileSpace( int allocated, const QSqlDatabase& database )
{
	const int maxCount = 100;
	const int maxSize = 50 * 1024 * 1024;

	Query query( database );

	int count = 0;
	int size = 0;

	if ( !query.execQuery( "SELECT COUNT(*), SUM( file_size ) FROM files_cache" ) )
		return false;

	if ( query.next() ) {
		count = query.value( 0 ).toInt();
		size = query.value( 1 ).toInt();
	}

	if ( allocated > 0 ) {
		count++;
		size += allocated;
	}

	QStringList paths;

	if ( !query.execQuery( "SELECT file_path, file_size FROM files_cache ORDER BY last_access" ) )
		return false;

	while ( query.next() ) {
		QString path = query.value( 0 ).toString();
        if ( !QFile::exists( path ) || ( count > maxCount || size > maxSize ) && QFile::remove( path ) ) {
			paths.append( path );
			count--;
			size -= query.value( 1 ).toInt();
		}
	}

	query.setQuery( "DELETE FROM files_cache WHERE file_path = ?" );

	foreach ( const QString& path, paths )
		query.exec( path );

	return true;
}

void FileCache::commitFile( int fileId, const QString& path, int size )
{
    QSqlDatabase database = QSqlDatabase::database( "FileCache" );

	Query query( database );

	query.execQuery( "INSERT INTO files_cache ( server_uuid, file_id, file_path, file_size, last_access ) VALUES ( ?, ?, ?, ?, strftime( '%s', 'now' ) )", m_uuid, fileId, path, size );
}
