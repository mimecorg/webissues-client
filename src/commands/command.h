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

#ifndef COMMAND_H
#define COMMAND_H

#include "reply.h"

#include <QObject>
#include <QList>
#include <QVariant>

class QIODevice;

/**
* Class storing information about a command to execute.
*
* This class stores the command keyword and arguments, optional attachment
* and rules for validating the response. This information is used by CommandManager
* to send a request to the WebIssues server.
*
* This class also provides signals for updating progress and for processing
* the server's reply to the command.
*
* In order to provide commands to be executed by the CommandManager, create
* a subclass of the AbstractBatch and implement the fetchNext() method which
* creates the Command objects.
*/
class Command : public QObject
{
    Q_OBJECT
public:
    /**
    * Default constructor.
    */
    Command();

    /**
    * Destructor.
    */
    ~Command();

public:
    /**
    * Set the command keyword.
    * Refer to the WebIssues protocol documentation for a list of recognized commands.
    */
    void setKeyword( const QString& keyword ) { m_keyword = keyword; }

    /**
    * Return the command keyword.
    */
    const QString& keyword() const { return m_keyword; }

    /**
    * Set a list of command arguments.
    * The arguments can be integers or strings. Refer to the WebIssues protocol
    * documentation for a list of arguments for each command.
    */
    void setArgs( const QVariantList& args ) { m_args = args; }

    /**
    * Return the list of command arguments.
    */
    const QVariantList& args() const { return m_args; }

    /**
    * Add an integer value to the list of command arguments.
    */
    void addArg( int number );

    /**
    * Add a string value to the list of command arguments.
    */
    void addArg( const QString& string );

    /**
    * Return the given argument.
    */
    const QVariant& arg( int index ) const { return m_args.at( index ); }

    /**
    * Read the given integer argument from the line.
    */
    int argInt( int index ) const { return m_args.at( index ).toInt(); }

    /**
    * Read the given string argument from the line.
    */
    QString argString( int index ) const { return m_args.at( index ).toString(); }

    /**
    * Set the attachment data for the command.
    * The attachment is sent to the server together with the command.
    * @param input Device containing attachment data.
    */
    void setAttachmentInput( QIODevice* input ) { m_attachmentInput = input; }

    /**
    * Return the device containing attachment data for the command.
    */
    QIODevice* attachmentInput() const { return m_attachmentInput; }

    /**
    * Set the device for writing binary response.
    */
    void setBinaryResponseOutput( QIODevice* output ) { m_binaryResponseOutput = output; }

    /**
    * Return the device for writing binary response.
    */
    QIODevice* binaryResponseOutput() const { return m_binaryResponseOutput; }

    /**
    * Set a flag indicating if a @c NULL reply is accepted.
    * If the server returns a @c NULL reply, the validation rules are not checked.
    */
    void setAcceptNullReply( bool accept ) { m_acceptNullReply = accept; }

    /**
    * Return @c true if a @c NULL reply is accepted.
    */
    bool acceptNullReply() const { return m_acceptNullReply; }

    /**
    * Set a flag indicating if the commandReply() signal is emitted with an empty reply.
    */
    void setReportNullReply( bool report ) { m_reportNullReply = report; }

    /**
    * Return @c true if the commandReply() signal is emitted with an empty reply.
    */
    bool reportNullReply() const { return m_reportNullReply; }

    /**
    * Set a list of rules for validating the reply.
    * The server's reply is validated against this list. Each line must match the
    * corresponding rule in the list, in the same order as they are defined. Otherwise
    * an error is reported and processing the command is aborted.
    */
    void setRules( const QList<ReplyRule>& rules ) { m_rules = rules; }

    /**
    * Get the list of rules for validating the reply.
    */
    const QList<ReplyRule>& rules() const { return m_rules; }

    /**
    * Add a rule to the list of rules for validating the reply.
    * @param signature The signature of the reply line.
    * @param multiplicity A flag indicating how many times the line can occur.
    */
    void addRule( const QString& signature, ReplyRule::Multiplicity multiplicity );

signals:
    /**
    * Emitted when the progress of sending the request changes.
    * This signal is emitted while sending the request to the server.
    */
    void uploadProgress( qint64 sent, qint64 total );

    /**
    * Emitted when the progress of reading the response changes.
    * This signal is emitted while receiving the response from the server.
    */
    void downloadProgress( qint64 received, qint64 total );

    /**
    * Emitted when the reply is received and validated.
    * This signal is emitted after receiving and validating the server's reply.
    * The reply is guaranteed to be valid according to the validation rules.
    */
    void commandReply( const Reply& reply );

private:
    QString m_keyword;
    QVariantList m_args;

    QIODevice* m_attachmentInput;
    QIODevice* m_binaryResponseOutput;

    bool m_acceptNullReply;
    QList<ReplyRule> m_rules;

    bool m_reportNullReply;
};

#endif
