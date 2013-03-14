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

#include "commentview.h"

#include "application.h"
#include "commands/commandmanager.h"
#include "commands/issuebatch.h"
#include "data/datamanager.h"
#include "data/entities.h"
#include "data/updateevent.h"
#include "data/localsettings.h"
#include "dialogs/messagebox.h"
#include "utils/formatter.h"
#include "utils/iconloader.h"
#include "views/viewmanager.h"
#include "widgets/inputtextedit.h"
#include "xmlui/builder.h"

#include <QTextBlock>
#include <QAction>
#include <QMenu>
#include <QApplication>
#include <QClipboard>

CommentView::CommentView( QObject* parent, QWidget* parentWidget ) : View( parent ),
    m_sending( false ),
    m_issueId( 0 ),
    m_folderId( 0 )
{
    QAction* action;

    action = new QAction( IconLoader::icon( "comment-send" ), tr( "&Send Comment" ), this );
    action->setShortcut( tr( "Ctrl+Return" ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( sendComment() ), Qt::QueuedConnection );
    setAction( "sendComment", action );

    m_edit = new InputTextEdit( parentWidget );
    m_edit->setRequired( true );
    m_edit->setMaxLength( dataManager->setting( "comment_max_length" ).toInt() );
    m_edit->setAutoValidate( false );
    m_edit->setContextMenuPolicy( Qt::CustomContextMenu );

    QPalette palette = m_edit->palette();
    palette.setBrush( QPalette::Base, QColor( 0xff, 0xff, 0xff ) );
    palette.setBrush( QPalette::Text, QColor( 0x49, 0x49, 0x49 ) );
    m_edit->setPalette( palette );

    connect( m_edit, SIGNAL( customContextMenuRequested( const QPoint& ) ),
        this, SLOT( contextMenu( const QPoint& ) ) );

    action = new QAction( IconLoader::icon( "edit-undo" ), tr( "&Undo" ), this );
    action->setShortcut( QKeySequence::Undo );
    connect( action, SIGNAL( triggered() ), m_edit, SLOT( undo() ) );
    connect( m_edit, SIGNAL( undoAvailable( bool ) ), action, SLOT( setEnabled( bool ) ) );
    setAction( "editUndo", action );

    action = new QAction( IconLoader::icon( "edit-redo" ), tr( "&Redo" ), this );
    action->setShortcut( QKeySequence::Redo );
    connect( action, SIGNAL( triggered() ), m_edit, SLOT( redo() ) );
    connect( m_edit, SIGNAL( redoAvailable( bool ) ), action, SLOT( setEnabled( bool ) ) );
    setAction( "editRedo", action );

    action = new QAction( IconLoader::icon( "edit-cut" ), tr( "Cu&t" ), this );
    action->setShortcut( QKeySequence::Cut );
    connect( action, SIGNAL( triggered() ), m_edit, SLOT( cut() ) );
    connect( m_edit, SIGNAL( copyAvailable( bool ) ), action, SLOT( setEnabled( bool ) ) );
    setAction( "editCut", action );

    action = new QAction( IconLoader::icon( "edit-copy" ), tr( "&Copy" ), this );
    action->setShortcut( QKeySequence::Copy );
    connect( action, SIGNAL( triggered() ), m_edit, SLOT( copy() ) );
    connect( m_edit, SIGNAL( copyAvailable( bool ) ), action, SLOT( setEnabled( bool ) ) );
    setAction( "editCopy", action );

    action = new QAction( IconLoader::icon( "edit-paste" ), tr( "&Paste" ), this );
    action->setShortcut( QKeySequence::Paste );
    connect( action, SIGNAL( triggered() ), m_edit, SLOT( paste() ) );
    setAction( "editPaste", action );

    action = new QAction( tr( "Select &All" ), this );
    action->setShortcut( QKeySequence::SelectAll );
    connect( action, SIGNAL( triggered() ), m_edit, SLOT( selectAll() ) );
    setAction( "editSelectAll", action );

    setTitle( "sectionSend", tr( "Send" ) );
    setTitle( "sectionEdit", tr( "Edit" ) );
    setTitle( "sectionClipboard", tr( "Clipboard" ) );

    loadXmlUiFile( ":/resources/commentview.xml" );

    connect( m_edit, SIGNAL( textChanged() ), this, SLOT( updateActions() ) );

    connect( QApplication::clipboard(), SIGNAL( dataChanged() ), this, SLOT( updateClipboard() ) );

    connect( application->applicationSettings(), SIGNAL( settingsChanged() ), this, SLOT( settingsChanged() ) );

    settingsChanged();

    setMainWidget( m_edit );

    setViewerSizeHint( QSize( 600, 400 ) );
}

CommentView::~CommentView()
{
}

void CommentView::initialUpdate()
{
    setAccess( checkDataAccess(), true );

    if ( isEnabled() && id() != m_issueId ) {
        ChangeEntity change = ChangeEntity::findComment( id() );
        m_oldText = change.comment().text();
        m_edit->setInputValue( m_oldText );
    }
}

Access CommentView::checkDataAccess()
{
    ChangeEntity change = ChangeEntity::findComment( id() );
    if ( change.isValid() )
        m_issueId = change.issueId();
    else
        m_issueId = id();

    IssueEntity issue = IssueEntity::find( m_issueId );
    if ( !issue.isValid() )
        return UnknownAccess;

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

    return NormalAccess;
}

void CommentView::enableView()
{
    updateCaption();
    updateActions();
    updateClipboard();
}

void CommentView::disableView()
{
    updateCaption();
}

void CommentView::updateCaption()
{
    QString name = tr( "Unknown Issue" ) ;
    if ( isEnabled() ) {
        IssueEntity issue = IssueEntity::find( m_issueId );
        if ( issue.isValid() )
            name = issue.name();
    }
    if ( id() == m_issueId ) {
        setCaption( tr( "Add Comment - %1" ).arg( name ) );
    } else {
        QString ident = QString( "#%1" ).arg( id() );
        setCaption( tr( "Edit Comment %1 - %2" ).arg( ident, name ) );
    }
}

void CommentView::updateActions()
{
    action( "sendComment" )->setEnabled( !m_edit->inputValue().isEmpty() );
    action( "editUndo" )->setEnabled( m_edit->document()->isUndoAvailable() );
    action( "editRedo" )->setEnabled( m_edit->document()->isRedoAvailable() );
    action( "editCut" )->setEnabled( m_edit->textCursor().hasSelection() );
    action( "editCopy" )->setEnabled( m_edit->textCursor().hasSelection() );
    action( "editSelectAll" )->setEnabled( !m_edit->document()->isEmpty() );
}

void CommentView::updateClipboard()
{
    action( "editPaste" )->setEnabled( !QApplication::clipboard()->text().isEmpty() );
}

void CommentView::sendComment()
{
    if ( !m_sending && isEnabled() ) {
        QString text = m_edit->inputValue();

        if ( id() != m_issueId && text == m_oldText ) {
            viewManager->closeView( this );
            return;
        }

        m_sending = true;

        showBusy( tr( "Sending comment..." ) );

        IssueBatch* batch = new IssueBatch( m_issueId );
        if ( id() == m_issueId )
            batch->addComment( text, PlainText );
        else
            batch->editComment( id(), text, PlainText );

        connect( batch, SIGNAL( completed( bool ) ), this, SLOT( sendCompleted( bool ) ) );

        commandManager->execute( batch );
    }
}

void CommentView::sendCompleted( bool successful )
{
    if ( successful )
        viewManager->closeView( this );
    else
        showError( tr( "Comment could not be sent: %1." ).arg( commandManager->errorMessage() ) );

    m_sending = false;
}

bool CommentView::queryClose()
{
    if ( isEnabled() && m_edit->document()->isModified() && !m_sending ) {
        if ( MessageBox::warning( mainWidget(), tr( "Warning" ),
            tr( "This comment has not been sent. You will lose all changes "
            "when you close it. Are you sure you want to continue?" ),
            QMessageBox::Ok | QMessageBox::Cancel ) != QMessageBox::Ok )
            return false;
    }

    return true;
}

void CommentView::updateEvent( UpdateEvent* e )
{
    setAccess( checkDataAccess() );

    if ( isEnabled() && e->unit() == UpdateEvent::Issue && e->id() == m_issueId )
        updateCaption();

    if ( isEnabled() && e->unit() == UpdateEvent::Folder && e->id() == m_folderId )
        updateCaption();
}

void CommentView::contextMenu( const QPoint& pos )
{
    QMenu* menu = builder()->contextMenu( "menuEdit" );
    if ( menu )
        menu->popup( m_edit->mapToGlobal( pos ) );
}

void CommentView::settingsChanged()
{
    LocalSettings* settings = application->applicationSettings();
    QString family = settings->value( "CommentFont" ).toString();
    int size = settings->value( "CommentFontSize" ).toInt();

    QFont font( family, size );
    font.setStyleHint( QFont::SansSerif );

    m_edit->setFont( font );
}
