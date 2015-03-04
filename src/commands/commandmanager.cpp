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

#include "commandmanager.h"
#include "command.h"
#include "abstractbatch.h"
#include "formdatamessage.h"

#include "application.h"
#include "data/localsettings.h"
#include "data/credentialsstore.h"
#include "data/certificatesstore.h"
#include "dialogs/logindialog.h"
#include "dialogs/ssldialogs.h"
#include "utils/errorhelper.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkProxy>
#include <QAuthenticator>
#include <QRegExp>
#include <QStringList>

CommandManager* commandManager = NULL;

CommandManager::CommandManager( QNetworkAccessManager* manager ) :
    m_manager( manager ),
    m_currentBatch( NULL ),
    m_currentCommand( NULL ),
    m_currentMessage( NULL ),
    m_currentReply( NULL ),
    m_statusCode( 0 ),
    m_error( NoError ),
    m_errorCode( 0 )
{
    connect( m_manager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT( finished( QNetworkReply* ) ) );

    connect( m_manager, SIGNAL( authenticationRequired( QNetworkReply*, QAuthenticator* ) ),
        this, SLOT( authenticationRequired( QNetworkReply*, QAuthenticator* ) ) );
    connect( m_manager, SIGNAL( proxyAuthenticationRequired( const QNetworkProxy&, QAuthenticator* ) ),
        this, SLOT( proxyAuthenticationRequired( const QNetworkProxy&, QAuthenticator* ) ) );

#if !defined( QT_NO_OPENSSL )
    connect( m_manager, SIGNAL( sslErrors( QNetworkReply*, const QList<QSslError>& ) ),
        this, SLOT( handleSslErrors( QNetworkReply*, const QList<QSslError>& ) ) );
#endif
}

CommandManager::~CommandManager()
{
    qDeleteAll( m_batches );
}

void CommandManager::setServerUrl( const QUrl& url )
{
    QString path = url.path();

    if ( path.endsWith( QLatin1String( "index.php" ), Qt::CaseInsensitive ) )
        path.truncate( path.length() - 9 );
    if ( path.endsWith( QLatin1String( "client/" ), Qt::CaseInsensitive ) )
        path.truncate( path.length() - 7 );

    if ( !path.endsWith( QLatin1String( ".php" ), Qt::CaseInsensitive ) ) {
        if ( !path.endsWith( QChar( '/' ) ) )
            path += QChar( '/' );
        path += QLatin1String( "server/webissues/handler.php" );
    }

    m_url = url;
    m_url.setPath( path );
}

QUrl CommandManager::serverUrl() const
{
    QUrl url = m_url;
    QString path = url.path();

    if ( path.endsWith( QLatin1String( "server/webissues/handler.php" ), Qt::CaseInsensitive ) )
        path.truncate( path.length() - 28 );

    url.setPath( path );
    return url;
}

void CommandManager::execute( AbstractBatch* batch )
{
    int pos = 0;
    for ( int i = 0; i < m_batches.count(); i++ ) {
        if ( m_batches.at( i )->priority() < batch->priority() )
            break;
        pos++;
    }
    m_batches.insert( pos, batch );

    checkPendingCommand();
}

void CommandManager::abort( AbstractBatch* batch )
{
    if ( batch == m_currentBatch ) {
        m_currentReply->abort();
    } else {
        setError( Aborted );
        m_batches.removeAt( m_batches.indexOf( batch ) );
        QMetaObject::invokeMethod( batch, "completed", Q_ARG( bool, false ) );
        delete batch;
    }
}

void CommandManager::abortAll()
{
    if ( m_currentReply )
        m_currentReply->abort();

    setError( Aborted );

    while ( !m_batches.isEmpty() ) {
        AbstractBatch* batch = m_batches.takeFirst();
        QMetaObject::invokeMethod( batch, "completed", Q_ARG( bool, false ) );
        delete batch;
    }
}

bool CommandManager::preventClose() const
{
    if ( m_currentBatch && m_currentBatch->preventClose() )
        return true;

    foreach ( AbstractBatch* batch, m_batches ) {
        if ( batch->preventClose() )
            return true;
    }

    return false;
}

QString CommandManager::errorMessage()
{
    switch ( m_error ) {
        case Aborted:
            return tr( "Request was aborted" );
        case InvalidServer:
            return tr( "This is not a WebIssues Server" );
        case InvalidVersion:
            return tr( "This is an unsupported version of WebIssues Server" );
        case InvalidResponse:
            return tr( "Server returned an invalid response" );
        case NetworkError:
            return networkError();
        case WebIssuesError:
            return webIssuesError();
        default:
            return QString();
    }
}

QString CommandManager::networkError()
{
    QNetworkReply::NetworkError error = (QNetworkReply::NetworkError)m_errorCode;

    if ( error <= QNetworkReply::UnknownNetworkError ) {
        switch ( error ) {
            case QNetworkReply::ConnectionRefusedError:
                return tr( "The remote server refused the connection" );
            case QNetworkReply::RemoteHostClosedError:
                return tr( "The remote server closed the connection" );
            case QNetworkReply::HostNotFoundError:
                return tr( "The remote host name was not found" );
            case QNetworkReply::TimeoutError:
                return tr( "Connection to the remote server timed out" );
            case QNetworkReply::SslHandshakeFailedError:
                return tr( "A secure connection could not be established" );
            default:
                return tr( "A network error occurred" );
        }
    }

    if ( error <= QNetworkReply::UnknownProxyError ) {
        switch ( error ) {
            case QNetworkReply::ProxyConnectionRefusedError:
                return tr( "The proxy refused the connection" );
            case QNetworkReply::ProxyConnectionClosedError:
                return tr( "The proxy closed the connection" );
            case QNetworkReply::ProxyNotFoundError:
                return tr( "The proxy host name was not found" );
            case QNetworkReply::ProxyTimeoutError:
                return tr( "Connection to the proxy timed out" );
            case QNetworkReply::ProxyAuthenticationRequiredError:
                return tr( "You have no permission to access the proxy" );
            default:
                return tr( "A proxy error occurred" );
        }
    }

    if ( error <= QNetworkReply::UnknownContentError ) {
        switch ( error ) {
            case QNetworkReply::ContentAccessDenied:
            case QNetworkReply::ContentOperationNotPermittedError:
            case QNetworkReply::AuthenticationRequiredError:
                return tr( "You have no permission to access the requested URL" );
            case QNetworkReply::ContentNotFoundError:
                return tr( "The requested URL was not found" );
            default:
                if ( m_statusCode == 406 )
                    return tr( "The request was not accepted by the server" );
                else
                    return tr( "Server returned an invalid response" );
        }
    }

    if ( m_statusCode >= 500 )
        return tr( "An internal server error occurred" );

    if ( error == QNetworkReply::ProtocolUnknownError )
        return tr( "Protocol is not supported" );

    return tr( "An unknown error occurred" );
}

QString CommandManager::webIssuesError()
{
    if ( m_errorCode >= 500 ) {
        switch ( m_errorCode ) {
            case 501:
            case 502:
                return tr( "The WebIssues Server is not correctly configured" );
            default:
                return tr( "An internal server error occurred" );
        }
    }

    if ( m_errorCode >= 400 ) {
        switch ( m_errorCode ) {
            case 403:
                return tr( "File could not be uploaded" );
            default:
                return tr( "Request could not be processed" );
        }
    }

    ErrorHelper helper;
    QString message = helper.errorMessage( (ErrorHelper::ErrorCode)m_errorCode );

    if ( message.isEmpty() )
        message = tr( "An unknown error occurred (%1 %2)" ).arg( m_errorCode ).arg( m_errorString );

    return message;
}

void CommandManager::checkPendingCommand()
{
    if ( m_currentBatch )
        return;

    while ( !m_batches.isEmpty() ) {
        AbstractBatch* batch = m_batches.first();

        Command* command = batch->fetchNext();
        if ( command ) {
            m_currentBatch = batch;
            m_currentCommand = command;
            sendCommandRequest( command );
            break;
        }

        m_batches.removeFirst();
        QMetaObject::invokeMethod( batch, "completed", Q_ARG( bool, true ) );
        delete batch;

        if ( m_currentBatch )
            break;
    }
}

static QString userAgent()
{
    QString agent = "Mozilla/5.0 (";

#if defined( Q_WS_WIN )
    agent += QLatin1String( "Windows" );
#elif defined( Q_WS_MAC )
    agent += QLatin1String( "Mac OS X" );
#elif defined( Q_WS_X11 )
    agent += QLatin1String( "X11; " );
#if defined( Q_OS_LINUX )
    agent += QLatin1String( "Linux" );
#else
    agent += QLatin1String( "Unknown" );
#endif
#else
    agent += QLatin1String( "Unknown" );
#endif

    agent += QLatin1String( ") WebIssues/" );
    agent += application->version();

    agent += QLatin1String( " Qt/" );
    agent += qVersion();

    return agent;
}

void CommandManager::sendCommandRequest( Command* command )
{
    QNetworkRequest request( m_url );

    QString commandLine = command->keyword();

    for ( int i = 0; i < command->args().count(); i++ ) {
        commandLine += QLatin1Char( ' ' );
        QVariant arg = command->args().at( i );
        if ( arg.type() == QVariant::String )
            commandLine += quoteString( arg.toString() );
        else
            commandLine += arg.toString();
    }

    m_currentMessage = new FormDataMessage( command );

    m_currentMessage->addField( "command", commandLine.toUtf8() );

    if ( command->attachmentInput() )
        m_currentMessage->addAttachment( "file", "file", command->attachmentInput() );

    m_currentMessage->finish();
    m_currentMessage->open( QIODevice::ReadOnly );

    request.setHeader( QNetworkRequest::ContentTypeHeader, m_currentMessage->contentType() );

    request.setRawHeader( "User-Agent", userAgent().toLatin1() );

    if ( command->binaryResponseOutput() )
        request.setRawHeader( "Accept", "application/octet-stream,*/*" );
    else
        request.setRawHeader( "Accept", "text/plain,*/*" );

    m_statusCode = 0;
    setError( NoError );

    m_currentReply = m_manager->post( request, m_currentMessage );

    connect( m_currentReply, SIGNAL( downloadProgress( qint64, qint64 ) ), command, SIGNAL( downloadProgress( qint64, qint64 ) ) );
    connect( m_currentReply, SIGNAL( uploadProgress( qint64, qint64 ) ), command, SIGNAL( uploadProgress( qint64, qint64 ) ) );

    connect( m_currentReply, SIGNAL( metaDataChanged() ), this, SLOT( metaDataChanged() ) );
    connect( m_currentReply, SIGNAL( readyRead() ), this, SLOT( readyRead() ) );
}

void CommandManager::metaDataChanged()
{
    m_statusCode = m_currentReply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
    m_redirectionTarget = m_currentReply->attribute( QNetworkRequest::RedirectionTargetAttribute ).toUrl();
    m_contentType = m_currentReply->header( QNetworkRequest::ContentTypeHeader ).toByteArray();
    m_protocolVersion = m_currentReply->rawHeader( "X-WebIssues-Version" );
#if !defined( QT_NO_OPENSSL )
    m_sslConfiguration = m_currentReply->sslConfiguration();
#endif

    if ( m_statusCode != 200 )
        return;

    if ( m_protocolVersion.isEmpty() ) {
        setError( InvalidServer );
        return;
    }

    if ( m_protocolVersion != application->protocolVersion() ) {
        setError( InvalidVersion );
        return;
    }

    int pos = m_contentType.indexOf( ';' );
    if ( pos >= 0 ) {
        m_contentType.truncate( pos );
        m_contentType = m_contentType.trimmed();
    }

    if ( m_contentType == "application/octet-stream" ) {
        if ( !m_currentCommand->binaryResponseOutput() )
            setError( InvalidResponse );
        return;
    }

    if ( m_contentType != "text/plain" )
        setError( InvalidResponse );
}

void CommandManager::readyRead()
{
    if ( m_error == NoError && m_statusCode == 200 && m_contentType == "application/octet-stream" ) {
        int length;
        char buffer[ 8192 ];
        while ( ( length = m_currentReply->read( buffer, 8192 ) ) > 0 )
            m_currentCommand->binaryResponseOutput()->write( buffer, length );
    }
}

void CommandManager::finished( QNetworkReply* reply )
{
    if ( reply != m_currentReply )
        return;

    m_currentReply = NULL;

    if ( reply->error() != QNetworkReply::NoError ) {
        if ( reply->error() == QNetworkReply::OperationCanceledError )
            setError( Aborted );
        else 
            setError( NetworkError, reply->error() );
    }

    if ( m_error == NoError && m_redirectionTarget.isValid() ) {
        m_url = m_url.resolved( m_redirectionTarget );
        m_currentMessage->reset();
        sendCommandRequest( m_currentCommand );
        reply->deleteLater();
        return;
    }

    if ( m_error == NoError && m_statusCode != 200 )
        setError( CommandManager::InvalidResponse );

    if ( m_error == NoError && m_contentType == "text/plain" ) {
        QByteArray body = reply->readAll();
        Reply reply;
        if ( parseReply( QString::fromUtf8( body.data(), body.size() ), reply ) )
            handleCommandReply( reply );
        else
            setError( InvalidResponse );
    }

    if ( m_error != NoError ) {
        m_batches.removeAt( m_batches.indexOf( m_currentBatch ) );
        QMetaObject::invokeMethod( m_currentBatch, "completed", Q_ARG( bool, false ) );
        delete m_currentBatch;
    }

    m_currentCommand->deleteLater();
    reply->deleteLater();

    m_currentBatch = NULL;
    m_currentCommand = NULL;
    m_currentMessage = NULL;

    QMetaObject::invokeMethod( this, "checkPendingCommand", Qt::QueuedConnection );
}

void CommandManager::handleCommandReply( const Reply& reply )
{
    bool isNull = false;

    if ( reply.lines().count() == 1 ) {
        ReplyLine line = reply.lines().at( 0 );
        QString signature = makeSignature( line );

        if ( signature == QLatin1String( "ERROR is" ) ) {
            setError( WebIssuesError, line.argInt( 0 ), line.argString( 1 ) );
            return;
        }

        if ( signature == QLatin1String( "NULL" ) )
            isNull = true;
    }

    bool isValid = isNull ? m_currentCommand->acceptNullReply() : validateReply( reply );

    if ( !isValid ) {
        setError( InvalidResponse );
        return;
    }

    if ( !isNull )
        QMetaObject::invokeMethod( m_currentCommand, "commandReply", Q_ARG( Reply, reply ) );
    else if ( m_currentCommand->reportNullReply() )
        QMetaObject::invokeMethod( m_currentCommand, "commandReply", Q_ARG( Reply, Reply() ) );
}

bool CommandManager::parseReply( const QString& string, Reply& reply )
{
    QStringList lines = string.split( "\r\n", QString::SkipEmptyParts );

    QString patternNumber = "-?\\d+";
    QString patternString = "'(?:\\\\['\\\\nt]|[^'\\\\])*'";
    QString patternArgument = QString( "(%1|%2)" ).arg( patternNumber, patternString );

    QRegExp lineRegExp( QString( "([A-Z]+)(?: %1)*" ).arg( patternArgument ) );
    QRegExp argumentRegExp( patternArgument );

    for ( QStringList::iterator it = lines.begin(); it != lines.end(); ++it ) {
        if ( !lineRegExp.exactMatch( *it ) )
            return false;

        ReplyLine line;
        line.setKeyword( lineRegExp.cap( 1 ) );

        int pos = 0;
        while ( ( pos = argumentRegExp.indexIn( *it, pos ) ) >= 0 ) {
            QString argument = argumentRegExp.cap( 0 );
            if ( argument[ 0 ] == QLatin1Char( '\'' ) )
                line.addArg( unquoteString( argument ) );
            else
                line.addArg( argument.toInt() );
            pos += argumentRegExp.matchedLength();
        }

        reply.addLine( line );
    }

    return true;
}

bool CommandManager::validateReply( const Reply& reply )
{
    int line = 0;
    int rule = 0;

    while ( line < reply.lines().count() && rule < m_currentCommand->rules().count() ) {
        ReplyRule replyRule = m_currentCommand->rules().at( rule );
        if ( makeSignature( reply.lines().at( line ) ) == replyRule.signature() ) {
            line++;
            if ( replyRule.multiplicity() == ReplyRule::One || replyRule.multiplicity() == ReplyRule::ZeroOrOne )
                rule++;
        } else {
            if ( replyRule.multiplicity() == ReplyRule::One )
                return false;
            rule++;
        }
    }

    while ( rule < m_currentCommand->rules().count() ) {
        if ( m_currentCommand->rules().at( rule ).multiplicity() == ReplyRule::One )
            return false;
        rule++;
    }

    if ( line < reply.lines().count() )
        return false;

    return true;
}

QString CommandManager::makeSignature( const ReplyLine& line )
{
    if ( line.args().isEmpty() )
        return line.keyword();

    QString signature = line.keyword() + ' ';

    for ( int i = 0; i < line.args().count(); i++ ) {
        switch ( line.args().at( i ).type() ) {
            case QVariant::Int:
                signature += QLatin1Char( 'i' );
                break;
            case QVariant::String:
                signature += QLatin1Char( 's' );
                break;
            default:
                signature += QLatin1Char( '?' );
                break;
        }
    }

    return signature;
}

QString CommandManager::quoteString( const QString& string )
{
    QString result = "\'";
    int length = string.length();
    for ( int i = 0; i < length; i++ ) {
        QChar ch = string[ i ];
        if  ( ch == QLatin1Char( '\\' ) || ch == QLatin1Char( '\'' ) || ch == QLatin1Char( '\n' ) || ch == QLatin1Char( '\t' ) ) {
            result += QLatin1Char( '\\' );
            if ( ch == QLatin1Char( '\n' ) )
                ch = QLatin1Char( 'n' );
            else if ( ch == QLatin1Char( '\t' ) )
                ch = QLatin1Char( 't' );
        }
        result += ch;
    }
    result += QLatin1Char( '\'' );
    return result;
}

QString CommandManager::unquoteString( const QString& string )
{
    QString result = "";
    int length = string.length();
    for ( int i = 1; i < length - 1; i++ ) {
        QChar ch = string[ i ];
        if ( ch == QLatin1Char( '\\' ) ) {
            ch = string[ ++i ];
            if ( ch == QLatin1Char( 'n' ) )
                ch = QLatin1Char( '\n' );
            else if ( ch == QLatin1Char( 't' ) )
                ch = QLatin1Char( '\t' );
        }
        result += ch;
    }
    return result;
}

void CommandManager::setError( Error error, int code, const QString& string )
{
    m_error = error;
    m_errorCode = code;
    m_errorString = string;
}

void CommandManager::authenticationRequired( QNetworkReply* reply, QAuthenticator* authenticator )
{
    handleAuthentication( LoginDialog::HttpMode, reply->url().host(), authenticator );
}

void CommandManager::proxyAuthenticationRequired( const QNetworkProxy& proxy, QAuthenticator* authenticator )
{
    handleAuthentication( LoginDialog::ProxyMode, proxy.hostName(), authenticator );
}

void CommandManager::handleAuthentication( int mode, const QString& hostName, QAuthenticator* authenticator )
{
    CredentialsStore* store = application->credentialsStore();

    HttpCredential credential;
    switch ( mode ) {
        case LoginDialog::HttpMode:
            credential = store->httpCredential( hostName );
            break;
        case LoginDialog::ProxyMode:
            credential = store->proxyCredential( hostName );
            break;
    }

    QString login = credential.login();
    QString password = credential.password();

    if ( login.isEmpty() || password.isEmpty() || !authenticator->user().isEmpty() ) {
        LoginDialog dialog( (LoginDialog::DialogMode)mode, hostName, authenticator->realm(), QApplication::activeWindow() );
        dialog.setLoginAndPassword( login, password );
        if ( !authenticator->user().isEmpty() )
            dialog.showLoginError();

        if ( dialog.exec() != QDialog::Accepted )
            return;

        login = dialog.login();
        password = dialog.password();

        HttpCredential newCredential( hostName, login, dialog.rememberPassword() ? password : QString() );
        switch ( mode ) {
            case LoginDialog::HttpMode:
                store->setHttpCredential( newCredential );
                break;
            case LoginDialog::ProxyMode:
                store->setProxyCredential( newCredential );
                break;
        }
    }

    authenticator->setUser( login );
    authenticator->setPassword( password );
}

#if !defined( QT_NO_OPENSSL )

void CommandManager::handleSslErrors( QNetworkReply* reply, const QList<QSslError>& errors )
{
    m_sslConfiguration = reply->sslConfiguration();

    QSslCertificate certificate = m_sslConfiguration.peerCertificate();
    if ( certificate.isNull() )
        return;

    if ( m_certificates.contains( certificate ) ) {
        reply->ignoreSslErrors();
        return;
    }

    CertificatesStore* store = application->certificatesStore();

    if ( store->containsCertificate( certificate ) ) {
        m_certificates.append( certificate );
        reply->ignoreSslErrors();
        return;
    }

    SslErrorsDialog dialog( QApplication::activeWindow() );
    dialog.setErrors( errors );
    dialog.setCertificates( m_sslConfiguration.peerCertificateChain() );

    if ( dialog.exec() != QDialog::Accepted )
        return;

    if ( dialog.acceptPermanently() )
        store->addCertificate( certificate );

    m_certificates.append( certificate );
    reply->ignoreSslErrors();
}

#endif // !defined( QT_NO_OPENSSL )
