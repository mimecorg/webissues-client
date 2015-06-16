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

#ifndef REPORTDIALOG_H
#define REPORTDIALOG_H

#include "dialogs/commanddialog.h"
#include "models/issuedetailsgenerator.h"

class QRadioButton;
class QWebPage;
class QPrinter;

/**
* Dialog for printing or exporting a report.
*/
class ReportDialog : public CommandDialog
{
    Q_OBJECT
public:
    /**
    * Source of the report.
    */
    enum SourceType
    {
        /** Report based on single issue. */
        IssueSource,
        /** Report based on a folder with issues. */
        FolderSource,
        /** Report based on project summary. */
        ProjectSource
    };

    /**
    * Mode of the report.
    */
    enum ReportMode
    {
        /** Printed HTML report. */
        Print,
        /** Report saved as a CVS file. */
        ExportCsv,
        /** Report saved as a HTML file. */
        ExportHtml,
        /** Report exported to PDF format. */
        ExportPdf
    };

public:
    /**
    * Constructor.
    * @param source Type of source for the report.
    * @param mode Mode and format of the report.
    * @param parent The parent widget.
    */
    ReportDialog( SourceType source, ReportMode mode, QWidget* parent );

    /**
    * Destructor.
    */
    ~ReportDialog();

public:
    /**
    * Set the issue used as the source for the report.
    */
    void setIssue( int issueId );

    /**
    * Set the folder used as the source for the report.
    */
    void setFolder( int folderId );
    
    /**
    * Set the type of the global list used as the source for the report.
    */
    void setGlobalList( int typeId );

    /**
    * Set the issues displayed in the folder report.
    */
    void setIssues( const QList<int>& issues );

    /**
    * Set the project used as the source for the report.
    */
    void setProject( int projectId );

    /**
    * Set current columns for table reports.
    */
    void setCurrentColumns( const QList<int>& columns );

    /**
    * Set all available columns for table reports.
    */
    void setAvailableColumns( const QList<int>& columns );

    /**
    * Set type of issue history to be included for summary reports.
    */
    void setHistory( IssueDetailsGenerator::History history );

public: // overrides
    void accept();

private slots:
    void showPreview();

    void previewReady();
    void printReady();
    void pdfReady();

private:
    void print();
    void exportCsv();
    void exportHtml();
    void exportPdf();

    QString generateCsvReport();
    QString generateHtmlReport( bool embedded );

    QString getReportFileName( const QString& extension, const QString& filter );

private:
    SourceType m_source;
    ReportMode m_mode;

    int m_folderId;
    int m_typeId;
    QList<int> m_issues;
    int m_projectId;

    QList<int> m_currentColumns;
    QList<int> m_availableColumns;

    IssueDetailsGenerator::History m_history;

    QRadioButton* m_tableButton;
    QRadioButton* m_fullTableButton;
    QRadioButton* m_summaryButton;
    QRadioButton* m_fullReportButton;

    QWebPage* m_page;

    QPrinter* m_pdfPrinter;
};

#endif
