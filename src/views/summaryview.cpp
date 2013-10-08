/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2013 WebIssues Team
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

#include "summaryview.h"

#include "application.h"
#include "commands/updatebatch.h"
#include "data/datamanager.h"
#include "data/entities.h"
#include "data/updateevent.h"
#include "data/localsettings.h"
#include "dialogs/dialogmanager.h"
#include "dialogs/finditemdialog.h"
#include "dialogs/projectdialogs.h"
#include "dialogs/reportdialog.h"
#include "models/projectsummarygenerator.h"
#include "utils/htmlwriter.h"
#include "utils/iconloader.h"
#include "widgets/findbar.h"
#include "views/viewmanager.h"
#include "xmlui/builder.h"

#include <QAction>
#include <QMenu>
#include <QFrame>
#include <QLayout>
#include <QTimer>
#include <QKeyEvent>
#include <QClipboard>
#include <QWebView>
#include <QWebFrame>

SummaryView::SummaryView( QObject* parent, QWidget* parentWidget ) : View( parent ),
    m_isFindEnabled( false )
{
    QAction* action;

    action = new QAction( IconLoader::icon( "file-reload" ), tr( "&Update Project" ), this );
    action->setShortcut( QKeySequence::Refresh );
    connect( action, SIGNAL( triggered() ), this, SLOT( updateProject() ), Qt::QueuedConnection );
    setAction( "updateProject", action );

    action = new QAction( IconLoader::icon( "description-new" ), tr( "Add &Description..." ), this );
    connect( action, SIGNAL( triggered() ), this, SLOT( addDescription() ), Qt::QueuedConnection );
    setAction( "addDescription", action );

    action = new QAction( IconLoader::icon( "find" ), tr( "&Find..." ), this );
    action->setShortcut( QKeySequence::Find );
    connect( action, SIGNAL( triggered() ), this, SLOT( find() ) );
    setAction( "find", action );

    action = new QAction( IconLoader::icon( "find-next" ), tr( "Find &Next" ), this );
    action->setShortcut( QKeySequence::FindNext );
    connect( action, SIGNAL( triggered() ), this, SLOT( findNext() ) );
    setAction( "findNext", action );

    action = new QAction( IconLoader::icon( "find-previous" ), tr( "Find &Previous" ), this );
    action->setShortcut( QKeySequence::FindPrevious );
    connect( action, SIGNAL( triggered() ), this, SLOT( findPrevious() ) );
    setAction( "findPrevious", action );

    action = new QAction( IconLoader::icon( "edit-goto" ), tr( "&Go To Item..." ), this );
    connect( action, SIGNAL( triggered() ), this, SLOT( openLink() ), Qt::QueuedConnection );
    setAction( "gotoLinkItem", action );

    action = new QAction( IconLoader::icon( "mail-send" ), tr( "&Send Email" ), this );
    connect( action, SIGNAL( triggered() ), this, SLOT( openLink() ) );
    setAction( "sendEmail", action );

    action = new QAction( IconLoader::icon( "window-new" ), tr( "&Open Link in Browser" ), this );
    connect( action, SIGNAL( triggered() ), this, SLOT( openLink() ) );
    setAction( "openLink", action );

    action = new QAction( IconLoader::icon( "edit-copy" ), tr( "&Copy Email Address" ), this );
    connect( action, SIGNAL( triggered() ), this, SLOT( copyLink() ) );
    setAction( "copyEmail", action );

    action = new QAction( IconLoader::icon( "edit-copy" ), tr( "&Copy Link Address" ), this );
    connect( action, SIGNAL( triggered() ), this, SLOT( copyLink() ) );
    setAction( "copyLink", action );

    action = new QAction( IconLoader::icon( "edit-copy" ), tr( "&Copy" ), this );
    action->setShortcut( QKeySequence::Copy );
    connect( action, SIGNAL( triggered() ), this, SLOT( copy() ) );
    setAction( "editCopy", action );

    action = new QAction( tr( "Select &All" ), this );
    action->setShortcut( QKeySequence::SelectAll );
    connect( action, SIGNAL( triggered() ), this, SLOT( selectAll() ) );
    setAction( "editSelectAll", action );

    action = new QAction( IconLoader::icon( "file-print" ), tr( "Print Summary" ), this );
    action->setIconText( tr( "Print" ) );
    action->setShortcut( QKeySequence::Print );
    connect( action, SIGNAL( triggered() ), this, SLOT( printReport() ), Qt::QueuedConnection );
    setAction( "printReport", action );

    action = new QAction( IconLoader::icon( "export-pdf" ), tr( "Export Summary" ), this );
    action->setIconText( tr( "Export" ) );
    setAction( "popupExport", action );

    action = new QAction( IconLoader::icon( "export-html" ), tr( "Export To HTML" ), this );
    connect( action, SIGNAL( triggered() ), this, SLOT( exportHtml() ), Qt::QueuedConnection );
    setAction( "exportHtml", action );

    action = new QAction( IconLoader::icon( "export-pdf" ), tr( "Export To PDF" ), this );
    connect( action, SIGNAL( triggered() ), this, SLOT( exportPdf() ), Qt::QueuedConnection );
    setAction( "exportPdf", action );

    setTitle( "sectionAdd", tr( "Add" ) );
    setTitle( "sectionProject", tr( "Project" ) );
    setTitle( "sectionEdit", tr( "Edit" ) );
    setTitle( "sectionReport", tr( "Report" ) );

    setPopupMenu( "popupExport", "menuExport", "exportPdf" );

    setDefaultMenuAction( "menuLink", "openLink" );
    setDefaultMenuAction( "menuLinkItem", "gotoLinkItem" );
    setDefaultMenuAction( "menuLinkEmail", "sendEmail" );

    loadXmlUiFile( ":/resources/summaryview.xml" );

    QFrame* main = new QFrame( parentWidget );
    main->setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );

    QVBoxLayout* mainLayout = new QVBoxLayout( main );
    mainLayout->setMargin( 0 );
    mainLayout->setSpacing( 0 );

    m_browser = new QWebView( main );
    m_browser->setContextMenuPolicy( Qt::CustomContextMenu );
    m_browser->page()->setLinkDelegationPolicy( QWebPage::DelegateAllLinks );

    QPalette palette = m_browser->palette();
    palette.setBrush( QPalette::Inactive, QPalette::Highlight, palette.brush( QPalette::Active, QPalette::Highlight ) );
    palette.setBrush( QPalette::Inactive, QPalette::HighlightedText, palette.brush( QPalette::Active, QPalette::HighlightedText ) );
    m_browser->setPalette( palette );

    mainLayout->addWidget( m_browser, 1 );

    connect( m_browser, SIGNAL( customContextMenuRequested( const QPoint& ) ),
        this, SLOT( summaryContextMenu( const QPoint& ) ) );
    connect( m_browser, SIGNAL( linkClicked( const QUrl& ) ), this, SLOT( linkClicked( const QUrl& ) ) );
    connect( m_browser, SIGNAL( selectionChanged() ), this, SLOT( updateActions() ) );

    m_findBar = new FindBar( main );
    m_findBar->setBoundWidget( m_browser );
    m_findBar->setAutoFillBackground( true );
    m_findBar->hide();

    mainLayout->addWidget( m_findBar );

    connect( m_findBar, SIGNAL( find( const QString& ) ), this, SLOT( findText( const QString& ) ) );
    connect( m_findBar, SIGNAL( findPrevious() ), this, SLOT( findPrevious() ) );
    connect( m_findBar, SIGNAL( findNext() ), this, SLOT( findNext() ) );
    connect( m_findBar, SIGNAL( findEnabled( bool ) ), this, SLOT( updateActions() ) );

    setMainWidget( main );

    main->installEventFilter( this );

    setViewerSizeHint( QSize( 700, 500 ) );

    m_populateTimer = new QTimer( this );
    m_populateTimer->setInterval( 600 );
    m_populateTimer->setSingleShot( true );

    connect( m_populateTimer, SIGNAL( timeout() ), this, SLOT( populateSummary() ) );
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
        MemberEntity member = MemberEntity::findEffective( id(), dataManager->currentUserId() );
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
    m_browser->setHtml( QString() );
    m_findBar->hide();

    updateCaption();
}

void SummaryView::updateAccess( Access access )
{
    action( "addDescription" )->setVisible( access == AdminAccess );
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
    m_isFindEnabled = m_findBar->isFindEnabled();

    // NOTE: hasSelection() is currently broken and always returns true, so using this workaround
    bool hasSelection = m_browser->pageAction( QWebPage::Copy )->isEnabled();

    bool linkNotEmpty = !m_actionLink.isEmpty();

    ProjectEntity project = ProjectEntity::find( id() );

    action( "addDescription" )->setEnabled( !project.description().isValid() );
    action( "editCopy" )->setEnabled( hasSelection );
    action( "findNext" )->setEnabled( m_isFindEnabled );
    action( "findPrevious" )->setEnabled( m_isFindEnabled );
    action( "gotoLinkItem" )->setEnabled( linkNotEmpty );
    action( "sendEmail" )->setEnabled( linkNotEmpty );
    action( "openLink" )->setEnabled( linkNotEmpty );
    action( "copyEmail" )->setEnabled( linkNotEmpty );
    action( "copyLink" )->setEnabled( linkNotEmpty );
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

void SummaryView::addDescription()
{
    if ( isEnabled() ) {
        if ( dialogManager->activateDialog( "AddProjectDescriptionDialog", id() ) )
            return;
        AddProjectDescriptionDialog* dialog = new AddProjectDescriptionDialog( id() );
        dialogManager->addDialog( dialog, id() );
        dialog->show();
    }
}

void SummaryView::copy()
{
    if ( isEnabled() )
        m_browser->triggerPageAction( QWebPage::Copy );
}

void SummaryView::selectAll()
{
    if ( isEnabled() )
        m_browser->triggerPageAction( QWebPage::SelectAll );
}

void SummaryView::find()
{
    if ( isEnabled() ) {
        m_findBar->show();
        m_findBar->setFocus();
        m_findBar->selectAll();
    }
}

void SummaryView::findText( const QString& text )
{
    if ( isEnabled() )
        findText( text, m_findBar->isCaseSensitive() ? QWebPage::FindCaseSensitively : 0 );
}

void SummaryView::findNext()
{
    if ( isEnabled() && m_isFindEnabled ) {
        findText( m_findBar->text(), m_findBar->isCaseSensitive() ? QWebPage::FindCaseSensitively : 0 );
        m_findBar->selectAll();
    }
}

void SummaryView::findPrevious()
{
    if ( isEnabled() && m_isFindEnabled ) {
        findText( m_findBar->text(), ( m_findBar->isCaseSensitive() ? QWebPage::FindCaseSensitively : 0 ) | QTextDocument::FindBackward );
        m_findBar->selectAll();
    }
}

void SummaryView::findText( const QString& text, int flags )
{
    bool warn = false;

    if ( !text.isEmpty() )
        warn = !m_browser->findText( text, (QWebPage::FindFlags)flags | QWebPage::FindWrapsAroundDocument );

    m_findBar->show();
    m_findBar->setFocus();
    m_findBar->showWarning( warn );
}

bool SummaryView::eventFilter( QObject* obj, QEvent* e )
{
    if ( obj == mainWidget() ) {
        if ( e->type() == QEvent::ShortcutOverride ) {
            QKeyEvent* ke = (QKeyEvent*)e;
            if ( ke->key() == Qt::Key_F3 && ( ke->modifiers() & ~Qt::ShiftModifier ) == 0 ) {
                if ( isEnabled() && !m_isFindEnabled ) {
                    find();
                    ke->accept();
                    return true;
                }
            }
        }
    }
    return View::eventFilter( obj, e );
}

void SummaryView::openLink()
{
    if ( isEnabled() && !m_actionLink.isEmpty() )
        linkClicked( m_actionLink );
}

void SummaryView::copyLink()
{
    if ( isEnabled() && !m_actionLink.isEmpty() ) {
        QString link = m_actionLink.toString();

        if ( link.startsWith( "mailto:", Qt::CaseInsensitive ) )
            link = link.mid( 7 );

        QApplication::clipboard()->setText( link, QClipboard::Clipboard );
    }
}

void SummaryView::printReport()
{
    ReportDialog dialog( ReportDialog::ProjectSource, ReportDialog::Print, mainWidget() );
    dialog.setProject( id() );
    dialog.exec();
}

void SummaryView::exportHtml()
{
    ReportDialog dialog( ReportDialog::ProjectSource, ReportDialog::ExportHtml, mainWidget() );
    dialog.setProject( id() );
    dialog.exec();
}

void SummaryView::exportPdf()
{
    ReportDialog dialog( ReportDialog::ProjectSource, ReportDialog::ExportPdf, mainWidget() );
    dialog.setProject( id() );
    dialog.exec();
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

    QPoint pos = m_browser->page()->mainFrame()->scrollPosition();

    ProjectSummaryGenerator generator;
    generator.setProject( id() );

    HtmlWriter writer;
    generator.write( &writer );

    m_browser->setHtml( writer.toHtml() );

    m_browser->page()->mainFrame()->setScrollPosition( pos );

    QApplication::restoreOverrideCursor();
}

bool SummaryView::linkContextMenu( const QUrl& link, const QPoint& pos )
{
    m_actionLink = link;
    updateActions();

    QString menuName;
    QString scheme = link.scheme().toLower();

    if ( scheme == QLatin1String( "id" ) )
        menuName = "menuLinkItem";
    else if ( scheme == QLatin1String( "mailto" ) )
        menuName = "menuLinkEmail";
    else if ( scheme != QLatin1String( "command" ) )
        menuName = "menuLink";

    if ( !menuName.isEmpty() ) {
        QMenu* menu = builder()->contextMenu( menuName );
        if ( menu ) {
            menu->exec( pos );
            return true;
        }
    }

    return false;
}

void SummaryView::summaryContextMenu( const QPoint& pos )
{
    QWebHitTestResult result = m_browser->page()->mainFrame()->hitTestContent( pos );
    QUrl link = result.linkUrl();

    if ( !link.isEmpty() ) {
        if ( linkContextMenu( link, m_browser->mapToGlobal( pos ) ) )
            return;
    }

    QString menuName;
    if ( m_browser->pageAction( QWebPage::Copy )->isEnabled() )
        menuName = "menuSelection";
    else
        menuName = "menuSummary";

    QMenu* menu = builder()->contextMenu( menuName );
    if ( menu )
        menu->popup( m_browser->mapToGlobal( pos ) );
}

void SummaryView::linkClicked( const QUrl& url )
{
    QString scheme = url.scheme().toLower();

    if ( scheme == QLatin1String( "id" ) ) {
        int itemId = url.host().toInt();
        findItem( itemId );
    } else if ( scheme == QLatin1String( "command" ) ) {
        handleCommand( url.host() );
    } else {
        Application::openUrl( mainWidget(), url );
    }
}

void SummaryView::handleCommand( const QString& command )
{
    if ( command == QLatin1String( "edit-description" ) ) {
        if ( dialogManager->activateDialog( "EditProjectDescriptionDialog", id() ) )
            return;
        EditProjectDescriptionDialog* dialog = new EditProjectDescriptionDialog( id() );
        dialogManager->addDialog( dialog, id() );
        dialog->show();
    } else if ( command == QLatin1String( "delete-description" ) ) {
        DeleteProjectDescriptionDialog dialog( id(), mainWidget() );
        dialog.exec();
    }
}

void SummaryView::findItem( int itemId )
{
    int issueId = FindItemDialog::getFindItem( mainWidget(), itemId );

    if ( issueId == 0 )
        return;

    if ( viewManager->isStandAlone( this ) )
        viewManager->openIssueView( issueId, itemId );
    else
        emit issueActivated( issueId, itemId );
}
