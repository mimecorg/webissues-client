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

#ifndef REPORTGENERATOR_H
#define REPORTGENERATOR_H

#include "issuedetailsgenerator.h"

class HtmlWriter;
class CsvWriter;
class FolderRow;
class IssueRow;

class QDateTime;

/**
* Class for generating  reports.
*
* This class exctracts the information about a folder or an issue and outputs it
* to the HtmlWriter or CsvWriter.
*/
class ReportGenerator : public QObject
{
    Q_OBJECT
public:
    /**
    * Default constructor.
    */
    ReportGenerator();

    /**
    * Destructor.
    */
    ~ReportGenerator();

public:
    /**
    * Set report source to single issue.
    * @param issueId Identifier of the issue.
    */
    void setIssueSource( int issueId );

    /**
    * Set report source to multiple issues.
    * @param folderId Identifier of the folder containing the issues.
    * @param issues Identifiers of issues to be included in the report.
    */
    void setFolderSource( int folderId, const QList<int>& issues );

    /**
    * Set report mode to table of issues.
    * @param columns Identifiers of columns of the table.
    */
    void setTableMode( const QList<int>& columns );

    /**
    * Set report mode to summary of issue(s).
    * @param history Type of issue history to be included.
    */
    void setSummaryMode( IssueDetailsGenerator::History history );

    /**
    * Return the title of the report.
    */
    QString title() const { return m_title; }

    /**
    * Write the report as a text document.
    * @param writer The text document writer to output the report to.
    */
    void write( HtmlWriter* writer );

    /**
    * Write the report as a CSV file.
    * @param writer The CSV file writer to output the report to.
    */
    void write( CsvWriter* writer );

private:
    int m_folderId;
    QList<int> m_issues;

    QList<int> m_columns;
    bool m_summary;
    IssueDetailsGenerator::History m_history;

    QString m_title;
};

#endif
