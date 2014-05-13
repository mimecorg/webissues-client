/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2014 WebIssues Team
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

#include "propertypanel.h"

#include "utils/iconloader.h"

#include <QLayout>
#include <QLabel>
#include <QTextBrowser>
#include <QFrame>
#include <QMenu>
#include <QKeyEvent>

PropertyPanel::PropertyPanel( QWidget* parent ) : QWidget( parent ),
    m_menu( false )
{
    m_layout = new QGridLayout( this );
    m_layout->setAlignment( Qt::AlignTop );
    m_layout->setColumnStretch( 1, 1 );
    m_layout->setVerticalSpacing( 3 );
}

PropertyPanel::~PropertyPanel()
{
}

void PropertyPanel::clear()
{
    QObjectList widgets = children();
    for ( int i = 0; i < widgets.count(); i++ ) {
        if ( widgets.at( i )->isWidgetType() )
            delete widgets.at( i );
    }

    m_browserMap.clear();
}

void PropertyPanel::addProperty( const QString& key, const QString& name )
{
    int row = m_layout->rowCount();

    QLabel* label = new QLabel( name, this );
    m_layout->addWidget( label, row, 0 );

    QTextBrowser* browser = new QTextBrowser( this );
    browser->document()->setDocumentMargin( 2 );
    browser->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Fixed );
    browser->setFrameStyle( QFrame::NoFrame );
    browser->viewport()->setBackgroundRole( QPalette::Window );
    browser->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    browser->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    browser->setWordWrapMode( QTextOption::NoWrap );
    browser->setContextMenuPolicy( Qt::CustomContextMenu );
    m_layout->addWidget( browser, row, 1 );

    QPalette palette = browser->palette();
    palette.setBrush( QPalette::Inactive, QPalette::Highlight, palette.brush( QPalette::Active, QPalette::Highlight ) );
    palette.setBrush( QPalette::Inactive, QPalette::HighlightedText, palette.brush( QPalette::Active, QPalette::HighlightedText ) );
    browser->setPalette( palette );

    int height = browser->fontMetrics().lineSpacing() + 4;
#if !defined( Q_WS_WIN )
    height += 1;
#endif
    browser->setFixedHeight( height );

    connect( browser, SIGNAL( customContextMenuRequested( const QPoint& ) ),
        this, SLOT( browserContextMenu( const QPoint& ) ) );

    browser->installEventFilter( this );

    if ( isVisible() ) {
        label->show();
        browser->show();
    }

    m_browserMap.insert( key, browser );
}

void PropertyPanel::addSeparator()
{
    int row = m_layout->rowCount();

    QFrame* separator = new QFrame( this );
    separator->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    m_layout->addWidget( separator, row, 0, 1, 2 );

    if ( isVisible() )
        separator->show();
}

void PropertyPanel::clearValues()
{
    QList<QTextBrowser*> browsers = m_browserMap.values();
    for ( int i = 0; i < browsers.count(); i++ )
        browsers.at( i )->clear();
}

void PropertyPanel::setValue( const QString& key, const QString& value )
{
    m_browserMap.value( key )->setPlainText( value );
}

void PropertyPanel::setInnerMargin( int margin )
{
    m_layout->setMargin( margin );
}

void PropertyPanel::browserContextMenu( const QPoint& pos )
{
    QTextBrowser* browser = (QTextBrowser*)sender();

    m_menu = true;

    QMenu menu;
    QAction* action;

    action = new QAction( IconLoader::icon( "edit-copy" ), tr( "&Copy" ), &menu );
    action->setShortcut( QKeySequence::Copy );
    action->setEnabled( browser->textCursor().hasSelection() );
    connect( action, SIGNAL( triggered() ), browser, SLOT( copy() ) );
    menu.addAction( action );

    menu.addSeparator();

    action = new QAction( tr( "Select &All" ), &menu );
    action->setShortcut( QKeySequence::SelectAll );
    action->setEnabled( !browser->document()->isEmpty() );
    connect( action, SIGNAL( triggered() ), browser, SLOT( selectAll() ) );
    menu.addAction( action );

    menu.exec( browser->mapToGlobal( pos ) );

    m_menu = false;

    if ( !browser->hasFocus() ) {
        QTextCursor cursor = browser->textCursor();
        cursor.clearSelection();
        browser->setTextCursor( cursor );
    }
}

bool PropertyPanel::eventFilter( QObject* watched, QEvent* e )
{
    if ( QTextBrowser* browser = qobject_cast<QTextBrowser*>( watched ) ) {
        if ( e->type() == QEvent::FocusOut && !m_menu ) {
            QTextCursor cursor = browser->textCursor();
            cursor.clearSelection();
            browser->setTextCursor( cursor );
        } else if ( e->type() == QEvent::ShortcutOverride ) {
            QKeyEvent* ke = (QKeyEvent*)e;
            if ( ke == QKeySequence::Copy ) {
                browser->copy();
                ke->accept();
                return true;
            } else if ( ke == QKeySequence::SelectAll ) {
                browser->selectAll();
                ke->accept();
                return true;
            }
        }
    }
    return QWidget::eventFilter( watched, e );
}
