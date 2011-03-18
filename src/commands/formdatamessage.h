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

#ifndef FORMDATAMESSAGE_H
#define FORMDATAMESSAGE_H

#include <QIODevice>
#include <QString>
#include <QList>

class QBuffer;

/**
* Class encapsulating a form data message.
*
* This class creates a <tt>multipart/form-data</tt> message as defined in [RFC 2388].
*
* The message may consist of a number of form fields and attachments. It can be used as
* a body of a <tt>POST</tt> request in the <tt>HTTP</tt> protocol.
*/
class FormDataMessage : public QIODevice
{
public:
    /**
    * Default constructor.
    * Create an empty message.
    */
    FormDataMessage( QObject* parent );

    /**
    * Destructor.
    */
    ~FormDataMessage();

public:
    /**
    * Add a form field to the message.
    * @param name Name of the form field.
    * @param value Value of the form field.
    */
    void addField( const QString& name, const QByteArray& value );

    /**
    * Add an attached file to the message.
    * @param name Name of the form field.
    * @param fileName Name of the attached file.
    * @param input Device containg file data.
    */
    void addAttachment( const QString& name, const QString& fileName, QIODevice* input );

    /**
    * Finish the message.
    * This method must be called after adding the last field to properly terminate the message.
    */
    void finish();

    /**
    * Return the <tt>Content-Type</tt> header of the message.
    * @return The <tt>multipart/form-data</tt> content type with appropriate boundary.
    */
    QString contentType();

public: // overrides
    qint64 size() const;
    bool seek( qint64 pos );

protected: // overrides
    qint64 readData( char* data, qint64 maxSize );
    qint64 writeData( const char* data, qint64 maxSize );

private:
    QString randomString( int length );

    void beginBuffer();
    void endBuffer();

    void writeLine();
    void writeLine( const QString& line );
    void writeLine( const QByteArray& line );

private:
    QString m_boundary;
    QBuffer* m_buffer;
    QList<QIODevice*> m_parts;
    qint64 m_size;
    int m_index;
};

#endif
