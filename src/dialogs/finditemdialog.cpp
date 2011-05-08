/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2011 WebIssues Team
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

#include "finditemdialog.h"

#include "data/entities.h"
#include "commands/finditembatch.h"
#include "utils/iconloader.h"

#include <QLayout>
#include <QLabel>
#include <QSpinBox>

FindItemDialog::FindItemDialog( QWidget* parent ) : CommandDialog( parent ),
    m_itemId( 0 ),
    m_issueId( 0 )
{
    setWindowTitle( tr( "Go To Item" ) );
    setPrompt( tr( "Enter an issue, comment or attachment identifier:" ) );
    setPromptPixmap( IconLoader::pixmap( "edit-goto", 22 ) );

    showInfo( tr( "Enter item identifier." ) );

    QHBoxLayout* layout = new QHBoxLayout();

    QLabel* label = new QLabel( tr( "&ID:" ), this );
    layout->addWidget( label, 0 );

    m_idSpin = new QSpinBox( this );
    m_idSpin->setRange( 1, INT_MAX );
    m_idSpin->setPrefix( "#" );
    layout->addWidget( m_idSpin, 1 );

    label->setBuddy( m_idSpin );

    setContentLayout( layout, true );

    m_idSpin->setFocus();
    m_idSpin->selectAll();
}

FindItemDialog::~FindItemDialog()
{
}

void FindItemDialog::findItem( int itemId )
{
    m_idSpin->setValue( itemId );

    accept();
}

void FindItemDialog::accept()
{
    m_itemId = m_idSpin->value();

    m_issueId = IssueEntity::findItem( m_itemId );
    if ( m_issueId != 0 ) {
        QDialog::accept();
        return;
    }

    FindItemBatch* batch = new FindItemBatch( m_itemId );

    executeBatch( batch );
}

bool FindItemDialog::batchSuccessful( AbstractBatch* batch )
{
    m_issueId = ( (FindItemBatch*)batch )->issueId();

    return true;
}
