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

#include "dialogmanager.h"

#include "application.h"
#include "data/localsettings.h"
#include "dialogs/commanddialog.h"
#include "dialogs/informationdialog.h"

#include <QDialog>
#include <QCloseEvent>
#include <QDesktopWidget>

DialogManager* dialogManager = NULL;

DialogManager::DialogManager()
{
}

DialogManager::~DialogManager()
{
    closeAllDialogs();
}

void DialogManager::addDialog( QDialog* dialog, int id )
{
    m_dialogs.insert( dialog, id );

    dialog->installEventFilter( this );

    restoreGeometry( dialog );

    dialog->setAttribute( Qt::WA_DeleteOnClose, true );

    connect( dialog, SIGNAL( destroyed( QObject* ) ), this, SLOT( dialogDestroyed( QObject* ) ) );
}

bool DialogManager::activateDialog( const char* className, int id )
{
    for ( QMap<QDialog*, int>::const_iterator it = m_dialogs.begin(); it != m_dialogs.end(); ++it ) {
        if ( it.key()->inherits( className ) && ( id == 0 || it.value() == id ) ) {
            it.key()->raise();
            it.key()->activateWindow();
            return true;
        }
    }
    return false;
}

bool DialogManager::queryCloseDialogs()
{
    foreach ( QDialog* dialog, m_dialogs.keys() ) {
        QCloseEvent e;
        QApplication::sendEvent( dialog, &e );
        if ( !e.isAccepted() )
            return false;
    }
    return true;
}

void DialogManager::closeAllDialogs()
{
    qDeleteAll( m_dialogs.keys() );

    m_dialogs.clear();
}

bool DialogManager::eventFilter( QObject* object, QEvent* e )
{
    if ( e->type() == QEvent::Show || e->type() == QEvent::Hide ) {
        if ( !e->spontaneous() )
            storeGeometry( (QDialog*)object, e->type() == QEvent::Show );
    }
    return false;
}

static inline QString sizeKeyOf( QDialog* dialog )
{
    if ( CommandDialog* commandDialog = qobject_cast<CommandDialog*>( dialog ) ) {
        if ( commandDialog->isFixed() )
            return QString();
        return CommandDialog::staticMetaObject.className();
    }

    if ( InformationDialog* informationDialog = qobject_cast<InformationDialog*>( dialog ) )
        return informationDialog->dialogSizeKey();

    return dialog->metaObject()->className();
}

void DialogManager::storeGeometry( QDialog* dialog, bool offset )
{
    LocalSettings* settings = application->applicationSettings();

    QString key = sizeKeyOf( dialog );
    if ( key.isEmpty() )
        return;

    QString geometryKey = QString( "%1Geometry" ).arg( key );
    QString offsetKey = QString( "%1Offset" ).arg( key );

    settings->setValue( geometryKey, dialog->saveGeometry() );
    settings->setValue( offsetKey, offset );
}

void DialogManager::restoreGeometry( QDialog* dialog )
{
    LocalSettings* settings = application->applicationSettings();

    QString key = sizeKeyOf( dialog );
    if ( key.isEmpty() )
        return;

    QString geometryKey = QString( "%1Geometry" ).arg( key );
    QString offsetKey = QString( "%1Offset" ).arg( key );

    if ( settings->contains( geometryKey ) ) {
        dialog->restoreGeometry( settings->value( geometryKey ).toByteArray() );

        if ( settings->value( offsetKey ).toBool() ) {
            QPoint position = dialog->pos() + QPoint( 40, 40 );
            QRect available = QApplication::desktop()->availableGeometry( dialog );
            QRect frame = dialog->frameGeometry();
            if ( position.x() + frame.width() > available.right() )
                position.rx() = available.left();
            if ( position.y() + frame.height() > available.bottom() - 20 )
                position.ry() = available.top();
            dialog->move( position );
        }
    }
}

void DialogManager::dialogDestroyed( QObject* dialog )
{
    m_dialogs.remove( (QDialog*)dialog );
}
