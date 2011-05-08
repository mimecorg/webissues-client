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

#include "updatebatch.h"
#include "command.h"

#include "data/datamanager.h"

UpdateBatch::UpdateBatch( int priority ) : AbstractBatch( priority ),
    m_ifNeeded( false )
{
}

UpdateBatch::~UpdateBatch()
{
}

void UpdateBatch::updateSettings()
{
    m_queue.addJob( &UpdateBatch::updateSettingsJob );
}

void UpdateBatch::updateLocale()
{
    m_queue.addJob( &UpdateBatch::updateLocaleJob );
}

void UpdateBatch::updatePreferences( int userId )
{
    Job job( &UpdateBatch::updatePreferencesJob );
    job.addArg( userId );
    m_queue.addJob( job );
}

void UpdateBatch::updateUsers()
{
    m_queue.addJob( &UpdateBatch::updateUsersJob );
}

void UpdateBatch::updateTypes()
{
    m_queue.addJob( &UpdateBatch::updateTypesJob );
}

void UpdateBatch::updateProjects()
{
    m_queue.addJob( &UpdateBatch::updateProjectsJob );
}

void UpdateBatch::updateStates()
{
    m_queue.addJob( &UpdateBatch::updateStatesJob );
}

void UpdateBatch::updateFolder( int folderId )
{
    Job job( &UpdateBatch::updateFolderJob );
    job.addArg( folderId );
    m_queue.addJob( job );
}

void UpdateBatch::updateIssue( int issueId )
{
    Job job( &UpdateBatch::updateIssueJob );
    job.addArg( issueId );
    m_queue.addJob( job );
}

void UpdateBatch::setIfNeeded( bool value )
{
    m_ifNeeded = value;
}

Command* UpdateBatch::fetchNext()
{
    while ( m_queue.moreJobs() ) {
        Command* command = m_queue.callJob( this );
        if ( command )
            return command;
    }

    return NULL;
}

Command* UpdateBatch::updateSettingsJob( const Job& /*job*/ )
{
    return dataManager->updateSettings();
}

Command* UpdateBatch::updateLocaleJob( const Job& job )
{
    if ( dataManager->localeUpdateNeeded() )
        return dataManager->updateLocale();
}

Command* UpdateBatch::updatePreferencesJob( const Job& job )
{
    return dataManager->updatePreferences( job.argInt( 0 ) );
}

Command* UpdateBatch::updateUsersJob( const Job& /*job*/ )
{
    return dataManager->updateUsers();
}

Command* UpdateBatch::updateTypesJob( const Job& /*job*/ )
{
    return dataManager->updateTypes();
}

Command* UpdateBatch::updateProjectsJob( const Job& /*job*/ )
{
    return dataManager->updateProjects();
}

Command* UpdateBatch::updateStatesJob( const Job& /*job*/ )
{
    return dataManager->updateStates();
}

Command* UpdateBatch::updateFolderJob( const Job& job )
{
    int folderId = job.argInt( 0 );

    if ( !m_ifNeeded || dataManager->folderUpdateNeeded( folderId ) )
        return dataManager->updateFolder( folderId );

    return NULL;
}

Command* UpdateBatch::updateIssueJob( const Job& job )
{
    int issueId = job.argInt( 0 );

    if ( !m_ifNeeded || dataManager->issueUpdateNeeded( issueId ) )
        return dataManager->updateIssue( issueId );

    return NULL;
}
