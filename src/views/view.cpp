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

#include "view.h"

#include "commands/commandmanager.h"
#include "commands/abstractbatch.h"
#include "data/datamanager.h"
#include "utils/iconloader.h"
#include "xmlui/builder.h"

#include <QMessageBox>

View::View( QObject* parent ) : QObject( parent ),
    m_mainWidget( NULL ),
    m_id( 0 ),
    m_access( UnknownAccess ),
    m_enabled( false ),
    m_updating( false )
{
    if ( dataManager )
        dataManager->addObserver( this );
}

View::~View()
{
    if ( dataManager )
        dataManager->removeObserver( this );

    delete m_mainWidget;
}

void View::setMainWidget( QWidget* widget )
{
    m_mainWidget = widget;
}

void View::setViewerSizeHint( const QSize& size )
{
    m_sizeHint = size;
}

void View::setAccess( Access access, bool initial )
{
    if ( initial || m_access != access ) {
        m_access = access;

        if ( ( initial || !m_enabled ) && ( access == NormalAccess || access == AdminAccess ) ) {
            m_enabled = true;
            enableView();
            emit enabledChanged( true );
            showInfo( tr( "OK" ) );
        } else if ( ( initial || m_enabled ) && ( access == UnknownAccess || access == NoAccess ) ) {
            m_enabled = false;
            emit enabledChanged( false );
            showSummary( QPixmap(), QString() );
            disableView();
        }

        if ( access == NormalAccess || access == AdminAccess ) {
            updateAccess( access );
            if ( builder() )
                builder()->rebuildAll();
        }

        if ( access == UnknownAccess || access == NoAccess )
            showWarning( tr( "This view is not accessible." ) );
    }
}

void View::enableView()
{
}

void View::disableView()
{
}

void View::updateAccess( Access /*access*/ )
{
}

void View::updateEvent( UpdateEvent* /*e*/ )
{
}

bool View::queryClose()
{
    return true;
}

void View::setCaption( const QString& text )
{
    m_caption = text;
    emit captionChanged( text );
}

void View::customEvent( QEvent* e )
{
    if ( e->type() == UpdateEvent::Type )
        updateEvent( (UpdateEvent*)e );
}

void View::initialUpdate()
{
}

void View::executeUpdate( AbstractBatch* batch )
{
    m_updating = true;

    showBusy( tr( "Updating view..." ) );

    connect( batch, SIGNAL( completed( bool ) ), this, SLOT( updateCompleted( bool ) ) );

    commandManager->execute( batch );
}

void View::updateCompleted( bool successful )
{
    if ( successful ) {
        showInfo( tr( "View was updated successfully." ) );
    } else {
        showError( tr( "View could not be updated: %1." ).arg( commandManager->errorMessage() ) );
        updateFailed();
    }

    m_updating = false;
}

void View::updateFailed()
{
}

void View::showInfo( const QString& text )
{
    m_statusPixmap = IconLoader::pixmap( "status-info" );
    m_statusText = text;
    emit statusChanged( m_statusPixmap, m_statusText );
}

void View::showWarning( const QString& text )
{
    m_statusPixmap = IconLoader::pixmap( "status-warning" );
    m_statusText = text;
    emit statusChanged( m_statusPixmap, m_statusText, QMessageBox::Warning );
}

void View::showError( const QString& text )
{
    m_statusPixmap = IconLoader::pixmap( "status-error" );
    m_statusText = text;
    emit statusChanged( m_statusPixmap, m_statusText, QMessageBox::Critical );
}

void View::showBusy( const QString& text )
{
    m_statusPixmap = IconLoader::pixmap( "status-busy" );
    m_statusText = text;
    emit statusChanged( m_statusPixmap, m_statusText );
}

void View::showSummary( const QPixmap& pixmap, const QString& text )
{
    m_summaryPixmap = pixmap;
    m_summaryText = text;
    emit summaryChanged( m_summaryPixmap, m_summaryText );
}
