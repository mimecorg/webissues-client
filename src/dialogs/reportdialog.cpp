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

#include "reportdialog.h"

#include "application.h"
#include "data/datamanager.h"
#include "data/localsettings.h"
#include "dialogs/messagebox.h"
#include "models/reportgenerator.h"
#include "models/projectsummarygenerator.h"
#include "utils/htmlwriter.h"
#include "utils/csvwriter.h"
#include "utils/iconloader.h"

#include <QLayout>
#include <QLabel>
#include <QRadioButton>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QWebPage>
#include <QPrintDialog>
#include <QFileDialog>
#include <QTextStream>
#include <QTextCodec>
#include <QSettings>
#include <QPrinter>
#include <QPrintPreviewDialog>
#include <QApplication>
#include <QDesktopWidget>

ReportDialog::ReportDialog( SourceType source, ReportMode mode, QWidget* parent ) : CommandDialog( parent ),
    m_source( source ),
    m_mode( mode ),
    m_folderId( 0 ),
    m_projectId( 0 ),
    m_history( IssueDetailsGenerator::NoHistory ),
    m_tableButton( NULL ),
    m_fullTableButton( NULL ),
    m_summaryButton( NULL ),
    m_fullReportButton( NULL )
{
    if ( source == FolderSource ) {
        m_tableButton = new QRadioButton( tr( "Table with visible columns only" ), this );
        m_fullTableButton = new QRadioButton( tr( "Table with all system and user columns" ), this );
    }

    if ( mode != ExportCsv && source != ProjectSource )
        m_summaryButton = new QRadioButton( tr( "Summary report including issue details" ), this );

    if ( mode != ExportCsv && source == IssueSource )
        m_fullReportButton = new QRadioButton( tr( "Full report including issue details and history" ), this );

    QVBoxLayout* optionsLayout = NULL;

    if ( m_tableButton || m_fullTableButton || m_summaryButton || m_fullReportButton ) {
        optionsLayout = new QVBoxLayout();
        if ( m_tableButton )
            optionsLayout->addWidget( m_tableButton );
        if ( m_fullTableButton )
            optionsLayout->addWidget( m_fullTableButton );
        if ( m_summaryButton )
            optionsLayout->addWidget( m_summaryButton );
        if ( m_fullReportButton )
            optionsLayout->addWidget( m_fullReportButton );
    }

    if ( optionsLayout || mode == Print ) {
        QHBoxLayout* layout = new QHBoxLayout();

        if ( optionsLayout )
            layout->addLayout( optionsLayout );

        if ( mode == Print ) {
            QPushButton* previewButton = new QPushButton( tr( "&Print Preview..." ), this );
            previewButton->setIcon( IconLoader::icon( "print-preview" ) );
            previewButton->setIconSize( QSize( 16, 16 ) );
            layout->addWidget( previewButton, 0, Qt::AlignBottom | Qt::AlignRight );

            connect( previewButton, SIGNAL( clicked() ), this, SLOT( showPreview() ) );
        }

        setContentLayout( layout, true );
    }

    if ( m_tableButton )
        m_tableButton->setChecked( true );
    else if ( m_summaryButton )
        m_summaryButton->setChecked( true );

    QString sourceName;
    switch ( source ) {
        case FolderSource:
            sourceName = tr( "list of issues" );
            break;
        case IssueSource:
            sourceName = tr( "issue details" );
            break;
        case ProjectSource:
            sourceName = tr( "project summary" );
            break;
    }

    switch ( mode ) {
        case Print:
            setWindowTitle( tr( "Print" ) );
            setPromptPixmap( IconLoader::pixmap( "file-print", 22 ) );
            setPrompt( tr( "Print %1:" ).arg( sourceName ) );
            break;
        case ExportCsv:
            setWindowTitle( tr( "Export To CSV" ) );
            setPromptPixmap( IconLoader::pixmap( "export-csv", 22 ) );
            setPrompt( tr( "Export %1 to CSV file:" ).arg( sourceName ) );
            break;
        case ExportHtml:
            setWindowTitle( tr( "Export To HTML" ) );
            setPromptPixmap( IconLoader::pixmap( "export-html", 22 ) );
            setPrompt( tr( "Export %1 to HTML file:" ).arg( sourceName ) );
            break;
        case ExportPdf:
            setWindowTitle( tr( "Export To PDF" ) );
            setPromptPixmap( IconLoader::pixmap( "export-pdf", 22 ) );
            setPrompt( tr( "Export %1 to PDF file:" ).arg( sourceName ) );
            break;
    }

    showInfo( tr( "Create the report." ) );
}

ReportDialog::~ReportDialog()
{
}

void ReportDialog::setIssue( int issueId )
{
    m_folderId = 0;
    m_issues.clear();
    m_issues.append( issueId );
    m_projectId = 0;
}

void ReportDialog::setFolder( int folderId, const QList<int>& issues )
{
    m_folderId = folderId;
    m_issues = issues;
    m_projectId = 0;
}

void ReportDialog::setProject( int projectId )
{
    m_projectId = projectId;
    m_folderId = 0;
    m_issues.clear();
}

void ReportDialog::setHistory( IssueDetailsGenerator::History history )
{
    m_history = history;
}

void ReportDialog::setColumns( const QList<int>& currentColumns, const QList<int>& availableColumns )
{
    m_currentColumns = currentColumns;
    m_availableColumns = availableColumns;
}

void ReportDialog::accept()
{
    bool result = false;

    switch ( m_mode ) {
        case Print:
            result = print();
            break;
        case ExportCsv:
            result = exportCsv();
            break;
        case ExportHtml:
            result = exportHtml();
            break;
        case ExportPdf:
            result = exportPdf();
            break;
    }

    if ( result )
        QDialog::accept();
}

bool ReportDialog::print()
{
    QPrinter* printer = application->printer();
    printer->setFromTo( 0, 0 );

    QPrintDialog dialog( printer, this );

    if ( dialog.exec() != QDialog::Accepted )
        return false;

    QString html = generateHtmlReport( false );

    QWebPage page;
    page.mainFrame()->setHtml( html );

    page.mainFrame()->print( printer );

    return true;
}

bool ReportDialog::exportCsv()
{
    QString fileName = getReportFileName( ".csv", tr( "CSV Files (*.csv)" ) );

    if ( fileName.isEmpty() )
        return false;

    QFile file( fileName );
    if ( !file.open( QIODevice::WriteOnly ) ) {
        MessageBox::warning( this, tr( "Warning" ), tr( "File could not be saved." ) );
        return false;
    }

    QTextStream stream( &file );
    stream.setCodec( QTextCodec::codecForName( "UTF-8" ) );

    stream << generateCsvReport();

    return true;
}

bool ReportDialog::exportHtml()
{
    QString fileName = getReportFileName( ".html", tr( "HTML Files (*.html)" ) );

    if ( fileName.isEmpty() )
        return false;

    QFile file( fileName );
    if ( !file.open( QIODevice::WriteOnly ) ) {
        MessageBox::warning( this, tr( "Warning" ), tr( "File could not be saved." ) );
        return false;
    }

    QTextStream stream( &file );
    stream.setCodec( QTextCodec::codecForName( "UTF-8" ) );

    stream << generateHtmlReport( true );

    return true;
}

bool ReportDialog::exportPdf()
{
    QString fileName = getReportFileName( ".pdf", tr( "PDF Files (*.pdf)" ) );

    if ( fileName.isEmpty() )
        return false;

    QPrinter printer( QPrinter::HighResolution );
    printer.setOutputFileName( fileName );
    printer.setOutputFormat( QPrinter::PdfFormat );

    QString html = generateHtmlReport( false );

    QWebPage page;
    page.mainFrame()->setHtml( html );

    page.mainFrame()->print( &printer );

    return true;
}

void ReportDialog::showPreview()
{
    QPrinter* printer = application->printer();
    printer->setFromTo( 0, 0 );

    QString html = generateHtmlReport( false );

    QWebPage page;
    page.mainFrame()->setHtml( html );

    QPrintPreviewDialog dialog( printer, this );
    dialog.setWindowTitle( tr( "Print Preview" ) );

    LocalSettings* settings = application->applicationSettings();
    if ( settings->contains( "PrintPreviewGeometry" ) )
        dialog.restoreGeometry( settings->value( "PrintPreviewGeometry" ).toByteArray() );
    else
        dialog.resize( QApplication::desktop()->screenGeometry( this ).size() * 4 / 5 );

    connect( &dialog, SIGNAL( paintRequested( QPrinter* ) ), page.mainFrame(), SLOT( print( QPrinter* ) ) );

    int result = dialog.exec();

    settings->setValue( "PrintPreviewGeometry", dialog.saveGeometry() );

    if ( result == QDialog::Accepted )
        QDialog::accept();
}

QString ReportDialog::generateCsvReport()
{
    ReportGenerator generator;

    generator.setFolderSource( m_folderId, m_issues );
    if ( m_tableButton->isChecked() )
        generator.setTableMode( m_currentColumns );
    else if ( m_fullTableButton->isChecked() )
        generator.setTableMode( m_availableColumns );

    CsvWriter writer;
    generator.write( &writer );

    return writer.toString();
}

QString ReportDialog::generateHtmlReport( bool embedded )
{
    HtmlWriter writer;
    writer.setEmbedded( embedded );

    if ( m_source == ProjectSource ) {
        ProjectSummaryGenerator generator;
        generator.setProject( m_projectId );

        writer.setTitle( generator.title() );
        generator.write( &writer, HtmlText::NoInternalLinks );
    } else {
        ReportGenerator generator;

        if ( m_source == FolderSource ) {
            generator.setFolderSource( m_folderId, m_issues );
            if ( m_tableButton->isChecked() )
                generator.setTableMode( m_currentColumns );
            else if ( m_fullTableButton->isChecked() )
                generator.setTableMode( m_availableColumns );
            else
                generator.setSummaryMode( IssueDetailsGenerator::NoHistory );
        } else if ( m_source == IssueSource && !m_issues.isEmpty() ) {
            generator.setIssueSource( m_issues.first() );
            generator.setSummaryMode( m_fullReportButton->isChecked() ? m_history : IssueDetailsGenerator::NoHistory );
        }

        writer.setTitle( generator.title() );
        generator.write( &writer );
    }

    return writer.toHtml();
}

QString ReportDialog::getReportFileName( const QString& extension, const QString& filter )
{
    LocalSettings* settings = application->applicationSettings();
    QString dir = settings->value( "SaveReportPath", QDir::homePath() ).toString();

    QString path = QFileDialog::getSaveFileName( this, tr( "Save As" ), dir, filter );

    if ( !path.isEmpty() ) {
        QFileInfo fileInfo( path );
        if ( fileInfo.suffix().isEmpty() )
            path += extension;
        settings->setValue( "SaveReportPath", fileInfo.absoluteDir().path() );
    }

    return path;
}
