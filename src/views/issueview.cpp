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

#include "issueview.h"

#include "application.h"
#include "commands/updatebatch.h"
#include "commands/statebatch.h"
#include "data/datamanager.h"
#include "data/entities.h"
#include "data/updateevent.h"
#include "data/localsettings.h"
#include "dialogs/issuedialogs.h"
#include "dialogs/finditemdialog.h"
#include "dialogs/checkmessagebox.h"
#include "dialogs/reportdialog.h"
#include "dialogs/statedialogs.h"
#include "dialogs/messagebox.h"
#include "models/issuedetailsgenerator.h"
#include "utils/datetimehelper.h"
#include "utils/treeviewhelper.h"
#include "utils/textwriter.h"
#include "utils/formatter.h"
#include "utils/iconloader.h"
#include "views/viewmanager.h"
#include "widgets/findbar.h"
#include "xmlui/builder.h"

#include <QLayout>
#include <QTextBrowser>
#include <QAction>
#include <QMenu>
#include <QFileDialog>
#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QSettings>
#include <QKeyEvent>
#include <QScrollBar>
#include <QPushButton>
#include <QTimer>

#if defined( Q_WS_WIN )
#include <qt_windows.h>
#undef MessageBox
#endif

IssueView::IssueView( QObject* parent, QWidget* parentWidget ) : View( parent ),
    m_document( NULL ),
    m_gotoItemId( 0 ),
    m_folderId( 0 ),
    m_typeId( 0 ),
    m_isRead( false ),
    m_history( IssueDetailsGenerator::AllHistory ),
    m_lockedIssueId( 0 )
{
    QAction* action;

    action = new QAction( IconLoader::icon( "file-reload" ), tr( "&Update Issue" ), this );
    action->setShortcut( QKeySequence::Refresh );
    connect( action, SIGNAL( triggered() ), this, SLOT( updateIssue() ), Qt::QueuedConnection );
    setAction( "updateIssue", action );

    action = new QAction( IconLoader::icon( "comment" ), tr( "Add &Comment..." ), this );
    action->setShortcut( QKeySequence::New );
    connect( action, SIGNAL( triggered() ), this, SLOT( addComment() ), Qt::QueuedConnection );
    setAction( "addComment", action );

    action = new QAction( IconLoader::icon( "file-attach" ), tr( "Add &Attachment..." ), this );
    connect( action, SIGNAL( triggered() ), this, SLOT( addAttachment() ), Qt::QueuedConnection );
    setAction( "addAttachment", action );

    action = new QAction( IconLoader::icon( "edit-modify" ), tr( "&Edit Attributes..." ), this );
    action->setShortcut( tr( "F2" ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( editIssue() ), Qt::QueuedConnection );
    setAction( "editIssue", action );

    action = new QAction( IconLoader::icon( "issue-clone" ), tr( "Clone Issue..." ), this );
    connect( action, SIGNAL( triggered() ), this, SLOT( cloneIssue() ), Qt::QueuedConnection );
    setAction( "cloneIssue", action );

    action = new QAction( IconLoader::icon( "issue-move" ), tr( "&Move Issue..." ), this );
    action->setIconText( tr( "Move" ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( moveIssue() ), Qt::QueuedConnection );
    setAction( "moveIssue", action );

    action = new QAction( IconLoader::icon( "edit-delete" ), tr( "&Delete Issue" ), this );
    action->setIconText( tr( "Delete" ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( deleteIssue() ), Qt::QueuedConnection );
    setAction( "deleteIssue", action );

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

    action = new QAction( IconLoader::icon( "file-open" ), tr( "&Open Attachment" ), this );
    connect( action, SIGNAL( triggered() ), this, SLOT( openAttachment() ), Qt::QueuedConnection );
    setAction( "openAttachment", action );

    action = new QAction( IconLoader::icon( "file-save-as" ), tr( "&Save Attachment As..." ), this );
    connect( action, SIGNAL( triggered() ), this, SLOT( saveAttachment() ), Qt::QueuedConnection );
    setAction( "saveAttachment", action );

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

    action = new QAction( IconLoader::icon( "file-print" ), tr( "Print Issue" ), this );
    action->setIconText( tr( "Print" ) );
    action->setShortcut( QKeySequence::Print );
    connect( action, SIGNAL( triggered() ), this, SLOT( printReport() ), Qt::QueuedConnection );
    setAction( "printReport", action );

    action = new QAction( IconLoader::icon( "export-pdf" ), tr( "Export Issue" ), this );
    action->setIconText( tr( "Export" ) );
    setAction( "popupExport", action );

    action = new QAction( IconLoader::icon( "export-html" ), tr( "Export To HTML" ), this );
    connect( action, SIGNAL( triggered() ), this, SLOT( exportHtml() ), Qt::QueuedConnection );
    setAction( "exportHtml", action );

    action = new QAction( IconLoader::icon( "export-pdf" ), tr( "Export To PDF" ), this );
    connect( action, SIGNAL( triggered() ), this, SLOT( exportPdf() ), Qt::QueuedConnection );
    setAction( "exportPdf", action );

    action = new QAction( IconLoader::icon( "issue" ), tr( "Mark As Read" ), this );
    connect( action, SIGNAL( triggered() ), this, SLOT( markAsRead() ), Qt::QueuedConnection );
    setAction( "markAsRead", action );

    setTitle( "sectionAdd", tr( "Add" ) );
    setTitle( "sectionIssue", tr( "Issue" ) );
    setTitle( "sectionEdit", tr( "Edit" ) );
    setTitle( "sectionReport", tr( "Report" ) );

    setPopupMenu( "popupExport", "menuExport", "exportPdf" );

    setDefaultMenuAction( "menuLink", "openLink" );
    setDefaultMenuAction( "menuLinkItem", "gotoLinkItem" );
    setDefaultMenuAction( "menuLinkEmail", "sendEmail" );

    loadXmlUiFile( ":/resources/issueview.xml" );

    QWidget* main = new QWidget( parentWidget );
    QVBoxLayout* mainLayout = new QVBoxLayout( main );
    mainLayout->setMargin( 0 );
    mainLayout->setSpacing( 0 );

    m_browser = new QTextBrowser( main );
    m_browser->setContextMenuPolicy( Qt::CustomContextMenu );

    QPalette palette = m_browser->palette();
    palette.setBrush( QPalette::Base, QColor( 0xff, 0xff, 0xff ) );
    palette.setBrush( QPalette::Text, QColor( 0x49, 0x49, 0x49 ) );
    palette.setBrush( QPalette::Inactive, QPalette::Highlight, palette.brush( QPalette::Active, QPalette::Highlight ) );
    palette.setBrush( QPalette::Inactive, QPalette::HighlightedText, palette.brush( QPalette::Active, QPalette::HighlightedText ) );
    m_browser->setPalette( palette );

    mainLayout->addWidget( m_browser );

    m_browser->addAction( this->action( "findNext" ) );
    m_browser->addAction( this->action( "findPrevious" ) );

    connect( m_browser, SIGNAL( customContextMenuRequested( const QPoint& ) ),
        this, SLOT( historyContextMenu( const QPoint& ) ) );
    connect( m_browser, SIGNAL( anchorClicked( const QUrl& ) ), this, SLOT( anchorClicked( const QUrl& ) ) );
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

    connect( m_populateTimer, SIGNAL( timeout() ), this, SLOT( populateDetails() ) );

    connect( application->applicationSettings(), SIGNAL( settingsChanged() ), this, SLOT( populateDetails() ) );
}

IssueView::~IssueView()
{
    cleanUp();
}

void IssueView::cleanUp()
{
    if ( dataManager && m_lockedIssueId != 0 ) {
        dataManager->unlockIssue( m_lockedIssueId );
        m_lockedIssueId = 0;
    }
}

void IssueView::initialUpdate()
{
    cleanUp();

    m_history = (IssueDetailsGenerator::History)dataManager->setting( "history_filter" ).toInt();

    setAccess( checkDataAccess(), true );

    if ( id() != 0 ) {
        dataManager->lockIssue( id() );
        m_lockedIssueId = id();
    }

    initialUpdateIssue();
}

Access IssueView::checkDataAccess()
{
    IssueEntity issue = IssueEntity::find( id() );
    if ( !issue.isValid() )
        return UnknownAccess;

    m_folderId = issue.folderId();

    FolderEntity folder = issue.folder();
    if ( !folder.isValid() )
        return NoAccess;

    if ( dataManager->currentUserAccess() != AdminAccess ) {
        MemberEntity member = MemberEntity::find( folder.projectId(), dataManager->currentUserId() );
        if ( !member.isValid() )
            return NoAccess;
    }

    TypeEntity type = folder.type();
    if ( !type.isValid() )
        return UnknownAccess;

    m_typeId = type.id();

    if ( IssueEntity::isAdmin( id() ) )
        return AdminAccess;

    return NormalAccess;
}

void IssueView::enableView()
{
    populateDetails();

    updateCaption();
    updateActions();
}

void IssueView::disableView()
{
    m_browser->clear();
    m_findBar->hide();

    updateCaption();
}

void IssueView::updateAccess( Access access )
{
    action( "moveIssue" )->setVisible( access == AdminAccess );
    action( "deleteIssue" )->setVisible( access == AdminAccess );
}

void IssueView::gotoItem( int itemId )
{
    if ( itemId == id() ) {
        m_browser->verticalScrollBar()->setSliderPosition( 0 );
        return;
    }

    if ( isUpdating() )
        m_gotoItemId = itemId;

    if ( IssueEntity::findItem( itemId ) == id() )
        m_browser->scrollToAnchor( QString( "id%1" ).arg( itemId ) );
}

void IssueView::updateCaption()
{
    QString name = tr( "Unknown Issue" ) ;
    if ( isEnabled() ) {
        IssueEntity issue = IssueEntity::find( id() );
        if ( issue.isValid() )
            name = issue.name();
    }
    setCaption( name );
}

void IssueView::updateActions()
{
    m_isFindEnabled = m_findBar->isFindEnabled();

    bool hasSelection = m_browser->textCursor().hasSelection();

    bool linkNotEmpty = !m_actionLink.isEmpty();

    IssueEntity issue = IssueEntity::find( id() );
    m_isRead = issue.isValid() ? issue.readId() >= issue.stampId() : false;

    action( "editCopy" )->setEnabled( hasSelection );
    action( "findNext" )->setEnabled( m_isFindEnabled );
    action( "findPrevious" )->setEnabled( m_isFindEnabled );
    action( "openAttachment" )->setEnabled( linkNotEmpty );
    action( "saveAttachment" )->setEnabled( linkNotEmpty );
    action( "gotoLinkItem" )->setEnabled( linkNotEmpty );
    action( "sendEmail" )->setEnabled( linkNotEmpty );
    action( "openLink" )->setEnabled( linkNotEmpty );
    action( "copyEmail" )->setEnabled( linkNotEmpty );
    action( "copyLink" )->setEnabled( linkNotEmpty );

    action( "markAsRead" )->setText( m_isRead ? tr( "Mark As Unread" ) : tr( "Mark As Read" ) );
    action( "markAsRead" )->setIcon( IconLoader::icon( m_isRead ? "issue-unread" : "issue" ) );
}

void IssueView::initialUpdateIssue()
{
    if ( isEnabled() ) {
        if ( dataManager->issueUpdateNeeded( id() ) ) {
            UpdateBatch* batch = new UpdateBatch();
            batch->setIfNeeded( true );
            batch->updateIssue( id(), true );

            executeUpdate( batch );
        } else {
            IssueEntity issue = IssueEntity::find( id() );

            if ( issue.stampId() > issue.readId() ) {
                StateBatch* batch = new StateBatch();
                batch->setIssueRead( id(), issue.stampId() );

                executeUpdate( batch );
            }
        }
    }
}

void IssueView::updateIssue()
{
    if ( isEnabled() && !isUpdating() ) {
        UpdateBatch* batch = new UpdateBatch();
        batch->updateIssue( id(), true );

        executeUpdate( batch );
    }
}

void IssueView::cascadeUpdateIssue()
{
    if ( isEnabled() && !isUpdating() && dataManager->issueUpdateNeeded( id() ) ) {
        UpdateBatch* batch = new UpdateBatch( -10 );
        batch->setIfNeeded( true );
        batch->updateIssue( id(), false );

        executeUpdate( batch );
    }
}

void IssueView::addComment()
{
    if ( isEnabled() )
        viewManager->openCommentView( id() );
}

void IssueView::addAttachment()
{
    if ( isEnabled() ) {
        LocalSettings* settings = application->applicationSettings();
        QString dir = settings->value( "AddAttachmentPath", QDir::homePath() ).toString();

        QString path = QFileDialog::getOpenFileName( mainWidget(), tr( "Add Attachment" ), dir );

        if ( !path.isEmpty() ) {
            QFileInfo fileInfo( path );

            int size = fileInfo.size();
            int maxSize = dataManager->setting( "file_max_size" ).toInt();

            if ( size > maxSize ) {
                Formatter formatter;
                MessageBox::warning( mainWidget(), tr( "Warning" ),
                    tr( "The selected file is bigger than the maximum allowed file size\non this server which is %1." ).arg( formatter.formatSize( maxSize ) ) );
                return;
            }

            settings->setValue( "AddAttachmentPath", fileInfo.absoluteDir().path() );

            AddAttachmentDialog dialog( id(), path, path, mainWidget() );
            dialog.exec();
        }
    }
}

void IssueView::editIssue()
{
    if ( isEnabled() ) {
        EditIssueDialog dialog( id(), mainWidget() );
        dialog.exec();
    }
}

void IssueView::cloneIssue()
{
    if ( isEnabled() ) {
        CloneIssueDialog cloneDialog( id(), mainWidget() );
        if ( cloneDialog.exec() == QDialog::Accepted ) {
            int folderId = cloneDialog.folderId();

            AddIssueDialog addDialog( folderId, id(), mainWidget() );
            if ( addDialog.exec() == QDialog::Accepted ) {
                int issueId = addDialog.issueId();

                if ( viewManager->isStandAlone( this ) )
                    viewManager->openIssueView( issueId );
                else
                    emit issueActivated( issueId, issueId );
            }
        }
    }
}

void IssueView::moveIssue()
{
    if ( isEnabled() && IssueEntity::isAdmin( id() ) ) {
        MoveIssueDialog dialog( id(), mainWidget() );
        dialog.exec();
    }
}

void IssueView::deleteIssue()
{
    if ( isEnabled() && IssueEntity::isAdmin( id() ) ) {
        DeleteIssueDialog dialog( id(), mainWidget() );

        if ( dialog.exec() == QDialog::Accepted && viewManager->isStandAlone( this ) )
            viewManager->closeView( this );
    }
}

void IssueView::markAsRead()
{
    if ( isEnabled() ) {
        int readId;
        if ( m_isRead ) {
            readId = 0;
        } else {
            IssueEntity issue = IssueEntity::find( id() );
            readId = issue.stampId();
        }

        IssueStateDialog dialog( id(), readId, mainWidget() );
        dialog.accept();
        dialog.exec();
    }
}

void IssueView::copy()
{
    if ( isEnabled() )
        m_browser->copy();
}

void IssueView::selectAll()
{
    if ( isEnabled() )
        m_browser->selectAll();
}

void IssueView::find()
{
    if ( isEnabled() ) {
        m_findBar->show();
        m_findBar->setFocus();
        m_findBar->selectAll();
    }
}

void IssueView::findText( const QString& text )
{
    if ( isEnabled() )
        findText( text, m_browser->textCursor().selectionStart(), m_findBar->flags() );
}

void IssueView::findNext()
{
    if ( isEnabled() && m_isFindEnabled ) {
        findText( m_findBar->text(), m_browser->textCursor().selectionStart() + 1, m_findBar->flags() );
        m_findBar->selectAll();
    }
}

void IssueView::findPrevious()
{
    if ( isEnabled() && m_isFindEnabled ) {
        findText( m_findBar->text(), m_browser->textCursor().selectionStart(), m_findBar->flags() | QTextDocument::FindBackward );
        m_findBar->selectAll();
    }
}

void IssueView::findText( const QString& text, int from, QTextDocument::FindFlags flags )
{
    QTextCursor found;
    bool warn = false;

    if ( !text.isEmpty() ) {
        found = m_browser->document()->find( text, from, flags );

        if ( found.isNull() ) {
            if ( flags & QTextDocument::FindBackward ) {
                QTextCursor end( m_browser->document() );
                end.movePosition( QTextCursor::End );
                from = end.position();
            } else {
                from = 0;
            }

            found = m_browser->document()->find( text, from, flags );

            if ( found.isNull() )
                warn = true;
        }
    }

    if ( found.isNull() ) {
        found = m_browser->textCursor();
        found.setPosition( found.selectionStart() );
    }

    m_findBar->show();
    m_findBar->setFocus();

    m_browser->setTextCursor( found );

    m_findBar->showWarning( warn );
}

bool IssueView::eventFilter( QObject* obj, QEvent* e )
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

void IssueView::openAttachment()
{
    if ( isEnabled() && !m_actionLink.isEmpty() )
        handleAttachment( QUrl( m_actionLink ).host().toInt(), ActionOpen );
}

void IssueView::saveAttachment()
{
    if ( isEnabled() && !m_actionLink.isEmpty() )
        handleAttachment( QUrl( m_actionLink ).host().toInt(), ActionSaveAs );
}

void IssueView::openLink()
{
    if ( isEnabled() && !m_actionLink.isEmpty() )
        anchorClicked( QUrl( m_actionLink ) );
}

void IssueView::copyLink()
{
    if ( isEnabled() && !m_actionLink.isEmpty() ) {
        QString url = m_actionLink;

        if ( url.startsWith( "mailto:" ) )
            url = url.mid( 7 );

        QApplication::clipboard()->setText( url, QClipboard::Clipboard );
    }
}

void IssueView::printReport()
{
    ReportDialog dialog( ReportDialog::IssueSource, ReportDialog::Print, mainWidget() );
    dialog.setIssue( id() );
    dialog.setHistory( m_history );
    dialog.exec();
}

void IssueView::exportHtml()
{
    ReportDialog dialog( ReportDialog::IssueSource, ReportDialog::ExportHtml, mainWidget() );
    dialog.setIssue( id() );
    dialog.setHistory( m_history );
    dialog.exec();
}

void IssueView::exportPdf()
{
    ReportDialog dialog( ReportDialog::IssueSource, ReportDialog::ExportPdf, mainWidget() );
    dialog.setIssue( id() );
    dialog.setHistory( m_history );
    dialog.exec();
}

void IssueView::updateEvent( UpdateEvent* e )
{
    setAccess( checkDataAccess() );

    if ( isEnabled() ) {
        if ( e->unit() == UpdateEvent::Issue && e->id() == id() ) {
            updateCaption();
            updateActions();
            populateDetails();
        }

        if ( e->unit() == UpdateEvent::Users )
            populateDetailsDelayed();

        if ( e->unit() == UpdateEvent::Types )
            populateDetailsDelayed();

        if ( e->unit() == UpdateEvent::Projects )
            populateDetailsDelayed();

        if ( e->unit() == UpdateEvent::Folder && e->id() == m_folderId ) {
            updateCaption();
            updateActions();
            populateDetails();
        }

        if ( e->unit() == UpdateEvent::States )
            updateActions();
    }

    if ( isEnabled() && m_gotoItemId != 0 && e->unit() == UpdateEvent::Issue && e->id() == id() ) {
        gotoItem( m_gotoItemId );
        m_gotoItemId = 0;
    }

    if ( id() != 0 && m_folderId != 0 && e->unit() == UpdateEvent::Folder && e->id() == m_folderId )
        cascadeUpdateIssue();
}

void IssueView::populateDetailsDelayed()
{
    m_populateTimer->start();
}

void IssueView::populateDetails()
{
    m_populateTimer->stop();

    if ( !isEnabled() )
        return;

    QApplication::setOverrideCursor( Qt::WaitCursor );

    int pos = m_browser->verticalScrollBar()->sliderPosition();

    IssueDetailsGenerator generator;
    generator.setIssue( id(), m_history );

    if ( m_document )
        m_document->clear();

    QTextDocument* document = new QTextDocument( m_browser );

    TextWriter writer( document );
    generator.write( &writer );

    m_browser->setDocument( document );

    delete m_document;
    m_document = document;

    m_browser->verticalScrollBar()->setSliderPosition( pos );

    QStringList status;
    if ( m_history != IssueDetailsGenerator::OnlyFiles )
        status.append( tr( "%1 comments" ).arg( generator.commentsCount() ) );
    if ( m_history != IssueDetailsGenerator::OnlyComments )
        status.append( tr( "%1 attachments" ).arg( generator.filesCount() ) );
    showSummary( QPixmap(), status.join( ", " ) );

    QApplication::restoreOverrideCursor();
}

void IssueView::linkContextMenu( const QString& link, const QPoint& pos )
{
    m_actionLink = link;
    updateActions();

    QUrl url( link );

    QString menuName;
    if ( url.scheme() == QLatin1String( "id" ) )
        menuName = "menuLinkItem";
    else if ( url.scheme() == QLatin1String( "attachment" ) )
        menuName = "menuLinkAttachment";
    else if ( url.scheme() == QLatin1String( "mailto" ) )
        menuName = "menuLinkEmail";
    else if ( url.scheme() == QLatin1String( "command" ) )
        menuName = "menuHistory";
    else
        menuName = "menuLink";

    QMenu* menu = builder()->contextMenu( menuName );
    if ( menu )
        menu->exec( pos );
}

void IssueView::historyContextMenu( const QPoint& pos )
{
    QString link = m_browser->anchorAt( pos );
    if ( !link.isEmpty() ) {
        linkContextMenu( link, m_browser->mapToGlobal( pos ) );
        return;
    }

    QString menuName;
    if ( m_browser->textCursor().hasSelection() )
        menuName = "menuSelection";
    else
        menuName = "menuHistory";

    QMenu* menu = builder()->contextMenu( menuName );
    if ( menu )
        menu->popup( m_browser->mapToGlobal( pos ) );
}

void IssueView::anchorClicked( const QUrl& url )
{
    int pos = m_browser->verticalScrollBar()->sliderPosition();
    m_browser->setSource( QUrl() );
    m_browser->verticalScrollBar()->setSliderPosition( pos );

    if ( url.scheme() == QLatin1String( "id" ) ) {
        int itemId = url.host().toInt();
        findItem( itemId );
    } else if ( url.scheme() == QLatin1String( "attachment" ) ) {
        int attachmentId = url.host().toInt();
        handleAttachment( attachmentId );
    } else if ( url.scheme() == QLatin1String( "command" ) ) {
        int argument = url.path().mid( 1 ).toInt();
        handleCommand( url.host(), argument );
    } else {
#if defined( Q_WS_WIN )
        if ( url.scheme() == QLatin1String( "file" ) ) {
            if ( url.isValid() ) {
                QString path = url.path();
                if ( path.startsWith( QLatin1Char( '/' ) ) )
                    path = path.mid( 1 );
                path = QDir::toNativeSeparators( path );
                QString host = url.host();
                if ( !host.isEmpty() )
                    path = QLatin1String( "\\\\" ) + host + QLatin1String( "\\" ) + path;
                if ( !path.isEmpty() )
                    ShellExecute( mainWidget()->effectiveWinId(), NULL, (LPCTSTR)path.utf16(), NULL, NULL, SW_NORMAL );
            }
        } else
#endif
        QDesktopServices::openUrl( url );
    }
}

void IssueView::handleCommand( const QString& command, int argument )
{
    if ( command == QLatin1String( "filter" ) ) {
        m_history = (IssueDetailsGenerator::History)argument;
        populateDetails();
    } else if ( command == QLatin1String( "edit-comment" ) ) {
        viewManager->openCommentView( argument );
    } else if ( command == QLatin1String( "edit-file" ) ) {
        EditAttachmentDialog dialog( argument, mainWidget() );
        dialog.exec();
    } else if ( command == QLatin1String( "delete-comment" ) ) {
        DeleteCommentDialog dialog( argument, mainWidget() );
        dialog.exec();
    } else if ( command == QLatin1String( "delete-file" ) ) {
        DeleteAttachmentDialog dialog( argument, mainWidget() );
        dialog.exec();
    }
}

void IssueView::findItem( int itemId )
{
    int issueId = IssueEntity::findItem( itemId );

    if ( issueId == 0 ) {
        FindItemDialog dialog( mainWidget() );
        dialog.findItem( itemId );
        if ( dialog.exec() == QDialog::Accepted )
            issueId = dialog.issueId();
        else
            return;
    }

    if ( issueId == id() )
        gotoItem( itemId );
    else if ( viewManager->isStandAlone( this ) )
        viewManager->openIssueView( issueId, itemId );
    else
        emit issueActivated( issueId, itemId );
}

void IssueView::handleAttachment( int fileId )
{
    LocalSettings* settings = application->applicationSettings();
    handleAttachment( fileId, (AttachmentAction)settings->value( "DefaultAttachmentAction" ).toInt() );
}

void IssueView::handleAttachment( int fileId, AttachmentAction action )
{
    ChangeEntity change = ChangeEntity::findFile( fileId );
    FileEntity file = change.file();

    if ( action == ActionAsk ) {
        CheckMessageBox box( mainWidget() );

        box.setWindowTitle( tr( "Attachment" ) );
        box.setPrompt( tr( "Do you want to save or open attachment <b>%1</b>?" ).arg( file.name() ) );
        box.setPromptPixmap( IconLoader::pixmap( "status-question", 22 ) );
        box.setCheckBoxText( tr( "Do this automatically for all attachments" ) );
        box.setStandardButtons( QMessageBox::Save | QMessageBox::Open | QMessageBox::Cancel );

        box.button( QMessageBox::Save )->setText( tr( "&Save As..." ) );
        box.button( QMessageBox::Save )->setIcon( IconLoader::icon( "file-save-as" ) );
        box.button( QMessageBox::Open )->setText( tr( "&Open" ) );
        box.button( QMessageBox::Open )->setIcon( IconLoader::icon( "file-open" ) );
        box.button( QMessageBox::Cancel )->setMinimumHeight( box.button( QMessageBox::Save )->sizeHint().height() );

        int result = box.exec();

        if ( result == QMessageBox::Save )
            action = ActionSaveAs;
        else if ( result == QMessageBox::Open )
            action = ActionOpen;
        else
            return;

        if ( box.isChecked() ) {
            LocalSettings* settings = application->applicationSettings();
            settings->setValue( "DefaultAttachmentAction", (int)action );
        }
    }

    QString path;

    if ( action == ActionSaveAs ) {
        LocalSettings* settings = application->applicationSettings();
        QString dir = settings->value( "SaveAttachmentPath", QDir::homePath() ).toString();

        QFileInfo fileInfo( QDir( dir ), file.name() );

        path = QFileDialog::getSaveFileName( mainWidget(), tr( "Save Attachment" ), fileInfo.absoluteFilePath() );
        if ( path.isEmpty() )
            return;

        fileInfo.setFile( path );
        settings->setValue( "SaveAttachmentPath", fileInfo.absoluteDir().path() );
    }

    QString cachePath = dataManager->findFilePath( fileId );

    if ( cachePath.isEmpty() ) {
        cachePath = dataManager->generateFilePath( file.name() );
        dataManager->allocFileSpace( file.size() );

        GetAttachmentDialog dialog( fileId, cachePath, path, mainWidget() );
        dialog.download();

        if ( dialog.exec() != QDialog::Accepted ) {
            QFile::remove( cachePath );
            return;
        }

        dataManager->commitFile( fileId, cachePath, file.size() );
    }

    if ( action == ActionOpen ) {
        QDesktopServices::openUrl( QUrl::fromLocalFile( cachePath ) );
    } else {
        if ( QFile::exists( path ) ) {
            if ( !QFile::remove( path ) ) {
                MessageBox::warning( mainWidget(), tr( "Error" ), tr( "File could not be overwritten." ) );
                return;
            }
        }

        if ( !QFile::copy( cachePath, path ) )
            MessageBox::warning( mainWidget(), tr( "Error" ), tr( "File could not be saved." ) );
    }
}
