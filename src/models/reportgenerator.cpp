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

#include "reportgenerator.h"

#include "application.h"
#include "commands/commandmanager.h"
#include "data/datamanager.h"
#include "data/entities.h"
#include "models/issuedetailsgenerator.h"
#include "models/foldermodel.h"
#include "utils/htmlwriter.h"
#include "utils/csvwriter.h"

ReportGenerator::ReportGenerator() :
    m_folderId( 0 ),
    m_summary( false ),
    m_history( IssueDetailsGenerator::NoHistory )
{
}

ReportGenerator::~ReportGenerator()
{
}

void ReportGenerator::setIssueSource( int issueId )
{
    m_folderId = 0;
    m_issues.clear();
    m_issues.append( issueId );

    IssueEntity issue = IssueEntity::find( issueId );
    m_title = issue.name();
}

void ReportGenerator::setFolderSource( int folderId, const QList<int>& issues )
{
    m_folderId = folderId;
    m_issues = issues;

    FolderEntity folder = FolderEntity::find( folderId );
    m_title = folder.name();
}

void ReportGenerator::setTableMode( const QList<int>& columns )
{
    m_columns = columns;
    m_summary = false;
}

void ReportGenerator::setSummaryMode( IssueDetailsGenerator::History history )
{
    m_summary = true;
    m_history = history;
}

void ReportGenerator::write( HtmlWriter* writer )
{
    if ( m_folderId != 0 && !m_summary ) {
        FolderEntity folder = FolderEntity::find( m_folderId );
        writer->writeBlock( folder.name(), HtmlWriter::Header2Block );

        FolderModel model( m_folderId, NULL );
        model.setColumns( m_columns );

        QStringList headers;
        for ( int i = 0; i < m_columns.count(); i++ ) {
            QString name = model.headerData( i, Qt::Horizontal, Qt::DisplayRole ).toString();
            headers.append( name );
        }
        writer->createTable( headers );

        for ( int i = 0; i < m_issues.count(); i++ ) {
            QModelIndex index = model.findIndex( 0, m_issues.at( i ), 0 );
            if ( index.isValid() ) {
                QList<HtmlText> cells;
                for ( int j = 0; j < m_columns.count(); j++ ) {
                    QString text = model.data( model.index( index.row(), j ), Qt::DisplayRole ).toString();
                    cells.append( HtmlText::parse( text, HtmlText::NoInternalLinks ) );
                }
                writer->appendTableRow( cells );
            }
        }

        writer->endTable();
    }

    if ( m_summary ) {
        IssueDetailsGenerator generator;

        for ( int i = 0; i < m_issues.count(); i++ ) {
            generator.setIssue( m_issues.at( i ), m_history );
            generator.write( writer, HtmlText::NoInternalLinks );
        }
    }
}

void ReportGenerator::write( CsvWriter* writer )
{
    if ( m_folderId != 0 && !m_summary ) {
        FolderModel model( m_folderId, NULL );
        model.setColumns( m_columns );

        QStringList headers;
        for ( int i = 0; i < m_columns.count(); i++ ) {
            QString name = model.headerData( i, Qt::Horizontal, Qt::DisplayRole ).toString();
            headers.append( name );
        }
        writer->appendRow( headers );

        for ( int i = 0; i < m_issues.count(); i++ ) {
            QModelIndex index = model.findIndex( 0, m_issues.at( i ), 0 );
            if ( index.isValid() ) {
                QStringList cells;
                for ( int j = 0; j < m_columns.count(); j++ ) {
                    QString text = model.data( model.index( index.row(), j ), Qt::DisplayRole ).toString();
                    cells.append( text );
                }
                writer->appendRow( cells );
            }
        }
    }
}
