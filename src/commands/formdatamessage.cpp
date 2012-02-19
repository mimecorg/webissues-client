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

#include "formdatamessage.h"

#include <QDateTime>
#include <QBuffer>

// fix for GCC 4.3-snapshot
#include <cstdlib>

FormDataMessage::FormDataMessage( QObject* parent ) : QIODevice( parent ),
    m_buffer( NULL ),
    m_size( 0 ),
    m_index( 0 )
{
    m_boundary = QString( "nextPart-%1" ).arg( randomString( 12 ) );
    beginBuffer();
}

FormDataMessage::~FormDataMessage()
{
}

void FormDataMessage::addField( const QString& name, const QByteArray& value )
{
    writeLine( QString( "--%1" ).arg( m_boundary ) );
    writeLine( QString( "Content-Disposition: form-data; name=\"%1\"" ).arg( name ) );
    writeLine();
    writeLine( value );
}

void FormDataMessage::addAttachment( const QString& name, const QString& fileName, QIODevice* input )
{
    writeLine( QString( "--%1" ).arg( m_boundary ) );
    writeLine( QString( "Content-Disposition: form-data; name=\"%1\"; filename=\"%2\"" ).arg( name, fileName ) );
    writeLine( QByteArray( "Content-Type: application/octet-stream" ) );
    writeLine();
    endBuffer();
    m_parts.append( input );
    m_size += input->size();
    beginBuffer();
    writeLine();
}

void FormDataMessage::finish()
{
    writeLine( QString( "--%1--" ).arg( m_boundary ) );
    endBuffer();
}

QString FormDataMessage::contentType()
{
    return QString( "multipart/form-data; boundary=%1" ).arg( m_boundary );
}

QString FormDataMessage::randomString( int length )
{
    static bool init = false;
    if ( !init ) {
        srand( QDateTime::currentDateTime().toTime_t() );
        init = true;
    }

    QString result;
    result.resize( length );

    for ( int i = 0; i < length; i++ ) {
        int n = rand() % 62 + 48;
        if ( n > 57 )
            n += 7;
        if ( n > 90 )
            n += 6;
        result[ i ] = (char)n;
    }

    return result;
}

void FormDataMessage::beginBuffer()
{
    m_buffer = new QBuffer( this );
    m_buffer->open( QIODevice::ReadWrite );
}

void FormDataMessage::endBuffer()
{
    m_buffer->reset();
    m_parts.append( m_buffer );
    m_size += m_buffer->size();
    m_buffer = NULL;
}

void FormDataMessage::writeLine()
{
    m_buffer->write( "\r\n", 2 );
}

void FormDataMessage::writeLine( const QString& line )
{
    if ( !line.isEmpty() )
        m_buffer->write( line.toLatin1() );
    writeLine();
}

void FormDataMessage::writeLine( const QByteArray& line )
{
    if ( !line.isEmpty() )
        m_buffer->write( line );
    writeLine();
}

qint64 FormDataMessage::size() const
{
    return m_size;
}

bool FormDataMessage::seek( qint64 pos )
{
    if ( pos == 0 ) {
        m_index = 0;
        for ( int i = 0; i < m_parts.count(); i++ ) {
            if ( !m_parts.at( i )->reset() )
                return false;
        }
        return QIODevice::seek( pos );
    }
    return false;
}

qint64 FormDataMessage::readData( char* data, qint64 maxSize )
{
    if ( m_index >= m_parts.count() )
        return -1;

    qint64 pos = 0;
    while ( pos < maxSize ) {
        qint64 read = m_parts.at( m_index )->read( data + pos, maxSize - pos );
        if ( read < 0 )
            return -1;

        if ( read == 0 ) {
            m_index++;
            if ( m_index >= m_parts.count() )
                break;
        }

        pos += read;
    }

    return pos;
}

qint64 FormDataMessage::writeData( const char* /*data*/, qint64 /*maxSize*/ )
{
    return -1;
}
