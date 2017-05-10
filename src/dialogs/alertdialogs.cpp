/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2017 WebIssues Team
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

#include "alertdialogs.h"

#include "commands/alertsbatch.h"
#include "data/entities.h"
#include "utils/errorhelper.h"
#include "utils/iconloader.h"
#include "widgets/separatorcombobox.h"
#include "widgets/schedulewidget.h"

#include <QLayout>
#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>
#include <QGroupBox>

AlertDialog::AlertDialog( QWidget* parent ) : CommandDialog( parent ),
    m_viewCombo( NULL ),
    m_emailGroup( NULL ),
    m_scheduleWidget( NULL )
{
}

AlertDialog::~AlertDialog()
{
}

bool AlertDialog::initialize( Flags flags, int typeId, const QList<int>& used )
{
    QVBoxLayout* layout = new QVBoxLayout();

    if ( flags.testFlag( WithView ) ) {
        QList<ViewEntity> personalViews;
        QList<ViewEntity> publicViews;

        TypeEntity type = TypeEntity::find( typeId );

        foreach ( const ViewEntity& view, type.views() ) {
            if ( view.isPublic() ) {
                if ( !used.contains( view.id() ) )
                    publicViews.append( view );
            } else if ( !flags.testFlag( OnlyPublic ) ) {
                if ( !used.contains( view.id() ) )
                    personalViews.append( view );
            }
        }

        if ( used.contains( 0 ) && publicViews.isEmpty() && personalViews.isEmpty() ) {
            showWarning( tr( "There are no more available views to use." ) );
            showCloseButton();
            setContentLayout( NULL, true );
            return false;
        }

        QHBoxLayout* viewLayout = new QHBoxLayout();
        layout->addLayout( viewLayout );

        QLabel* viewLabel = new QLabel( tr( "&View:" ), this );
        viewLayout->addWidget( viewLabel );

        m_viewCombo = new SeparatorComboBox( this );
        m_viewCombo->setFixedWidth( 200 );
        viewLayout->addWidget( m_viewCombo );

        viewLabel->setBuddy( m_viewCombo );

        viewLayout->addStretch( 1 );

        bool separator = false;
        bool select = false;

        if ( !used.contains( 0 ) ) {
            m_viewCombo->addItem( tr( "All Issues" ), 0 );
            separator = true;
            select = true;
        }

        if ( !personalViews.isEmpty() ) {
            if ( separator )
                m_viewCombo->addSeparator();
            m_viewCombo->addParentItem( tr( "Personal Views" ) );
            separator = true;

            foreach ( const ViewEntity& view, personalViews ) {
                m_viewCombo->addChildItem( view.name(), view.id() );
                if ( !select ) {
                    m_viewCombo->setCurrentIndex( m_viewCombo->count() - 1 );
                    select = true;
                }
            }
        }

        if ( !publicViews.isEmpty() ) {
            if ( separator )
                m_viewCombo->addSeparator();
            m_viewCombo->addParentItem( tr( "Public Views" ) );

            foreach ( const ViewEntity& view, publicViews ) {
                m_viewCombo->addChildItem( view.name(), view.id() );
                if ( !select ) {
                    m_viewCombo->setCurrentIndex( m_viewCombo->count() - 1 );
                    select = true;
                }
            }
        }
    }

    int emailEnabled = dataManager->setting( "email_enabled" ).toInt();

    if ( emailEnabled != 0 ) {
        QGroupBox* emailBox = new QGroupBox( tr( "Email Type" ), this );
        QVBoxLayout* emailLayout = new QVBoxLayout( emailBox );

        QLabel* emailLabel = new QLabel( tr( "Send the following type of emails for this alert:" ), emailBox );
        emailLayout->addWidget( emailLabel );

        m_emailGroup = new QButtonGroup( this );

        QRadioButton* noneButton = new QRadioButton( tr( "&None" ), emailBox );
        m_emailGroup->addButton( noneButton, NoEmail );
        emailLayout->addWidget( noneButton );

        QRadioButton* immediateButton = new QRadioButton( tr( "&Immediate notifications" ), emailBox );
        m_emailGroup->addButton( immediateButton, ImmediateNotificationEmail );
        emailLayout->addWidget( immediateButton );

        QRadioButton* summaryButton = new QRadioButton( tr( "&Summary of notifications" ), emailBox );
        m_emailGroup->addButton( summaryButton, SummaryNotificationEmail );
        emailLayout->addWidget( summaryButton );

        QRadioButton* reportButton = new QRadioButton( tr( "Summary &reports" ), emailBox );
        m_emailGroup->addButton( reportButton, SummaryReportEmail );
        emailLayout->addWidget( reportButton );

        if ( flags.testFlag( CheckEmail ) && dataManager->preference( "email" ).isEmpty() ) {
            emailLayout->addSpacing( 5 );

            QHBoxLayout* warningLayout = new QHBoxLayout();

            QLabel* warningIcon = new QLabel( emailBox );
            warningIcon->setPixmap( IconLoader::pixmap( "status-warning" ) );
            warningLayout->addWidget( warningIcon );

            QLabel* warningLabel = new QLabel( tr( "You will not receive any emails until you enter an email address in your preferences." ), emailBox );
            warningLabel->setWordWrap( true );
            warningLayout->addWidget( warningLabel, 1 );

            emailLayout->addLayout( warningLayout );
        }

        layout->addWidget( emailBox );

        QGroupBox* scheduleBox = new QGroupBox( tr( "Summary Schedule" ), this );
        QVBoxLayout* scheduleLayout = new QVBoxLayout( scheduleBox );

        m_scheduleWidget = new ScheduleWidget( scheduleBox );
        scheduleLayout->addWidget( m_scheduleWidget );

        layout->addWidget( scheduleBox );

        connect( m_emailGroup, SIGNAL( buttonClicked( int ) ), this, SLOT( emailButtonClicked() ) );
    }

    setContentLayout( layout, true );

    return true;
}

int AlertDialog::viewId() const
{
    int index = m_viewCombo->currentIndex();
    return m_viewCombo->itemData( index ).toInt();
}

void AlertDialog::setAlertEmail( AlertEmail email )
{
    if ( m_emailGroup ) {
        m_emailGroup->button( email )->setChecked( true );

        m_scheduleWidget->setEnabled( email == SummaryNotificationEmail || email == SummaryReportEmail );
    }
}

AlertEmail AlertDialog::alertEmail() const
{
    if ( m_emailGroup )
        return (AlertEmail)m_emailGroup->checkedId();
    else
        return NoEmail;
}

void AlertDialog::setSummaryDays( const QString& days )
{
    if ( m_scheduleWidget )
        m_scheduleWidget->setDays( days );
}

QString AlertDialog::summaryDays() const
{
    AlertEmail email = alertEmail();

    if ( email == SummaryNotificationEmail || email == SummaryReportEmail )
        return m_scheduleWidget->days();

    return QString();
}

void AlertDialog::setSummaryHours( const QString& hours )
{
    if ( m_scheduleWidget )
        m_scheduleWidget->setHours( hours );
}

QString AlertDialog::summaryHours() const
{
    AlertEmail email = alertEmail();

    if ( email == SummaryNotificationEmail || email == SummaryReportEmail )
        return m_scheduleWidget->hours();

    return QString();
}

bool AlertDialog::validateSummary()
{
    AlertEmail email = alertEmail();

    if ( email == SummaryNotificationEmail || email == SummaryReportEmail ) {
        if ( m_scheduleWidget->days().isEmpty() ) {
            showWarning( tr( "No days selected" ) );
            return false;
        }
        if ( m_scheduleWidget->hours().isEmpty() ) {
            showWarning( tr( "No hours selected" ) );
            return false;
        }
    }

    return true;
}

void AlertDialog::emailButtonClicked()
{
    AlertEmail email = alertEmail();

    m_scheduleWidget->setEnabled( email == SummaryNotificationEmail || email == SummaryReportEmail );
}

AddAlertDialog::AddAlertDialog( int folderId, bool isPublic, QWidget* parent ) : AlertDialog( parent ),
    m_folderId( folderId ),
    m_isPublic( isPublic )
{
    FolderEntity folder = FolderEntity::find( folderId );

    if ( isPublic ) {
        setWindowTitle( tr( "Add Public Alert" ) );
        setPrompt( tr( "Create a new public alert for folder <b>%1</b>:" ).arg( folder.name() ) );
        setPromptPixmap( IconLoader::overlayedPixmap( "alert-new", "overlay-public", 22 ) );
    } else {
        setWindowTitle( tr( "Add Personal Alert" ) );
        setPrompt( tr( "Create a new personal alert for folder <b>%1</b>:" ).arg( folder.name() ) );
        setPromptPixmap( IconLoader::pixmap( "alert-new", 22 ) );
    }

    QList<int> used;
    foreach ( const AlertEntity& alert, folder.alerts() ) {
        if ( !isPublic || alert.isPublic() )
            used.append( alert.viewId() );
    }

    if ( !initialize( WithView | ( isPublic ? OnlyPublic : CheckEmail ), folder.typeId(), used ) )
        return;

    setAlertEmail( NoEmail );
}

AddAlertDialog::~AddAlertDialog()
{
}

void AddAlertDialog::accept()
{
    if ( !validateSummary() )
        return;

    AlertsBatch* batch = new AlertsBatch();
    batch->addAlert( m_folderId, viewId(), alertEmail(), summaryDays(), summaryHours(), m_isPublic );

    executeBatch( batch );
}

AddGlobalAlertDialog::AddGlobalAlertDialog( int typeId, bool isPublic, QWidget* parent ) : AlertDialog( parent ),
    m_typeId( typeId ),
    m_isPublic( isPublic )
{
    TypeEntity type = TypeEntity::find( typeId );

    if ( isPublic ) {
        setWindowTitle( tr( "Add Public Alert" ) );
        setPrompt( tr( "Create a new public alert for type <b>%1</b>:" ).arg( type.name() ) );
        setPromptPixmap( IconLoader::overlayedPixmap( "alert-new", "overlay-public", 22 ) );
    } else {
        setWindowTitle( tr( "Add Personal Alert" ) );
        setPrompt( tr( "Create a new personal alert for type <b>%1</b>:" ).arg( type.name() ) );
        setPromptPixmap( IconLoader::pixmap( "alert-new", 22 ) );
    }

    QList<int> used;
    foreach ( const AlertEntity& alert, type.alerts() ) {
        if ( !isPublic || alert.isPublic() )
            used.append( alert.viewId() );
    }

    if ( !initialize( WithView | ( isPublic ? OnlyPublic : CheckEmail ), typeId, used ) )
        return;

    setAlertEmail( NoEmail );
}

AddGlobalAlertDialog::~AddGlobalAlertDialog()
{
}

void AddGlobalAlertDialog::accept()
{
    if ( !validateSummary() )
        return;

    AlertsBatch* batch = new AlertsBatch();
    batch->addGlobalAlert( m_typeId, viewId(), alertEmail(), summaryDays(), summaryHours(), m_isPublic );

    executeBatch( batch );
}

ModifyAlertDialog::ModifyAlertDialog( int alertId, QWidget* parent ) : AlertDialog( parent ),
    m_alertId( alertId )
{
    AlertEntity alert = AlertEntity::find( alertId );
    QString name = alert.viewId() ? alert.view().name() : tr( "All Issues" );
    m_oldAlertEmail = alert.alertEmail();
    m_oldSummaryDays = alert.summaryDays();
    m_oldSummaryHours = alert.summaryHours();

    setWindowTitle( tr( "Modify Alert" ) );
    if ( alert.isPublic() )
        setPrompt( tr( "Modify public alert <b>%1</b>:" ).arg( name ) );
    else
        setPrompt( tr( "Modify your personal alert <b>%1</b>:" ).arg( name ) );
    setPromptPixmap( IconLoader::pixmap( "edit-modify", 22 ) );

    initialize( alert.isPublic() ? (Flags)0 : CheckEmail );

    setAlertEmail( m_oldAlertEmail );
    setSummaryDays( m_oldSummaryDays );
    setSummaryHours( m_oldSummaryHours );
}

ModifyAlertDialog::~ModifyAlertDialog()
{
}

void ModifyAlertDialog::accept()
{
    if ( !validateSummary() )
        return;

    if ( alertEmail() == m_oldAlertEmail && summaryDays() == m_oldSummaryDays && summaryHours() == m_oldSummaryHours ) {
        QDialog::accept();
        return;
    }

    AlertsBatch* batch = new AlertsBatch();
    batch->modifyAlert( m_alertId, alertEmail(), summaryDays(), summaryHours() );

    executeBatch( batch );
}

DeleteAlertDialog::DeleteAlertDialog( int alertId, QWidget* parent ) : CommandDialog( parent ),
    m_alertId( alertId )
{
    AlertEntity alert = AlertEntity::find( alertId );
    QString name = alert.viewId() ? alert.view().name() : tr( "All Issues" );

    setWindowTitle( tr( "Delete Alert" ) );
    if ( alert.isPublic() )
        setPrompt( tr( "Are you sure you want to delete public alert <b>%1</b>?" ).arg( name ) );
    else
        setPrompt( tr( "Are you sure you want to delete your personal alert <b>%1</b>?" ).arg( name ) );
    setPromptPixmap( IconLoader::pixmap( "edit-delete", 22 ) );

    setContentLayout( NULL, true );
}

DeleteAlertDialog::~DeleteAlertDialog()
{
}

void DeleteAlertDialog::accept()
{
    AlertsBatch* batch = new AlertsBatch();
    batch->deleteAlert( m_alertId );

    executeBatch( batch );
}
