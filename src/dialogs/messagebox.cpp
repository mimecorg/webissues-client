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

#include "messagebox.h"

#include "utils/iconloader.h"

#include <QDialogButtonBox>
#include <QPushButton>

MessageBox::MessageBox( QWidget* parent ) : InformationDialog( parent )
{
    disconnect( buttonBox(), NULL, this, NULL );
    connect( buttonBox(), SIGNAL( clicked( QAbstractButton* ) ), this, SLOT( buttonClicked( QAbstractButton* ) ) );
}

MessageBox::~MessageBox()
{
}

void MessageBox::setPrompt( const QString& text )
{
    InformationDialog::setPrompt( text );
}

void MessageBox::setPromptPixmap( const QPixmap& pixmap )
{
    InformationDialog::setPromptPixmap( pixmap );
}

void MessageBox::setStandardButtons( QMessageBox::StandardButtons buttons )
{
    buttonBox()->setStandardButtons( (QDialogButtonBox::StandardButtons)(int)buttons );
}

QPushButton* MessageBox::button( QMessageBox::StandardButton button )
{
    return buttonBox()->button( (QDialogButtonBox::StandardButton)button );
}

QMessageBox::StandardButton MessageBox::warning( QWidget* parent, const QString& title, const QString& prompt, QMessageBox::StandardButtons buttons /*= QMessageBox::Ok*/ )
{
    MessageBox box( parent );

    box.setWindowTitle( title );
    box.setPrompt( prompt );
    box.setPromptPixmap( IconLoader::pixmap( "status-warning", 22 ) );

    box.setStandardButtons( buttons );

    if ( buttons & QMessageBox::Cancel )
        box.button( QMessageBox::Cancel  )->setText( tr( "&Cancel" ) );

    QMessageBox helper;
    helper.setIcon( QMessageBox::Warning );
    QAccessible::updateAccessibility( &helper, 0, QAccessible::Alert );

    return (QMessageBox::StandardButton)box.exec();
}

void MessageBox::reject()
{
    if ( buttonBox()->buttons().count() == 1 )
        done( buttonBox()->standardButton( buttonBox()->buttons().first() ) );
    else if ( buttonBox()->button( QDialogButtonBox::Cancel ) )
        done( QDialogButtonBox::Cancel );
}

void MessageBox::buttonClicked( QAbstractButton* button )
{
    done( buttonBox()->standardButton( button ) );
}
