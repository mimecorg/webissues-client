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

#include "reportgenerator.h"
#include "issuedetailsgenerator.h"
#include "tablemodels.h"

#include "application.h"
#include "commands/commandmanager.h"
#include "data/datamanager.h"
#include "rdb/utilities.h"
#include "utils/datetimehelper.h"
#include "utils/tablemodelshelper.h"
#include "utils/textwriter.h"
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
}

void ReportGenerator::setFolderSource( int folderId, const QList<int>& issues )
{
    m_folderId = folderId;
    m_issues = issues;
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

void ReportGenerator::write( TextWriter* writer )
{
    if ( m_folderId != 0 && !m_summary ) {
        const FolderRow* folder = dataManager->folders()->find( m_folderId );
        if ( folder )
            writer->writeBlock( folder->name(), TextWriter::Header1Block );

        IssuesTableModel model( m_folderId, NULL );

        QStringList headers;
        for ( int i = 0; i < m_columns.count(); i++ ) {
            int column = m_columns.at( i );
            QString name = model.columnName( column );
            headers.append( name );
        }
        writer->createTable( headers );

        for ( int i = 0; i < m_issues.count(); i++ ) {
            QList<TextWithLinks> cells;
            int issueId = m_issues.at( i );
            for ( int j = 0; j < m_columns.count(); j++ ) {
                int column = m_columns.at( j );
                QString text = model.text( issueId, column );
                cells.append( TextWithLinks::parse( text, TextWithLinks::NoInternalLinks ) );
            }
            writer->appendTableRow( cells );
        }

        writer->endTable();
    }

    if ( m_summary ) {
        IssueDetailsGenerator generator;

        for ( int i = 0; i < m_issues.count(); i++ ) {
            generator.setIssue( m_issues.at( i ), m_history );
            generator.write( writer, TextWithLinks::NoInternalLinks );
        }
    }
}

void ReportGenerator::write( CsvWriter* writer )
{
    if ( m_folderId != 0 && !m_summary ) {
        IssuesTableModel model( m_folderId, NULL );

        QStringList headers;
        for ( int i = 0; i < m_columns.count(); i++ ) {
            int column = m_columns.at( i );
            QString name = model.columnName( column );
            headers.append( name );
        }
        writer->appendRow( headers );

        for ( int i = 0; i < m_issues.count(); i++ ) {
            QStringList cells;
            int issueId = m_issues.at( i );
            for ( int j = 0; j < m_columns.count(); j++ ) {
                int column = m_columns.at( j );
                QString text = model.text( issueId, column );
                cells.append( text );
            }
            writer->appendRow( cells );
        }
    }
}
