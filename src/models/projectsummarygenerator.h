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

#ifndef PROJECTSUMMARYGENERATOR_H
#define PROJECTSUMMARYGENERATOR_H

#include "utils/htmltext.h"

#include <QObject>

class HtmlWriter;
class DescriptionEntity;

/**
* Class providing project summary to the HtmlWriter.
*
* This class extracts the summary of a project and outputs it
* to the TextWriter.
*/
class ProjectSummaryGenerator : public QObject
{
    Q_OBJECT
public:
    /**
    * Default constructor.
    */
    ProjectSummaryGenerator();

    /**
    * Destructor.
    */
    ~ProjectSummaryGenerator();

public:
    /**
    * Set the project to provide details for.
    * @param projectId Idenitifier of the project.
    */
    void setProject( int projectId );

    /**
    * Return the title of the report.
    */
    const QString& title() const { return m_title; }

    /**
    * Output the issue details to the writer.
    * @param writer The text document writer to output the details to.
    * @param flags Optional flags affecting extracting of links.
    */
    void write( HtmlWriter* writer, HtmlText::Flags flags = 0 );

private:
    HtmlText descriptionLinks( const DescriptionEntity& description, HtmlText::Flags flags );

    HtmlText descriptionText( const DescriptionEntity& description, HtmlText::Flags flags );

private:
    int m_projectId;

    QString m_title;

    bool m_isAdmin;
};

#endif
