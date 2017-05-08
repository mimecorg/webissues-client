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

#include "folderview.h"

#include "commands/updatebatch.h"
#include "data/datamanager.h"
#include "data/entities.h"
#include "data/issuetypecache.h"
#include "data/updateevent.h"
#include "dialogs/dialogmanager.h"
#include "dialogs/issuedialogs.h"
#include "dialogs/reportdialog.h"
#include "dialogs/statedialogs.h"
#include "models/foldermodel.h"
#include "views/viewmanager.h"

FolderView::FolderView( QObject* parent, QWidget* parentWidget ) : ListView( parent, parentWidget )
{
    setProjectComboVisible( false );
}

FolderView::~FolderView()
{
}

void FolderView::initializeList()
{
    m_model->initializeFolder( id() );

    setAccess( checkDataAccess(), true );

    initialUpdateFolder();
}

Access FolderView::checkDataAccess()
{
    FolderEntity folder = FolderEntity::find( id() );
    if ( !folder.isValid() )
        return NoAccess;

    m_typeId = folder.typeId();

    if ( dataManager->currentUserAccess() != AdminAccess ) {
        MemberEntity member = MemberEntity::findEffective( folder.projectId(), dataManager->currentUserId() );
        if ( !member.isValid() )
            return NoAccess;
    }

    TypeEntity type = folder.type();
    if ( !type.isValid() )
        return UnknownAccess;

    if ( FolderEntity::isAdmin( id() ) )
        return AdminAccess;

    return NormalAccess;
}

void FolderView::updateCaption()
{
    QString name = tr( "Unknown Folder" );
    if ( isEnabled() ) {
        FolderEntity folder = FolderEntity::find( id() );
        if ( folder.isValid() )
            name = folder.name();
    }
    setCaption( name );
}

void FolderView::updateSelection()
{
    if ( m_selectedIssueId ) {
        IssueEntity issue = IssueEntity::find( m_selectedIssueId );
        m_isRead = issue.isValid() ? issue.readId() >= issue.stampId() : false;
        m_isSubscribed = issue.isValid() ? issue.subscriptionId() != 0 : false;
        m_isAdmin = access() == AdminAccess;
    }

    FolderEntity folder = FolderEntity::find( id() );
    m_hasIssues = folder.stampId() > 0;
}

void FolderView::initialUpdateFolder()
{
    if ( isEnabled() && dataManager->folderUpdateNeeded( id() ) ) {
        UpdateBatch* batch = new UpdateBatch();
        batch->setIfNeeded( true );
        batch->updateFolder( id() );

        executeUpdate( batch );
    }
}

void FolderView::updateFolder()
{
    if ( isEnabled() && !isUpdating() ) {
        UpdateBatch* batch = new UpdateBatch();
        batch->updateFolder( id() );

        executeUpdate( batch );
    }
}

void FolderView::cascadeUpdateFolder()
{
    if ( isEnabled() && !isUpdating() && dataManager->folderUpdateNeeded( id() ) ) {
        UpdateBatch* batch = new UpdateBatch( -10 );
        batch->setIfNeeded( true );
        batch->updateFolder( id() );

        executeUpdate( batch );
    }
}

void FolderView::addIssue()
{
    if ( isEnabled() ) {
        if ( dialogManager->activateDialog( "AddIssueDialog", id() ) )
            return;

        AddIssueDialog* dialog = new AddIssueDialog( id() );
        dialogManager->addDialog( dialog, id() );

        connect( dialog, SIGNAL( issueAdded( int, int ) ), this, SLOT( issueAdded( int, int ) ) );

        dialog->show();
    }
}

void FolderView::issueAdded( int issueId, int folderId )
{
    if ( viewManager->isStandAlone( this ) ) {
        if ( id() == folderId )
            setSelectedIssueId( issueId );

        viewManager->openIssueView( issueId );
    } else {
        emit issueActivated( issueId, issueId );
    }
}

void FolderView::markAllAsRead()
{
    if ( isEnabled() ) {
        FolderEntity folder = FolderEntity::find( id() );

        FolderStateDialog dialog( id(), folder.stampId(), mainWidget() );
        dialog.exec();
    }
}

void FolderView::markAllAsUnread()
{
    if ( isEnabled() ) {
        FolderStateDialog dialog( id(), 0, mainWidget() );
        dialog.exec();
    }
}

void FolderView::initializeReport( ReportDialog* dialog )
{
    IssueTypeCache* cache = dataManager->issueTypeCache( m_typeId );

    dialog->setFolder( id() );
    dialog->setAvailableColumns( cache->availableColumns( false ) );
}

void FolderView::updateEvent( UpdateEvent* e )
{
    setAccess( checkDataAccess() );

    if ( isEnabled() ) {
        if ( e->unit() == UpdateEvent::Projects )
            updateCaption();
    }

    if ( id() != 0 && e->unit() == UpdateEvent::Projects )
        cascadeUpdateFolder();

    ListView::updateEvent( e );
}
