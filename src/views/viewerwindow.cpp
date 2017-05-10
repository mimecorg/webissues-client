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

#include "viewerwindow.h"
#include "view.h"

#include "application.h"
#include "data/localsettings.h"
#include "dialogs/settingsdialog.h"
#include "utils/iconloader.h"
#include "widgets/statuslabel.h"
#include "xmlui/builder.h"
#include "xmlui/toolstrip.h"

#include <QAction>
#include <QCloseEvent>
#include <QApplication>
#include <QSettings>
#include <QToolBar>
#include <QDesktopWidget>
#include <QStatusBar>
#include <QMessageBox>
#include <QAccessible>

ViewerWindow::ViewerWindow() :
    m_view( NULL )
{
    setAttribute( Qt::WA_DeleteOnClose, true );

    QAction* action;

    action = new QAction( IconLoader::icon( "file-close" ), tr( "Close" ), this );
    action->setShortcut( tr( "Ctrl+W" ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( close() ) );
    setAction( "close", action );

    action = new QAction( IconLoader::icon( "configure" ), tr( "WebIssues Settings" ), this );
    connect( action, SIGNAL( triggered() ), this, SLOT( configure() ) );
    setAction( "configure", action );

    action = new QAction( IconLoader::icon( "about" ), tr( "About WebIssues" ), this );
    action->setShortcut( QKeySequence( Qt::Key_F1 ) );
    connect( action, SIGNAL( triggered() ), qApp, SLOT( about() ) );
    setAction( "about", action );

    loadXmlUiFile( ":/resources/viewerwindow.xml" );

    XmlUi::ToolStrip* strip = new XmlUi::ToolStrip( this );
    strip->addAuxiliaryAction( this->action( "configure" ) );
    strip->addAuxiliaryAction( this->action( "about" ) );
    setMenuWidget( strip );

    XmlUi::Builder* builder = new XmlUi::Builder( this );
    builder->registerToolStrip( "stripMain", strip );
    builder->addClient( this );

    QStatusBar* bar = statusBar();

    m_statusLabel = new StatusLabel( bar );
    bar->addWidget( m_statusLabel, 1 );

    m_summaryLabel = new StatusLabel( bar );
    bar->addWidget( m_summaryLabel, 0 );

    m_summaryLabel->hide();
}

ViewerWindow::~ViewerWindow()
{
    if ( !m_view )
        return;

    storeGeometry( false );
}

void ViewerWindow::setView( View* view )
{
    m_view = view;

    connect( view, SIGNAL( captionChanged( const QString& ) ), this, SLOT( captionChanged( const QString& ) ) );

    connect( view, SIGNAL( enabledChanged( bool ) ), this, SLOT( enabledChanged( bool ) ) );

    connect( view, SIGNAL( statusChanged( const QPixmap&, const QString&, int ) ), this, SLOT( showStatus( const QPixmap&, const QString&, int ) ) );
    connect( view, SIGNAL( summaryChanged( const QPixmap&, const QString& ) ), this, SLOT( showSummary( const QPixmap&, const QString& ) ) );

    setCentralWidget( view->mainWidget() );

    enabledChanged( view->isEnabled() );

    LocalSettings* settings = application->applicationSettings();

    QString key = m_view->viewerSizeKey();

    QString geometryKey = QString( "%1Geometry" ).arg( key );
    QString offsetKey = QString( "%1Offset" ).arg( key );

    if ( settings->contains( geometryKey ) ) {
        restoreGeometry( settings->value( geometryKey ).toByteArray() );

        if ( settings->value( offsetKey ).toBool() ) {
            QPoint position = pos() + QPoint( 40, 40 );
            QRect available = QApplication::desktop()->availableGeometry( this );
            QRect frame = frameGeometry();
            if ( position.x() + frame.width() > available.right() )
                position.rx() = available.left();
            if ( position.y() + frame.height() > available.bottom() - 20 )
                position.ry() = available.top();
            move( position );
        }
    } else {
        resize( view->viewerSizeHint() );
    }

    captionChanged( view->caption() );
}

void ViewerWindow::enabledChanged( bool enabled )
{
    m_view->mainWidget()->setVisible( enabled );

    if ( enabled )
        builder()->addClient( m_view );
    else
        builder()->removeClient( m_view );
}

void ViewerWindow::closeEvent( QCloseEvent* e )
{
    if ( !m_view || m_view->queryClose() )
        e->accept();
    else
        e->ignore();
}

void ViewerWindow::showEvent( QShowEvent* e )
{
    if ( !e->spontaneous() )
        storeGeometry( true );
}

void ViewerWindow::storeGeometry( bool offset )
{
    LocalSettings* settings = application->applicationSettings();

    QString key = m_view->viewerSizeKey();

    QString geometryKey = QString( "%1Geometry" ).arg( key );
    QString offsetKey = QString( "%1Offset" ).arg( key );

    settings->setValue( geometryKey, saveGeometry() );
    settings->setValue( offsetKey, offset );
}

QMenu* ViewerWindow::createPopupMenu()
{
    return NULL;
}

void ViewerWindow::configure()
{
    SettingsDialog dialog( this );
    dialog.exec();
}

void ViewerWindow::captionChanged( const QString& caption )
{
    setWindowTitle( tr( "%1 - WebIssues Desktop Client" ).arg( caption ) );
}

void ViewerWindow::showStatus( const QPixmap& pixmap, const QString& text, int icon )
{
    m_statusLabel->setPixmap( pixmap );
    m_statusLabel->setText( text );

    if ( icon != 0 && topLevelWidget()->isActiveWindow() ) {
        QMessageBox box;
        box.setIcon( (QMessageBox::Icon)icon );
        QAccessibleEvent event( &box, QAccessible::Alert );
        QAccessible::updateAccessibility( &event );
    }
}

void ViewerWindow::showSummary( const QPixmap& pixmap, const QString& text )
{
    if ( !text.isEmpty() || !pixmap.isNull() ) {
        m_summaryLabel->setPixmap( pixmap );
        m_summaryLabel->setText( text );
        m_summaryLabel->show();
    } else {
        m_summaryLabel->hide();
    }
}
