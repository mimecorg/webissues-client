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

#include "projectsummarygenerator.h"

#include "data/datamanager.h"
#include "data/entities.h"
#include "utils/markupprocessor.h"
#include "utils/htmlwriter.h"
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

    ProjectEntity project = ProjectEntity::find( projectId );
    m_title = project.name();

    m_isAdmin = ProjectEntity::isAdmin( projectId );
}

void ProjectSummaryGenerator::write( HtmlWriter* writer, HtmlText::Flags flags )
{
    ProjectEntity project = ProjectEntity::find( m_projectId );

    if ( project.isValid() ) {
        writer->writeBlock( project.name(), HtmlWriter::Header2Block );

        DescriptionEntity description = project.description();

        if ( description.isValid() ) {
            writer->writeBlock( descriptionLinks( description, flags ), HtmlWriter::FloatBlock );
            writer->writeBlock( tr( "Description" ), HtmlWriter::Header3Block );
            writer->writeBlock( descriptionText( description, flags ), HtmlWriter::CommentBlock );
        }
    }
}

HtmlText ProjectSummaryGenerator::descriptionLinks( const DescriptionEntity& description, HtmlText::Flags flags )
{
    HtmlText result( flags );

    Formatter formatter;
    result.appendText( tr( "Last Edited:" ) );
    result.appendText( QString::fromUtf8( " %1 — %2" ).arg( formatter.formatDateTime( description.modifiedDate(), true ), description.modifiedUser() ) );

    if ( !flags.testFlag( HtmlText::NoInternalLinks ) && m_isAdmin ) {
        result.appendText( " | " );
        result.appendImageAndTextLink( "edit-modify", tr( "Edit" ), "command://edit-description/" );
        result.appendText( " | " );
        result.appendImageAndTextLink( "edit-delete", tr( "Delete" ), "command://delete-description/" );
    }

    return result;
}

HtmlText ProjectSummaryGenerator::descriptionText( const DescriptionEntity& description, HtmlText::Flags flags )
{
    if ( description.format() == TextWithMarkup )
        return MarkupProcessor::parse( description.text(), flags );
    else
        return HtmlText::parse( description.text(), flags );
}
