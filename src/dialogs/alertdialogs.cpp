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

#include "alertdialogs.h"

#include "commands/alertsbatch.h"
#include "data/entities.h"
#include "utils/errorhelper.h"
#include "utils/iconloader.h"
#include "widgets/separatorcombobox.h"

#include <QLayout>
#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>

AlertDialog::AlertDialog( QWidget* parent ) : CommandDialog( parent ),
    m_viewCombo( NULL ),
    m_emailGroup( NULL )
{
}

AlertDialog::~AlertDialog()
{
}

bool AlertDialog::initialize( Flags flags, int typeId, const QList<int>& used )
{
    QGridLayout* layout = new QGridLayout();
    int row = 0;

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

        QLabel* viewLabel = new QLabel( tr( "&View:" ), this );
        layout->addWidget( viewLabel, row, 0 );

        m_viewCombo = new SeparatorComboBox( this );
        layout->addWidget( m_viewCombo, row++, 1 );

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
    }

    int emailEnabled = dataManager->setting( "email_enabled" ).toInt();

    if ( emailEnabled != 0 ) {
        QLabel* emailLabel = new QLabel( tr( "Type of emails:" ), this );
        layout->addWidget( emailLabel, row, 0 );

        m_emailGroup = new QButtonGroup( this );

        QRadioButton* noneButton = new QRadioButton( tr( "&None" ), this );
        m_emailGroup->addButton( noneButton, NoEmail );
        layout->addWidget( noneButton, row++, 1 );

        QRadioButton* immediateButton = new QRadioButton( tr( "&Immediate notifications" ), this );
        m_emailGroup->addButton( immediateButton, ImmediateNotificationEmail );
        layout->addWidget( immediateButton, row++, 1 );

        QRadioButton* summaryButton = new QRadioButton( tr( "&Summary of notifications" ), this );
        m_emailGroup->addButton( summaryButton, SummaryNotificationEmail );
        layout->addWidget( summaryButton, row++, 1 );

        QRadioButton* reportButton = new QRadioButton( tr( "Summary &reports" ), this );
        m_emailGroup->addButton( reportButton, SummaryReportEmail );
        layout->addWidget( reportButton, row++, 1 );
    }

    layout->setColumnStretch( 1, 1 );

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
    if ( m_emailGroup )
        m_emailGroup->button( email )->setChecked( true );
}

AlertEmail AlertDialog::alertEmail() const
{
    if ( m_emailGroup )
        return (AlertEmail)m_emailGroup->checkedId();
    else
        return NoEmail;
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

    if ( !initialize( WithView | ( isPublic ? OnlyPublic : (Flags)0 ), folder.typeId(), used ) )
        return;

    setAlertEmail( NoEmail );
}

AddAlertDialog::~AddAlertDialog()
{
}

void AddAlertDialog::accept()
{
    AlertsBatch* batch = new AlertsBatch();
    batch->addAlert( m_folderId, viewId(), alertEmail(), m_isPublic );

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

    if ( !initialize( WithView | ( isPublic ? OnlyPublic : (Flags)0 ), typeId, used ) )
        return;

    setAlertEmail( NoEmail );
}

AddGlobalAlertDialog::~AddGlobalAlertDialog()
{
}

void AddGlobalAlertDialog::accept()
{
    AlertsBatch* batch = new AlertsBatch();
    batch->addGlobalAlert( m_typeId, viewId(), alertEmail(), m_isPublic );

    executeBatch( batch );
}

ModifyAlertDialog::ModifyAlertDialog( int alertId, QWidget* parent ) : AlertDialog( parent ),
    m_alertId( alertId )
{
    AlertEntity alert = AlertEntity::find( alertId );
    QString name = alert.viewId() ? alert.view().name() : tr( "All Issues" );
    m_oldAlertEmail = alert.alertEmail();

    setWindowTitle( tr( "Modify Alert" ) );
    if ( alert.isPublic() )
        setPrompt( tr( "Modify public alert <b>%1</b>:" ).arg( name ) );
    else
        setPrompt( tr( "Modify your personal alert <b>%1</b>:" ).arg( name ) );
    setPromptPixmap( IconLoader::pixmap( "edit-modify", 22 ) );

    initialize();

    setAlertEmail( m_oldAlertEmail );
}

ModifyAlertDialog::~ModifyAlertDialog()
{
}

void ModifyAlertDialog::accept()
{
    if ( alertEmail() == m_oldAlertEmail ) {
        QDialog::accept();
        return;
    }

    AlertsBatch* batch = new AlertsBatch();
    batch->modifyAlert( m_alertId, alertEmail() );

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
