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

#include "SummaryView.h"

#include "application.h"
#include "commands/updatebatch.h"
#include "data/datamanager.h"
#include "data/entities.h"
#include "data/updateevent.h"
#include "data/localsettings.h"
#include "models/projectsummarygenerator.h"
#include "utils/textwriter.h"
#include "utils/iconloader.h"
#include "xmlui/builder.h"

#include <QTextBrowser>
#include <QAction>
#include <QMenu>

SummaryView::SummaryView( QObject* parent, QWidget* parentWidget ) : View( parent ),
    m_document( NULL )
{
    QAction* action;

    action = new QAction( IconLoader::icon( "file-reload" ), tr( "&Update Project" ), this );
    action->setShortcut( QKeySequence::Refresh );
    connect( action, SIGNAL( triggered() ), this, SLOT( updateProject() ), Qt::QueuedConnection );
    setAction( "updateProject", action );

    setTitle( "sectionProject", tr( "Project" ) );

    loadXmlUiFile( ":/resources/summaryview.xml" );

    m_browser = new QTextBrowser( parentWidget );
    m_browser->setContextMenuPolicy( Qt::CustomContextMenu );

    QPalette palette = m_browser->palette();
    palette.setBrush( QPalette::Base, QColor( 0xff, 0xff, 0xff ) );
    palette.setBrush( QPalette::Text, QColor( 0x49, 0x49, 0x49 ) );
    palette.setBrush( QPalette::Inactive, QPalette::Highlight, palette.brush( QPalette::Active, QPalette::Highlight ) );
    palette.setBrush( QPalette::Inactive, QPalette::HighlightedText, palette.brush( QPalette::Active, QPalette::HighlightedText ) );
    m_browser->setPalette( palette );

    setMainWidget( m_browser );

    setViewerSizeHint( QSize( 700, 500 ) );

    m_populateTimer = new QTimer( this );
    m_populateTimer->setInterval( 600 );
    m_populateTimer->setSingleShot( true );

    connect( m_populateTimer, SIGNAL( timeout() ), this, SLOT( populateSummary() ) );

    connect( application->applicationSettings(), SIGNAL( settingsChanged() ), this, SLOT( populateSummary() ) );
}

SummaryView::~SummaryView()
{
}

void SummaryView::initialUpdate()
{
    setAccess( checkDataAccess(), true );

    initialUpdateProject();
}

Access SummaryView::checkDataAccess()
{
    ProjectEntity project = ProjectEntity::find( id() );
    if ( !project.isValid() )
        return UnknownAccess;

    if ( dataManager->currentUserAccess() != AdminAccess ) {
        MemberEntity member = MemberEntity::find( id(), dataManager->currentUserId() );
        if ( !member.isValid() )
            return NoAccess;
    }

    if ( ProjectEntity::isAdmin( id() ) )
        return AdminAccess;

    return NormalAccess;
}

void SummaryView::enableView()
{
    populateSummary();

    updateCaption();
    updateActions();
}

void SummaryView::disableView()
{
    m_browser->clear();

    updateCaption();
}

void SummaryView::updateAccess( Access /*access*/ )
{
}

void SummaryView::updateCaption()
{
    QString name = tr( "Unknown Project" ) ;
    if ( isEnabled() ) {
        ProjectEntity project = ProjectEntity::find( id() );
        if ( project.isValid() )
            name = project.name();
    }
    setCaption( name );
}

void SummaryView::updateActions()
{
}

void SummaryView::initialUpdateProject()
{
    if ( isEnabled() && dataManager->summaryUpdateNeeded( id() ) ) {
        UpdateBatch* batch = new UpdateBatch();
        batch->setIfNeeded( true );
        batch->updateSummary( id() );

        executeUpdate( batch );
    }
}

void SummaryView::updateProject()
{
    if ( isEnabled() && !isUpdating() ) {
        UpdateBatch* batch = new UpdateBatch();
        batch->updateSummary( id() );

        executeUpdate( batch );
    }
}

void SummaryView::cascadeUpdateProject()
{
    if ( isEnabled() && !isUpdating() && dataManager->summaryUpdateNeeded( id() ) ) {
        UpdateBatch* batch = new UpdateBatch( -10 );
        batch->setIfNeeded( true );
        batch->updateSummary( id() );

        executeUpdate( batch );
    }
}

void SummaryView::updateEvent( UpdateEvent* e )
{
    setAccess( checkDataAccess() );

    if ( isEnabled() ) {
        if ( e->unit() == UpdateEvent::Summary && e->id() == id() ) {
            updateCaption();
            updateActions();
            populateSummaryDelayed();
        }

        if ( e->unit() == UpdateEvent::Projects ) {
            updateCaption();
            updateActions();
            populateSummaryDelayed();
        }

        if ( e->unit() == UpdateEvent::Users )
            populateSummaryDelayed();
    }

    if ( id() != 0 && e->unit() == UpdateEvent::Projects )
        cascadeUpdateProject();
}

void SummaryView::populateSummaryDelayed()
{
    m_populateTimer->start();
}

void SummaryView::populateSummary()
{
    m_populateTimer->stop();

    if ( !isEnabled() )
        return;

    QApplication::setOverrideCursor( Qt::WaitCursor );

    int pos = m_browser->verticalScrollBar()->sliderPosition();

    ProjectSummaryGenerator generator;
    generator.setProject( id() );

    if ( m_document )
        m_document->clear();

    QTextDocument* document = new QTextDocument( m_browser );

    TextWriter writer( document );
    generator.write( &writer );

    m_browser->setDocument( document );

    delete m_document;
    m_document = document;

    m_browser->verticalScrollBar()->setSliderPosition( pos );

    QApplication::restoreOverrideCursor();
}
