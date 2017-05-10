/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2017 WebIssues Team
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

#ifndef COMMANDMANAGER_H
#define COMMANDMANAGER_H

#include <QObject>
#include <QUrl>
#include <QList>
#include <QSslConfiguration>

class AbstractBatch;
class Command;
class Reply;
class ReplyLine;
class FormDataMessage;

class QNetworkAccessManager;
class QNetworkReply;
class QAuthenticator;

/**
* Class for communicating with the WebIssues server.
*
* This class contains a priority queue of AbstractBatch objects which provide
* commands to execute. Only a single command is executed at a time. If there are
* two or more batches, commands from the batch with the highest priority are
* executed first. Processing commands is asynchronous.
*
* The instance of this class is available using the commandManager global variable.
* It is created and owned by the ConnectionManager.
*/
class CommandManager : public QObject
{
    Q_OBJECT
public:
    /**
    * Type of error that occurred while executing the command.
    */
    enum Error
    {
        /** No error. */
        NoError,
        /** Connection to the server failed. The errorCode() method returns a QNetworkReply::NetworkError flag. */
        NetworkError,
        /** No WebIssues server was found at the given URL. */
        InvalidServer,
        /** The server version is not supported. */
        InvalidVersion,
        /** The server returned an invalid response. */
        InvalidResponse,
        /** The server returned a WebIssues error. The errorCode() method returns the error code. */
        WebIssuesError,
        /** The request was aborted. */
        Aborted
    };

public:
    /**
    * Constructor.
    */
    CommandManager( QNetworkAccessManager* manager );

    /**
    * Destructor.
    */
    ~CommandManager();

public:
    /**
    * Set the URL of the server to send commands to.
    * @param url URL of the server.
    */
    void setServerUrl( const QUrl& url );

    /**
    * Return the URL of the server.
    */
    QUrl serverUrl() const;

    /**
    * Append the batch to the execution queue.
    * The CommandManager takes ownership of the batch object.
    * @param batch Batch to execute.
    */
    void execute( AbstractBatch* batch );

    /**
    * Abort processing the batch and remove it from the execution queue.
    * The batch object is deleted immediately.
    * @param batch Batch to abort.
    */
    void abort( AbstractBatch* batch );

    /**
    * Abort all batches and clear the execution queue.
    */
    void abortAll();

    /**
    * Return @c true if any pending batch has the prevent close flag set.
    */
    bool preventClose() const;

    /**
    * Return the type of the last error.
    */
    Error error() const { return m_error; }

    /**
    * Return the code of the last error.
    */
    int errorCode() const { return m_errorCode; }

    /**
    * Return full formatted error message;
    */
    QString errorMessage();

#if !defined( QT_NO_OPENSSL )
    /**
    * Return server's SSL configuration.
    */
    QSslConfiguration sslConfiguration() const { return m_sslConfiguration; }
#endif

private:
    void sendSetHostRequest();
    void sendCommandRequest( Command* command );

    void handleCommandReply( const Reply& reply );

    bool parseReply( const QString& string, Reply& reply );
    bool validateReply( const Reply& reply );

    QString makeSignature( const ReplyLine& line );

    QString quoteString( const QString& string );
    QString unquoteString( const QString& string );

    void setError( Error error, int code = 0, const QString& string = QString() );

    void handleAuthentication( int mode, const QString& hostName, QAuthenticator* authenticator );

    QString networkError();
    QString webIssuesError();

private slots:
    void checkPendingCommand();

    void metaDataChanged();
    void readyRead();

    void finished( QNetworkReply* reply );

    void authenticationRequired( QNetworkReply* reply, QAuthenticator* authenticator );
    void proxyAuthenticationRequired( const QNetworkProxy& proxy, QAuthenticator* authenticator );

#if !defined( QT_NO_OPENSSL )
    void handleSslErrors( QNetworkReply* reply, const QList<QSslError>& errors );
#endif

private:
    QNetworkAccessManager* m_manager;

#if !defined( QT_NO_OPENSSL )
    QList<QSslCertificate> m_certificates;

    QSslConfiguration m_sslConfiguration;
#endif

    QList<AbstractBatch*> m_batches;

    QUrl m_url;

    AbstractBatch* m_currentBatch;
    Command* m_currentCommand;
    FormDataMessage* m_currentMessage;
    QNetworkReply* m_currentReply;

    int m_statusCode;
    QUrl m_redirectionTarget;
    QByteArray m_contentType;

    QString m_protocolVersion;

    Error m_error;
    int m_errorCode;
    QString m_errorString;
};

/**
* Global pointer used to access the CommandManager.
*/
extern CommandManager* commandManager;

#endif
