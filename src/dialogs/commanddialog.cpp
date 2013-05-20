/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2013 WebIssues Team
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

#include "commanddialog.h"

#include "commands/abstractbatch.h"
#include "commands/commandmanager.h"
#include "dialogs/messagebox.h"
#include "utils/errorhelper.h"
#include "utils/iconloader.h"
#include "widgets/inputlineedit.h"
#include "widgets/inputtextedit.h"
#include "widgets/markuptextedit.h"
#include "widgets/elidedlabel.h"
#include "xmlui/gradientwidget.h"

#include <QLayout>
#include <QLabel>
#include <QProgressBar>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QTabWidget>
#include <QScrollArea>
#include <QMessageBox>
#include <QAccessible>

CommandDialog::CommandDialog( QWidget* parent, Qt::WindowFlags flags ) : QDialog( parent, flags ),
    m_promptPixmap( NULL ),
    m_contentLayout( NULL ),
    m_fixed( false ),
    m_statusSet( false ),
    m_queryClose( false ),
    m_progressBar( NULL ),
    m_progressLabel( NULL ),
    m_batch( NULL )
{
    QVBoxLayout* topLayout = new QVBoxLayout( this );
    topLayout->setMargin( 0 );
    topLayout->setSpacing( 0 );

    m_promptWidget = new XmlUi::GradientWidget( this );
    topLayout->addWidget( m_promptWidget );

    m_promptLayout = new QHBoxLayout( m_promptWidget );
    m_promptLayout->setSpacing( 10 );

    m_promptLabel = new QLabel( m_promptWidget );
    m_promptLabel->setWordWrap( true );
    m_promptLayout->addWidget( m_promptLabel, 1 );

    QFrame* separator = new QFrame( this );
    separator->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    topLayout->addWidget( separator );

    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setMargin( 9 );
    m_mainLayout->setSpacing( 6 );
    topLayout->addLayout( m_mainLayout );

    QHBoxLayout* statusLayout = new QHBoxLayout();
    m_mainLayout->addLayout( statusLayout );

    m_statusPixmap = new QLabel( this );
    statusLayout->addWidget( m_statusPixmap, 0 );

    m_statusLabel = new ElidedLabel( this );
    statusLayout->addWidget( m_statusLabel, 1 );

    m_buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        Qt::Horizontal, this );
    m_mainLayout->addWidget( m_buttonBox );

    m_buttonBox->button( QDialogButtonBox::Ok )->setText( tr( "&OK" ) );
    m_buttonBox->button( QDialogButtonBox::Cancel )->setText( tr( "&Cancel" ) );

    connect( m_buttonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
    connect( m_buttonBox, SIGNAL( rejected() ), this, SLOT( reject() ) );
}

CommandDialog::~CommandDialog()
{
}

void CommandDialog::setPrompt( const QString& text )
{
    m_promptLabel->setText( text );

    m_promptLabel->setMinimumWidth( 350 );
    m_promptLabel->setFixedHeight( m_promptLabel->heightForWidth( 350 ) );
}

void CommandDialog::setPromptPixmap( const QPixmap& pixmap )
{
    if ( !m_promptPixmap ) {
        m_promptPixmap = new QLabel( m_promptWidget );
        m_promptLayout->insertWidget( 0, m_promptPixmap, 0 );
    }

    m_promptPixmap->setPixmap( pixmap );
}

void CommandDialog::setContentLayout( QLayout* layout, bool fixed )
{
    if ( layout ) {
        m_contentLayout = layout;

        m_mainLayout->insertLayout( 0, layout );

        QFrame* separator = new QFrame( this );
        separator->setFrameStyle( QFrame::HLine | QFrame::Sunken );
        m_mainLayout->insertWidget( 1, separator );
    }

    if ( !m_statusSet )
        showInfo( layout ? tr( "Please enter values." ) : tr( "Please confirm." ) );

    if ( fixed ) {
        m_fixed = true;
        setFixedHeight( sizeHint().height() );
    }
}

void CommandDialog::createProgressPanel( int total, const QString& text )
{
    m_progressBar = new QProgressBar( this );
    m_progressBar->setRange( 0, total );
    m_progressBar->setTextVisible( false );
    m_progressBar->setEnabled( false );
    m_mainLayout->insertWidget( 2, m_progressBar );

    m_progressLabel = new QLabel( text, this );
    m_mainLayout->insertWidget( 3, m_progressLabel );

    QFrame* separator = new QFrame( this );
    separator->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    m_mainLayout->insertWidget( 4, separator );

    m_progressText = text;

    if ( m_fixed )
        setFixedHeight( sizeHint().height() );
}

void CommandDialog::setProgress( int done, const QString& text )
{
    m_progressBar->setValue( done );
    m_progressLabel->setText( text );
}

void CommandDialog::showInfo( const QString& text )
{
    m_statusPixmap->setPixmap( IconLoader::pixmap( "status-info" ) );
    m_statusLabel->setText( text );
    m_statusSet = true;
}

void CommandDialog::showWarning( const QString& text )
{
    m_statusPixmap->setPixmap( IconLoader::pixmap( "status-warning" ) );
    m_statusLabel->setText( text );
    m_statusSet = true;

    QMessageBox box;
    box.setIcon( QMessageBox::Warning );
    QAccessible::updateAccessibility( &box, 0, QAccessible::Alert );
}

void CommandDialog::showWarning( int code )
{
    ErrorHelper helper;
    showWarning( helper.statusMessage( (ErrorHelper::ErrorCode)code ) );
}

void CommandDialog::showError( const QString& text )
{
    m_statusPixmap->setPixmap( IconLoader::pixmap( "status-error" ) );
    m_statusLabel->setText( text );
    m_statusSet = true;

    QMessageBox box;
    box.setIcon( QMessageBox::Critical );
    QAccessible::updateAccessibility( &box, 0, QAccessible::Alert );
}

void CommandDialog::showCloseButton()
{
    m_buttonBox->button( QDialogButtonBox::Cancel )->setText( tr( "&Close" ) );
    m_buttonBox->button( QDialogButtonBox::Ok )->hide();
}

bool CommandDialog::validate()
{
    foreach ( MarkupTextEdit* edit, findChildren<MarkupTextEdit*>() )
        edit->closePreview();

    bool result = true;
    QWidget* errorWidget = NULL;

    foreach ( InputLineEdit* edit, findChildren<InputLineEdit*>() ) {
        if ( edit->isEnabled() && !edit->validate() ) {
            if ( !errorWidget )
                errorWidget = edit;
            result = false;
        }
    }

    foreach ( InputTextEdit* edit, findChildren<InputTextEdit*>() ) {
        if ( edit->isEnabled() && !edit->validate() ) {
            if ( !errorWidget )
                errorWidget = edit;
            result = false;
        }
    }

    if ( !result )
        showWarning( tr( "Some of the values you entered are incorrect." ) );

    if ( errorWidget ) {
        QWidget* widget = errorWidget;
        QWidget* parent = widget->parentWidget();

        while ( parent != this ) {
            QStackedWidget* stackedWidget = qobject_cast<QStackedWidget*>( parent );
            if ( stackedWidget ) {
                QTabWidget* tabWidget = qobject_cast<QTabWidget*>( stackedWidget->parentWidget() );
                if ( tabWidget )
                    tabWidget->setCurrentWidget( widget );
            }

            QScrollArea* scrollArea = qobject_cast<QScrollArea*>( parent );
            if ( scrollArea )
                scrollArea->ensureWidgetVisible( errorWidget );

            widget = parent;
            parent = widget->parentWidget();
        }

        errorWidget->setFocus();
    }

    return result;
}

void CommandDialog::executeBatch( AbstractBatch* batch, const QString& text )
{
    if ( m_contentLayout )
        setWidgetsEnabled( m_contentLayout, false );

    if ( m_progressBar )
        m_progressBar->setEnabled( true );

    m_buttonBox->button( QDialogButtonBox::Ok )->hide();
    m_buttonBox->button( QDialogButtonBox::Cancel )->setText( tr( "&Abort" ) );

    m_statusPixmap->setPixmap( IconLoader::pixmap( "status-busy" ) );
    m_statusLabel->setText( !text.isEmpty() ? text : tr( "Executing command..." ) );

    connect( batch, SIGNAL( completed( bool ) ), this, SLOT( batchCompleted( bool ) ) );

    batch->setPreventClose( true );

    m_batch = batch;

    commandManager->execute( batch );
}

void CommandDialog::reject()
{
    if ( m_batch )
        commandManager->abort( m_batch );
    else if ( queryClose() )
        QDialog::reject();
}

void CommandDialog::closeEvent( QCloseEvent* e )
{
    if ( !m_batch && queryClose() )
        e->accept();
    else
        e->ignore();
}

void CommandDialog::setQueryCloseEnabled( bool on )
{
    m_queryClose = on;
}

bool CommandDialog::queryClose()
{
    if ( !m_queryClose )
        return true;

    bool modified = false;

    foreach ( InputLineEdit* edit, findChildren<InputLineEdit*>() ) {
        if ( edit->isEnabled() && edit->isModified() )
            modified = true;
    }

    foreach ( InputTextEdit* edit, findChildren<InputTextEdit*>() ) {
        if ( edit->isEnabled() && edit->document()->isModified() )
            modified = true;
    }

    if ( modified ) {
        if ( MessageBox::warning( this, tr( "Warning" ), tr( "The changes have not been submitted and will be lost.\nAre you sure you want to continue?" ),
             QMessageBox::Ok | QMessageBox::Cancel ) != QMessageBox::Ok ) {
            return false;
        }
    }

    return true;
}

void CommandDialog::batchCompleted( bool successful )
{
    if ( successful ) {
        if ( batchSuccessful( m_batch ) ) {
            QDialog::accept();
            m_batch = NULL;
            return;
        }
    } else {
        showError( tr( "Command failed: %1." ).arg( commandManager->errorMessage() ) );
        if ( !batchFailed( m_batch ) ) {
            showCloseButton();
            m_batch = NULL;
            return;
        }
    }

    if ( m_contentLayout )
        setWidgetsEnabled( m_contentLayout, true );

    if ( m_progressBar ) {
        m_progressBar->setEnabled( false );
        setProgress( 0, m_progressText );
    }

    m_buttonBox->button( QDialogButtonBox::Ok )->show();
    m_buttonBox->button( QDialogButtonBox::Cancel )->setText( tr( "&Cancel" ) );

    m_batch = NULL;
}

bool CommandDialog::batchSuccessful( AbstractBatch* /*batch*/ )
{
    return true;
}

bool CommandDialog::batchFailed( AbstractBatch* /*batch*/ )
{
    return true;
}

void CommandDialog::setWidgetsEnabled( QLayoutItem* item, bool enabled )
{
    QLayout* layout = item->layout();
    if ( layout ) {
        for ( int i = 0; i < layout->count(); i++ )
            setWidgetsEnabled( layout->itemAt( i ), enabled );
    }

    QWidget* widget = item->widget();
    if ( widget )
        widget->setEnabled( enabled );
}
