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

#include "utils/textwithlinks.h"

#include <QObject>

class TextWriter;

/**
* Class providing project summary to the TextWriter.
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
    * Output the issue details to the writer.
    * @param writer The text document writer to output the details to.
    * @param flags Optional flags affecting extracting of links.
    */
    void write( TextWriter* writer, TextWithLinks::Flags flags = 0 );

private:
    int m_projectId;

    bool m_isAdmin;
};

#endif
