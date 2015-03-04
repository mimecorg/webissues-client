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

#include "statedialogs.h"

#include "commands/statebatch.h"
#include "data/datamanager.h"
#include "data/entities.h"
#include "utils/iconloader.h"

#include <QLayout>
#include <QLabel>

IssueStateDialog::IssueStateDialog( int issueId, int readId, QWidget* parent ) : CommandDialog( parent ),
    m_issueId( issueId ),
    m_readId( readId )
{
    IssueEntity issue = IssueEntity::find( issueId );

    if ( readId > 0 )
        setWindowTitle( tr( "Mark As Read" ) );
    else
        setWindowTitle( tr( "Mark As Unread" ) );
    setPrompt( tr( "Updating the state of issue <b>%1</b>." ).arg( issue.name() ) );
    setPromptPixmap( IconLoader::pixmap( ( readId > 0 ) ? "issue" : "issue-unread", 22 ) );

    setContentLayout( NULL, true );
}

IssueStateDialog::~IssueStateDialog()
{
}

void IssueStateDialog::accept()
{
    StateBatch* batch = new StateBatch();
    batch->setIssueRead( m_issueId, m_readId );

    executeBatch( batch );
}

FolderStateDialog::FolderStateDialog( int folderId, int readId, QWidget* parent ) : CommandDialog( parent ),
    m_folderId( folderId ),
    m_readId( readId )
{
    FolderEntity folder = FolderEntity::find( folderId );

    if ( readId > 0 ) {
        setWindowTitle( tr( "Mark All As Read" ) );
        setPrompt( tr( "Are you sure you want to mark all issues in folder <b>%1</b> as read?" ).arg( folder.name() ) );
        setPromptPixmap( IconLoader::pixmap( "folder-read", 22 ) );
    } else {
        setWindowTitle( tr( "Mark All As Unread" ) );
        setPrompt( tr( "Are you sure you want to mark all issues in folder <b>%1</b> as unread?" ).arg( folder.name() ) );
        setPromptPixmap( IconLoader::pixmap( "folder-unread", 22 ) );
    }

    setContentLayout( NULL, true );
}

FolderStateDialog::~FolderStateDialog()
{
}

void FolderStateDialog::accept()
{
    StateBatch* batch = new StateBatch();
    batch->setFolderRead( m_folderId, m_readId );

    executeBatch( batch );
}

GlobalListStateDialog::GlobalListStateDialog( int typeId, bool isRead, QWidget* parent ) : CommandDialog( parent ),
    m_typeId( typeId ),
    m_isRead( isRead )
{
    TypeEntity type = TypeEntity::find( typeId );

    if ( isRead ) {
        setWindowTitle( tr( "Mark All As Read" ) );
        setPrompt( tr( "Are you sure you want to mark all issues of type <b>%1</b> as read?" ).arg( type.name() ) );
        setPromptPixmap( IconLoader::pixmap( "folder-read", 22 ) );
    } else {
        setWindowTitle( tr( "Mark All As Unread" ) );
        setPrompt( tr( "Are you sure you want to mark all issues of type <b>%1</b> as unread?" ).arg( type.name() ) );
        setPromptPixmap( IconLoader::pixmap( "folder-unread", 22 ) );
    }

    setContentLayout( NULL, true );
}

GlobalListStateDialog::~GlobalListStateDialog()
{
}

void GlobalListStateDialog::accept()
{
    StateBatch* batch = NULL;

    foreach ( const FolderEntity& folder, FolderEntity::list() ) {
        if ( folder.typeId() == m_typeId && folder.stampId() > 0 ) {
            if ( !batch )
                batch = new StateBatch();
            if ( m_isRead )
                batch->setFolderRead( folder.id(), folder.stampId() );
            else
                batch->setFolderRead( folder.id(), 0 );
        }
    }

    if ( batch )
        executeBatch( batch );
    else
        QDialog::accept();
}

AddSubscriptionDialog::AddSubscriptionDialog( int issueId, QWidget* parent ) : CommandDialog( parent ),
    m_issueId( issueId )
{
    IssueEntity issue = IssueEntity::find( issueId );

    setWindowTitle( tr( "Subscribe To Issue" ) );
    setPrompt( tr( "You are about to subscribe to issue <b>%1</b>." ).arg( issue.name() ) );
    setPromptPixmap( IconLoader::pixmap( "issue-subscribe", 22 ) );
    showInfo( tr( "Please confirm." ) );

    QVBoxLayout* layout = new QVBoxLayout();

    QLabel* label = new QLabel( tr( "You will receive email notifications when someone else modifies this issue, adds a comment or attachment." ), this );
    label->setWordWrap( true );

    layout->addWidget( label );

    if ( dataManager->preference( "email" ).isEmpty() ) {
        layout->addSpacing( 5 );

        QHBoxLayout* warningLayout = new QHBoxLayout();

        QLabel* warningIcon = new QLabel( this );
        warningIcon->setPixmap( IconLoader::pixmap( "status-warning" ) );
        warningLayout->addWidget( warningIcon );

        QLabel* warningLabel = new QLabel( tr( "You will not receive any emails until you enter an email address in your preferences." ), this );
        warningLabel->setWordWrap( true );
        warningLayout->addWidget( warningLabel, 1 );

        layout->addLayout( warningLayout );
    }

    setContentLayout( layout, true );
}

AddSubscriptionDialog::~AddSubscriptionDialog()
{
}

void AddSubscriptionDialog::accept()
{
    StateBatch* batch = new StateBatch();
    batch->addSubscription( m_issueId );

    executeBatch( batch );
}

DeleteSubscriptionDialog::DeleteSubscriptionDialog( int issueId, QWidget* parent ) : CommandDialog( parent ),
    m_issueId( issueId )
{
    IssueEntity issue = IssueEntity::find( issueId );

    setWindowTitle( tr( "Unsubscribe From Issue" ) );
    setPrompt( tr( "You are about to unsubscribe from issue <b>%1</b>." ).arg( issue.name() ) );
    setPromptPixmap( IconLoader::pixmap( "issue-unsubscribe", 22 ) );
    showInfo( tr( "Please confirm." ) );

    QVBoxLayout* layout = new QVBoxLayout();

    QLabel* label = new QLabel( tr( "You will no longer receive email notifications for this issue." ), this );
    label->setWordWrap( true );

    layout->addWidget( label );

    setContentLayout( layout, true );
}

DeleteSubscriptionDialog::~DeleteSubscriptionDialog()
{
}

void DeleteSubscriptionDialog::accept()
{
    IssueEntity issue = IssueEntity::find( m_issueId );
    int subscriptionId = issue.subscriptionId();

    if ( subscriptionId == 0 ) {
        QDialog::accept();
        return;
    }

    StateBatch* batch = new StateBatch();
    batch->deleteSubscription( subscriptionId );

    executeBatch( batch );
}
