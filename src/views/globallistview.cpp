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

#include "globallistview.h"

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

#include <QAction>

GlobalListView::GlobalListView( QObject* parent, QWidget* parentWidget ) : ListView( parent, parentWidget )
{
    action( "updateFolder" )->setText( tr( "&Update All Issues" ) );
    setTitle( "sectionFolder", tr( "All Issues" ) );
}

GlobalListView::~GlobalListView()
{
}

void GlobalListView::initializeList()
{
    m_model->initializeGlobalList( id() );

    setAccess( checkDataAccess(), true );

    initialUpdateFolder();
}

Access GlobalListView::checkDataAccess()
{
    TypeEntity type = TypeEntity::find( id() );
    if ( !type.isValid() )
        return NoAccess;

    m_typeId = id();

    return NormalAccess;
}

void GlobalListView::updateCaption()
{
    QString name = tr( "Unknown Type" );
    if ( isEnabled() ) {
        TypeEntity type = TypeEntity::find( id() );
        if ( type.isValid() )
            name = type.name();
    }
    setCaption( name );
}

void GlobalListView::updateSelection()
{
    if ( m_selectedIssueId ) {
        IssueEntity issue = IssueEntity::find( m_selectedIssueId );
        m_isRead = issue.isValid() ? issue.readId() >= issue.stampId() : false;
        m_isSubscribed = issue.isValid() ? issue.subscriptionId() != 0 : false;
        m_isAdmin = IssueEntity::isAdmin( m_selectedIssueId );
    }

    foreach ( const FolderEntity& folder, FolderEntity::list() ) {
        if ( folder.typeId() == m_typeId && folder.stampId() > 0 )
            m_hasIssues = true;
    }
}

void GlobalListView::initialUpdateFolder()
{
    if ( isEnabled() ) {
        UpdateBatch* batch = NULL;

        foreach ( const FolderEntity& folder, FolderEntity::list() ) {
            if ( dataManager->folderUpdateNeeded( folder.id() ) ) {
                if ( !batch ) {
                    batch = new UpdateBatch();
                    batch->setIfNeeded( true );
                }
                batch->updateFolder( folder.id() );
            }
        }

        if ( batch )
            executeUpdate( batch );
    }
}

void GlobalListView::updateFolder()
{
    if ( isEnabled() && !isUpdating() ) {
        UpdateBatch* batch = NULL;

        foreach ( const FolderEntity& folder, FolderEntity::list() ) {
            if ( !batch )
                batch = new UpdateBatch();
            batch->updateFolder( folder.id() );
        }

        if ( batch )
            executeUpdate( batch );
    }
}

void GlobalListView::cascadeUpdateFolder()
{
    if ( isEnabled() && !isUpdating() ) {
        UpdateBatch* batch = NULL;

        foreach ( const FolderEntity& folder, FolderEntity::list() ) {
            if ( dataManager->folderUpdateNeeded( folder.id() ) ) {
                if ( !batch ) {
                    batch = new UpdateBatch( -10 );
                    batch->setIfNeeded( true );
                }
                batch->updateFolder( folder.id() );
            }
        }

        if ( batch )
            executeUpdate( batch );
    }
}

void GlobalListView::addIssue()
{
    if ( isEnabled() ) {
        if ( dialogManager->activateDialog( "AddGlobalIssueDialog", id() ) )
            return;

        AddGlobalIssueDialog* dialog = new AddGlobalIssueDialog( id() );
        dialogManager->addDialog( dialog, id() );

        connect( dialog, SIGNAL( issueAdded( int, int ) ), this, SLOT( issueAdded( int, int ) ) );

        dialog->show();
    }
}

void GlobalListView::issueAdded( int issueId, int folderId )
{
    if ( viewManager->isStandAlone( this ) ) {
        FolderEntity folder = FolderEntity::find( folderId );
        if ( id() == folder.typeId() )
            setSelectedIssueId( issueId );

        viewManager->openIssueView( issueId );
    } else {
        emit issueActivated( issueId, issueId );
    }
}

void GlobalListView::markAllAsRead()
{
    if ( isEnabled() ) {
        GlobalListStateDialog dialog( id(), true, mainWidget() );
        dialog.exec();
    }
}

void GlobalListView::markAllAsUnread()
{
    if ( isEnabled() ) {
        GlobalListStateDialog dialog( id(), false, mainWidget() );
        dialog.exec();
    }
}

void GlobalListView::initializeReport( ReportDialog* dialog )
{
    IssueTypeCache* cache = dataManager->issueTypeCache( m_typeId );

    dialog->setGlobalList( id() );
    dialog->setAvailableColumns( cache->availableColumns( true ) );
}

void GlobalListView::updateEvent( UpdateEvent* e )
{
    setAccess( checkDataAccess() );

    if ( isEnabled() ) {
        if ( e->unit() == UpdateEvent::Types )
            updateCaption();
    }

    if ( id() != 0 && e->unit() == UpdateEvent::Projects )
        cascadeUpdateFolder();

    ListView::updateEvent( e );
}
