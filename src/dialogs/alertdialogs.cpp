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

#include "alertdialogs.h"

#include "commands/alertsbatch.h"
#include "data/datamanager.h"
#include "data/entities.h"
#include "utils/errorhelper.h"
#include "utils/iconloader.h"
#include "widgets/separatorcombobox.h"

#include <QLayout>
#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>

AddAlertDialog::AddAlertDialog( int folderId, QWidget* parent ) : CommandDialog( parent ),
    m_folderId( folderId ),
    m_viewCombo( NULL ),
    m_emailGroup( NULL )
{
    FolderEntity folder = FolderEntity::find( folderId );

    setWindowTitle( tr( "Add Alert" ) );
    setPrompt( tr( "Create a new alert for folder <b>%1</b>:" ).arg( folder.name() ) );
    setPromptPixmap( IconLoader::pixmap( "alert-new", 22 ) );

    QList<int> used;
    foreach ( const AlertEntity& alert, folder.alerts() )
        used.append( alert.viewId() );

    QList<ViewEntity> personalViews;
    QList<ViewEntity> publicViews;

    foreach ( const ViewEntity& view, folder.type().views() ) {
        if ( view.isPublic() ) {
            if ( !used.contains( view.id() ) )
                publicViews.append( view );
        } else {
            if ( !used.contains( view.id() ) )
                personalViews.append( view );
        }
    }

    if ( used.contains( 0 ) && publicViews.isEmpty() && personalViews.isEmpty() ) {
        showWarning( tr( "There are no more available views to use." ) );
        showCloseButton();
        setContentLayout( NULL, true );
        return;
    }

    QGridLayout* layout = new QGridLayout();

    QLabel* viewLabel = new QLabel( tr( "&View:" ), this );
    layout->addWidget( viewLabel, 0, 0 );

    m_viewCombo = new SeparatorComboBox( this );
    layout->addWidget( m_viewCombo, 0, 1 );

    viewLabel->setBuddy( m_viewCombo );

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

    int emailEnabled = dataManager->setting( "email_enabled" ).toInt();

    if ( emailEnabled != 0 ) {
        QLabel* emailLabel = new QLabel( tr( "Type of emails:" ), this );
        layout->addWidget( emailLabel, 1, 0 );

        m_emailGroup = new QButtonGroup( this );

        QRadioButton* noneButton = new QRadioButton( tr( "&None" ), this );
        m_emailGroup->addButton( noneButton, NoEmail );
        layout->addWidget( noneButton, 1, 1 );

        QRadioButton* immediateButton = new QRadioButton( tr( "&Immediate notifications" ), this );
        m_emailGroup->addButton( immediateButton, ImmediateNotificationEmail );
        layout->addWidget( immediateButton, 2, 1 );

        QRadioButton* summaryButton = new QRadioButton( tr( "&Summary of notifications" ), this );
        m_emailGroup->addButton( summaryButton, SummaryNotificationEmail );
        layout->addWidget( summaryButton, 3, 1 );

        QRadioButton* reportButton = new QRadioButton( tr( "Summary &reports" ), this );
        m_emailGroup->addButton( reportButton, SummaryReportEmail );
        layout->addWidget( reportButton, 4, 1 );
    }

    layout->setColumnStretch( 1, 1 );

    if ( m_emailGroup )
        m_emailGroup->button( NoEmail )->setChecked( true );

    setContentLayout( layout, true );
}

AddAlertDialog::~AddAlertDialog()
{
}

void AddAlertDialog::accept()
{
    int index = m_viewCombo->currentIndex();
    int viewId = m_viewCombo->itemData( index ).toInt();
    int alertEmail = m_emailGroup ? m_emailGroup->checkedId() : 0;

    AlertsBatch* batch = new AlertsBatch();
    batch->addAlert( m_folderId, viewId, (AlertEmail)alertEmail );

    executeBatch( batch );
}

ModifyAlertDialog::ModifyAlertDialog( int alertId, QWidget* parent ) : CommandDialog( parent ),
    m_alertId( alertId )
{
    AlertEntity alert = AlertEntity::find( alertId );
    QString name = alert.viewId() ? alert.view().name() : tr( "All Issues" );
    m_oldAlertEmail = alert.alertEmail();

    setWindowTitle( tr( "Modify Alert" ) );
    setPrompt( tr( "Modify alert <b>%1</b>:" ).arg( name ) );
    setPromptPixmap( IconLoader::pixmap( "edit-modify", 22 ) );

    QGridLayout* layout = new QGridLayout();

    QLabel* emailLabel = new QLabel( tr( "Type of emails:" ), this );
    layout->addWidget( emailLabel, 0, 0 );

    m_emailGroup = new QButtonGroup( this );

    QRadioButton* noneButton = new QRadioButton( tr( "&None" ), this );
    m_emailGroup->addButton( noneButton, NoEmail );
    layout->addWidget( noneButton, 0, 1 );

    QRadioButton* immediateButton = new QRadioButton( tr( "&Immediate notifications" ), this );
    m_emailGroup->addButton( immediateButton, ImmediateNotificationEmail );
    layout->addWidget( immediateButton, 1, 1 );

    QRadioButton* summaryButton = new QRadioButton( tr( "&Summary of notifications" ), this );
    m_emailGroup->addButton( summaryButton, SummaryNotificationEmail );
    layout->addWidget( summaryButton, 2, 1 );

    QRadioButton* reportButton = new QRadioButton( tr( "Summary &reports" ), this );
    m_emailGroup->addButton( reportButton, SummaryReportEmail );
    layout->addWidget( reportButton, 3, 1 );

    layout->setColumnStretch( 1, 1 );

    m_emailGroup->button( m_oldAlertEmail )->setChecked( true );

    setContentLayout( layout, true );
}

ModifyAlertDialog::~ModifyAlertDialog()
{
}

void ModifyAlertDialog::accept()
{
    int alertEmail = m_emailGroup->checkedId();

    if ( alertEmail == m_oldAlertEmail ) {
        QDialog::accept();
        return;
    }

    AlertsBatch* batch = new AlertsBatch();
    batch->modifyAlert( m_alertId, (AlertEmail)alertEmail );

    executeBatch( batch );
}

DeleteAlertDialog::DeleteAlertDialog( int alertId, QWidget* parent ) : CommandDialog( parent ),
    m_alertId( alertId )
{
    AlertEntity alert = AlertEntity::find( alertId );
    QString name = alert.viewId() ? alert.view().name() : tr( "All Issues" );

    setWindowTitle( tr( "Delete Alert" ) );
    setPrompt( tr( "Are you sure you want to delete alert <b>%1</b>?" ).arg( name ) );
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
