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

#include "projectsummarygenerator.h"

#include "data/datamanager.h"
#include "data/entities.h"
#include "utils/datetimehelper.h"
#include "utils/textwriter.h"
#include "utils/formatter.h"

ProjectSummaryGenerator::ProjectSummaryGenerator() :
    m_projectId( 0 ),
    m_isAdmin( false )
{
}

ProjectSummaryGenerator::~ProjectSummaryGenerator()
{
}

void ProjectSummaryGenerator::setProject( int projectId )
{
    m_projectId = projectId;

    m_isAdmin = ProjectEntity::isAdmin( projectId );
}

void ProjectSummaryGenerator::write( TextWriter* writer, TextWithLinks::Flags flags /*= 0*/ )
{
    ProjectEntity project = ProjectEntity::find( m_projectId );

    if ( project.isValid() ) {
        writer->writeBlock( project.name(), TextWriter::Header1Block );

        DescriptionEntity description = project.description();

        if ( description.isValid() ) {
            writer->createLayout( 2, 2 );

            writer->gotoLayoutCell( 0, 0, TextWriter::NormalCell );
            writer->writeBlock( tr( "Description" ), TextWriter::Header2Block );

            TextWithLinks result( flags );
            Formatter formatter;
            result.appendText( tr( "Last Edited:" ) );
            result.appendText( QString::fromUtf8( " %1 — %2" ).arg( formatter.formatDateTime( description.modifiedDate(), true ), description.modifiedUser() ) );

            writer->gotoLayoutCell( 0, 1, TextWriter::NormalCell );
            writer->writeBlock( result, TextWriter::LinksBlock );

            writer->mergeLayoutCells( 1, 0, 1, 2 );
            writer->gotoLayoutCell( 1, 0, TextWriter::CommentCell );
            writer->writeBlock( TextWithLinks::parse( description.text(), flags ), TextWriter::NormalBlock );
        }
    }
}
