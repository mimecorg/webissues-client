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

#include "settingsdialog.h"

#include "application.h"
#include "data/localsettings.h"
#include "dialogs/messagebox.h"
#include "utils/networkproxyfactory.h"
#include "utils/iconloader.h"

#include <QDialogButtonBox>
#include <QPushButton>
#include <QNetworkProxy>

SettingsDialog::SettingsDialog( QWidget* parent ) : CommandDialog( parent )
{
    setWindowTitle( tr( "WebIssues Settings" ) );
    setPrompt( tr( "Configure settings of the WebIssues Desktop Client:" ) );
    setPromptPixmap( IconLoader::pixmap( "configure", 22 ) );

    showInfo( tr( "Edit program settings." ) );

    QVBoxLayout* layout = new QVBoxLayout();

    m_tabWidget = new QTabWidget( this );
    layout->addWidget( m_tabWidget );

    m_ui.setupUi( m_tabWidget );

    m_tabWidget->setTabIcon( 0, IconLoader::icon( "configure" ) );
    m_tabWidget->setTabIcon( 1, IconLoader::icon( "gear" ) );

    m_ui.languageComboBox->addSeparator();

    QMap<QString, QString> languages = application->languages();

    for ( QMap<QString, QString>::iterator it = languages.begin(); it != languages.end(); it++ )
        m_ui.languageComboBox->addItem( it.value(), it.key() );

    for ( int i = 100; i <= 150; i += 25 )
        m_ui.textSizeComboBox->addItem( QString( "%1%" ).arg( i ), i );

#if defined( Q_OS_WIN )
    if ( application->isPortableMode() )
        m_ui.autoStartCheckBox->hide();
#else
    m_ui.autoStartCheckBox->hide();
#endif

    m_ui.hostLineEdit->setMaxLength( 64 );
    m_ui.hostLineEdit->setRequired( true );

    m_ui.excludeLineEdit->setMultiSelect( true );

    QMetaObject::connectSlotsByName( this );

    setContentLayout( layout, true );

    QPushButton* applyButton = buttonBox()->addButton( tr( "&Apply" ), QDialogButtonBox::ApplyRole );
    connect( applyButton, SIGNAL( clicked() ), this, SLOT( apply() ) );

    LocalSettings* settings = application->applicationSettings();

    int index = m_ui.languageComboBox->findData( settings->value( "Language" ) );
    m_ui.languageComboBox->setCurrentIndex( index >= 2 ? index : 0 );

    m_ui.dockCheckBox->setChecked( settings->value( "Docked" ).toBool() );
    m_ui.showComboBox->setCurrentIndex( settings->value( "ShowAtStartup" ).toInt() );
    m_ui.reconnectComboBox->setCurrentIndex( settings->value( "ConnectAtStartup" ).toInt() );

    index = m_ui.textSizeComboBox->findData( settings->value( "TextSizeMultiplier" ) );
    m_ui.textSizeComboBox->setCurrentIndex( index > 0 ? index : 0 );

#if defined( Q_OS_WIN )
    if ( !application->isPortableMode() )
        m_ui.autoStartCheckBox->setChecked( settings->value( "AutoStart" ).toBool() );
#endif

    m_ui.autoUpdateCheckBox->setChecked( settings->value( "AutoUpdate" ).toBool() );

    m_ui.attachmentsComboBox->setCurrentIndex( settings->value( "DefaultAttachmentAction" ).toInt() );

    m_ui.foldersSpinBox->setValue( settings->value( "FolderUpdateInterval" ).toInt() );
    m_ui.fullSpinBox->setValue( settings->value( "UpdateInterval" ).toInt() );

    QNetworkProxy::ProxyType proxyType = (QNetworkProxy::ProxyType)settings->value( "ProxyType" ).toInt();
    if ( proxyType == QNetworkProxy::NoProxy || proxyType == QNetworkProxy::DefaultProxy ) {
        m_ui.customProxyCheckBox->setChecked( false );
    } else {
        m_ui.customProxyCheckBox->setChecked( true );
        m_ui.hostLineEdit->setInputValue( settings->value( "ProxyHost" ).toString() );
        m_ui.portSpinBox->setValue( settings->value( "ProxyPort" ).toInt() );
        m_ui.proxyTypeComboBox->setCurrentIndex( proxyType == QNetworkProxy::Socks5Proxy ? 1 : 0 );
        m_ui.excludeLineEdit->setInputValue( settings->value( "ProxyExclude" ).toStringList().join( ", " ) );
    }

    resize( 500, 500 );
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::accept()
{
    if ( apply() )
        QDialog::accept();
}

bool SettingsDialog::apply()
{
    if ( !validate() )
        return false;

    LocalSettings* settings = application->applicationSettings();

    QString language = m_ui.languageComboBox->itemData( m_ui.languageComboBox->currentIndex() ).toString();
    if ( language != settings->value( "Language" ).toString() )
        MessageBox::warning( this, tr( "Warning" ), tr( "Language settings will be applied when the application is restarted." ) );
    settings->setValue( "Language", language );

    settings->setValue( "Docked", m_ui.dockCheckBox->isChecked() );
    settings->setValue( "ShowAtStartup", m_ui.showComboBox->currentIndex() );
    settings->setValue( "ConnectAtStartup", m_ui.reconnectComboBox->currentIndex() );

    settings->setValue( "TextSizeMultiplier", m_ui.textSizeComboBox->itemData( m_ui.textSizeComboBox->currentIndex() ) );

#if defined( Q_OS_WIN )
    if ( !application->isPortableMode() )
        settings->setValue( "AutoStart", m_ui.autoStartCheckBox->isChecked() );
#endif

    settings->setValue( "AutoUpdate", m_ui.autoUpdateCheckBox->isChecked() );

    settings->setValue( "DefaultAttachmentAction", m_ui.attachmentsComboBox->currentIndex() );

    settings->setValue( "FolderUpdateInterval", m_ui.foldersSpinBox->value() );
    settings->setValue( "UpdateInterval", m_ui.fullSpinBox->value() );

    if ( !m_ui.customProxyCheckBox->isChecked() ) {
        settings->setValue( "ProxyType", (int)QNetworkProxy::NoProxy );
    } else {
        settings->setValue( "ProxyHost", m_ui.hostLineEdit->inputValue() );
        settings->setValue( "ProxyPort", m_ui.portSpinBox->value() );
        settings->setValue( "ProxyType", (int)( m_ui.proxyTypeComboBox->currentIndex() == 1 ? QNetworkProxy::Socks5Proxy : QNetworkProxy::HttpProxy ) );
        settings->setValue( "ProxyExclude", m_ui.excludeLineEdit->inputValue().split( ", " ) );
    }

    settings->save();

    return true;
}

void SettingsDialog::on_dockCheckBox_toggled( bool on )
{
    if ( on ) {
        LocalSettings* settings = application->applicationSettings();
        m_ui.showComboBox->setCurrentIndex( settings->value( "ShowAtStartup" ).toInt() );
    } else {
        m_ui.showComboBox->setCurrentIndex( (int)RestoreAlways );
    }
}
