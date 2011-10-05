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

#include "settingsdialog.h"

#include "application.h"
#include "commands/commandmanager.h"
#include "data/localsettings.h"
#include "utils/iconloader.h"
#include "widgets/separatorcombobox.h"
#include "widgets/inputlineedit.h"

#include <QLayout>
#include <QLabel>
#include <QTabWidget>
#include <QCheckBox>
#include <QSpinBox>
#include <QIntValidator>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QMessageBox>
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

    QWidget* appearanceTab = new QWidget( m_tabWidget );
    m_tabWidget->addTab( appearanceTab, IconLoader::icon( "appearance" ), tr( "Appearance" ) );

    QGridLayout* appearanceLayout = new QGridLayout( appearanceTab );
    appearanceLayout->setMargin( 15 );

    QLabel* languageLabel = new QLabel( tr( "&Language of user interface:" ), appearanceTab );
    appearanceLayout->addWidget( languageLabel, 0, 0 );

    m_languageComboBox = new SeparatorComboBox( appearanceTab );
    loadLanguages();
    appearanceLayout->addWidget( m_languageComboBox, 0, 1 );

    languageLabel->setBuddy( m_languageComboBox );

    appearanceLayout->setRowStretch( appearanceLayout->rowCount(), 1 );
    appearanceLayout->setColumnStretch( 0, 1 );

    QWidget* behaviorTab = new QWidget( m_tabWidget );
    m_tabWidget->addTab( behaviorTab, IconLoader::icon( "configure" ), tr( "Behavior" ) );

    QGridLayout* behaviorLayout = new QGridLayout( behaviorTab );
    behaviorLayout->setMargin( 15 );

    m_dockCheckBox = new QCheckBox( tr( "&Dock main window in system tray" ), behaviorTab );
    behaviorLayout->addWidget( m_dockCheckBox, 0, 0, 1, 2 );

    connect( m_dockCheckBox, SIGNAL( stateChanged( int ) ), this, SLOT( dockChanged() ) );

    QFrame* separator = new QFrame( behaviorTab );
    separator->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    separator->setFixedHeight( 20 );
    behaviorLayout->addWidget( separator, 1, 0, 1, 2 );

    QLabel* showLabel = new QLabel( tr( "&Show main window at startup:" ), behaviorTab );
    behaviorLayout->addWidget( showLabel, 2, 0 );

    m_showComboBox = new QComboBox( behaviorTab );
    m_showComboBox->addItem( tr( "Never" ) );
    m_showComboBox->addItem( tr( "Automatically" ) );
    m_showComboBox->addItem( tr( "Always" ) );
    behaviorLayout->addWidget( m_showComboBox, 2, 1 );

    showLabel->setBuddy( m_showComboBox );

    QLabel* reconnectLabel = new QLabel( tr( "&Restore last connection at startup:" ), behaviorTab );
    behaviorLayout->addWidget( reconnectLabel, 3, 0 );

    m_reconnectComboBox = new QComboBox( behaviorTab );
    m_reconnectComboBox->addItem( tr( "Never" ) );
    m_reconnectComboBox->addItem( tr( "Automatically" ) );
    m_reconnectComboBox->addItem( tr( "Always" ) );
    behaviorLayout->addWidget( m_reconnectComboBox, 3, 1 );

    reconnectLabel->setBuddy( m_reconnectComboBox );

    QFrame* separator2 = new QFrame( behaviorTab );
    separator2->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    separator2->setFixedHeight( 20 );
    behaviorLayout->addWidget( separator2, 4, 0, 1, 2 );

    QLabel* actionLabel = new QLabel( tr( "De&fault action for attachments:" ), behaviorTab );
    behaviorLayout->addWidget( actionLabel, 5, 0 );

    m_actionComboBox = new QComboBox( behaviorTab );
    m_actionComboBox->addItem( tr( "Ask what to do" ) );
    m_actionComboBox->addItem( tr( "Open" ) );
    m_actionComboBox->addItem( tr( "Save As..." ) );
    behaviorLayout->addWidget( m_actionComboBox, 5, 1 );

    actionLabel->setBuddy( m_actionComboBox );

    QFrame* separator3 = new QFrame( behaviorTab );
    separator3->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    separator3->setFixedHeight( 20 );
    behaviorLayout->addWidget( separator3, 6, 0, 1, 2 );

    m_autoUpdateCheckBox = new QCheckBox( tr( "&Enable automatic checking for latest version of WebIssues" ), behaviorTab );
    behaviorLayout->addWidget( m_autoUpdateCheckBox, 7, 0, 1, 2 );

#if defined( Q_WS_WIN )
    if ( !application->isPortableMode() ) {
        QFrame* separator3 = new QFrame( behaviorTab );
        separator3->setFrameStyle( QFrame::HLine | QFrame::Sunken );
        separator3->setFixedHeight( 20 );
        behaviorLayout->addWidget( separator3, 8, 0, 1, 2 );

        m_autoStartCheckBox = new QCheckBox( tr( "Start &WebIssues when I start Windows" ), behaviorTab );
        behaviorLayout->addWidget( m_autoStartCheckBox, 9, 0, 1, 2 );
    }
#endif

    behaviorLayout->setRowStretch( behaviorLayout->rowCount(), 1 );
    behaviorLayout->setColumnStretch( 0, 1 );

    QWidget* connectionTab = new QWidget( m_tabWidget );
    m_tabWidget->addTab( connectionTab, IconLoader::icon( "connection" ), tr( "Connection" ) );

    QGridLayout* connectionLayout = new QGridLayout( connectionTab );
    connectionLayout->setMargin( 15 );

    QLabel* proxyLabel = new QLabel( tr( "&Proxy used for Internet connections:" ), connectionTab );
    connectionLayout->addWidget( proxyLabel, 0, 0, 1, 2 );

    m_proxyCombo = new QComboBox( connectionTab );
#if !defined( NO_DEFAULT_PROXY )
    m_proxyCombo->addItem( tr( "System Default" ), (int)QNetworkProxy::DefaultProxy );
#endif
    m_proxyCombo->addItem( tr( "No Proxy" ), (int)QNetworkProxy::NoProxy );
    m_proxyCombo->addItem( tr( "HTTP Proxy" ), (int)QNetworkProxy::HttpProxy );
    m_proxyCombo->addItem( tr( "SOCKS5 Proxy" ), (int)QNetworkProxy::Socks5Proxy );

    connectionLayout->addWidget( m_proxyCombo, 0, 2 );

    connect( m_proxyCombo, SIGNAL( currentIndexChanged( int ) ), this, SLOT( proxyChanged() ) );

    proxyLabel->setBuddy( m_proxyCombo );

    QLabel* hostLabel = new QLabel( tr( "&Host name:" ), connectionTab );
    connectionLayout->addWidget( hostLabel, 1, 0 );

    m_proxyHostEdit = new InputLineEdit( connectionTab );
    m_proxyHostEdit->setMaxLength( 64 );
    m_proxyHostEdit->setRequired( true );
    connectionLayout->addWidget( m_proxyHostEdit, 1, 1, 1, 2 );

    hostLabel->setBuddy( m_proxyHostEdit );

    QLabel* portLabel = new QLabel( tr( "Port &number:" ), connectionTab );
    connectionLayout->addWidget( portLabel, 2, 0, 1, 2 );

    m_proxyPortSpin = new QSpinBox( connectionTab );
    m_proxyPortSpin->setRange( 1, 65535 );
    connectionLayout->addWidget( m_proxyPortSpin, 2, 2 );

    portLabel->setBuddy( m_proxyPortSpin );

    connectionLayout->setRowStretch( connectionLayout->rowCount(), 1 );
    connectionLayout->setColumnStretch( 1, 1 );

    QWidget* advancedTab = new QWidget( m_tabWidget );
    m_tabWidget->addTab( advancedTab, IconLoader::icon( "gear" ), tr( "Advanced" ) );

    QGridLayout* advancedLayout = new QGridLayout( advancedTab );
    advancedLayout->setMargin( 15 );

    QLabel* intervalLabel = new QLabel( tr( "I&nterval of periodic data updates:" ), advancedTab );
    advancedLayout->addWidget( intervalLabel, 0, 0 );

    m_intervalSpin = new QSpinBox( advancedTab );
    m_intervalSpin->setRange( 1, 120 );
    m_intervalSpin->setSuffix( tr( " min" ) );
    advancedLayout->addWidget( m_intervalSpin, 0, 1 );

    intervalLabel->setBuddy( m_intervalSpin );

    QFrame* separator4 = new QFrame( advancedTab );
    separator4->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    separator4->setFixedHeight( 20 );
    advancedLayout->addWidget( separator4, 1, 0, 1, 2 );

    QLabel* cacheLabel = new QLabel( tr( "&Maximum size of attachments cache:" ), advancedTab );
    advancedLayout->addWidget( cacheLabel, 2, 0 );

    m_cacheSpin = new QSpinBox( advancedTab );
    m_cacheSpin->setRange( 1, 200 );
    m_cacheSpin->setSuffix( tr( " MB" ) );
    advancedLayout->addWidget( m_cacheSpin, 2, 1 );

    cacheLabel->setBuddy( m_cacheSpin );

    advancedLayout->setRowStretch( advancedLayout->rowCount(), 1 );
    advancedLayout->setColumnStretch( 0, 1 );

    setContentLayout( layout, false );

    QPushButton* applyButton = buttonBox()->addButton( tr( "&Apply" ), QDialogButtonBox::ApplyRole );
    connect( applyButton, SIGNAL( clicked() ), this, SLOT( apply() ) );

    LocalSettings* settings = application->applicationSettings();

    m_dockCheckBox->setChecked( settings->value( "Docked" ).toBool() );
    m_showComboBox->setCurrentIndex( settings->value( "ShowAtStartup" ).toInt() );
    m_reconnectComboBox->setCurrentIndex( settings->value( "ConnectAtStartup" ).toInt() );

    int index = m_languageComboBox->findData( settings->value( "Language" ).toString() );
    if ( index >= 2 )
        m_languageComboBox->setCurrentIndex( index );

    m_intervalSpin->setValue( settings->value( "UpdateInterval" ).toInt() );
    m_actionComboBox->setCurrentIndex( settings->value( "DefaultAttachmentAction" ).toInt() );
    m_cacheSpin->setValue( settings->value( "AttachmentsCacheSize" ).toInt() );

    m_autoUpdateCheckBox->setChecked( settings->value( "AutoUpdate" ).toBool() );

    QNetworkProxy::ProxyType proxyType = (QNetworkProxy::ProxyType)settings->value( "ProxyType" ).toInt();
    m_proxyCombo->setCurrentIndex( m_proxyCombo->findData( (int)proxyType ) );
    if ( proxyType != QNetworkProxy::DefaultProxy && proxyType != QNetworkProxy::NoProxy ) {
        m_proxyHostEdit->setInputValue( settings->value( "ProxyHost" ).toString() );
        m_proxyPortSpin->setValue( settings->value( "ProxyPort" ).toInt() );
    } else {
        m_proxyPortSpin->setValue( 80 );
    }

#if defined( Q_WS_WIN )
    if ( !application->isPortableMode() )
        m_autoStartCheckBox->setChecked( settings->value( "AutoStart" ).toBool() );
#endif

    dockChanged();
    proxyChanged();

    resize( 450, 350 );
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

    settings->setValue( "Docked", m_dockCheckBox->isChecked() );
    settings->setValue( "ShowAtStartup", m_showComboBox->currentIndex() );
    settings->setValue( "ConnectAtStartup", m_reconnectComboBox->currentIndex() );

    QString language = m_languageComboBox->itemData( m_languageComboBox->currentIndex() ).toString();
    if ( language != settings->value( "Language" ).toString() )
        QMessageBox::warning( this, tr( "Warning" ), tr( "Language settings will be applied when the application is restarted." ) );
    settings->setValue( "Language", language );

    settings->setValue( "UpdateInterval", m_intervalSpin->value() );
    settings->setValue( "DefaultAttachmentAction", m_actionComboBox->currentIndex() );
    settings->setValue( "AttachmentsCacheSize", m_cacheSpin->value() );

    settings->setValue( "AutoUpdate", m_autoUpdateCheckBox->isChecked() );

    QNetworkProxy::ProxyType proxyType = (QNetworkProxy::ProxyType)m_proxyCombo->itemData( m_proxyCombo->currentIndex() ).toInt();
    QString proxyHost = m_proxyHostEdit->inputValue();
    int proxyPort = m_proxyPortSpin->value();

    settings->setValue( "ProxyType", (int)proxyType );
    settings->setValue( "ProxyHost", proxyHost );
    settings->setValue( "ProxyPort", proxyPort );

#if defined( Q_WS_WIN )
    if ( !application->isPortableMode() )
        settings->setValue( "AutoStart", m_autoStartCheckBox->isChecked() );
#endif

    settings->save();

    return true;
}

void SettingsDialog::dockChanged()
{
    if ( m_dockCheckBox->isChecked() ) {
        LocalSettings* settings = application->applicationSettings();
        m_showComboBox->setCurrentIndex( settings->value( "ShowAtStartup" ).toInt() );
        m_showComboBox->setEnabled( true );
    } else {
        m_showComboBox->setCurrentIndex( (int)RestoreAlways );
        m_showComboBox->setEnabled( false );
    }
}

void SettingsDialog::proxyChanged()
{
    QNetworkProxy::ProxyType type = (QNetworkProxy::ProxyType)m_proxyCombo->itemData( m_proxyCombo->currentIndex() ).toInt();
    m_proxyHostEdit->setEnabled( type != QNetworkProxy::DefaultProxy && type != QNetworkProxy::NoProxy );
    m_proxyPortSpin->setEnabled( type != QNetworkProxy::DefaultProxy && type != QNetworkProxy::NoProxy );
}

void SettingsDialog::loadLanguages()
{
    m_languageComboBox->addItem( tr( "System Default" ) );
    m_languageComboBox->addSeparator();

    QMap<QString, QString> languages = application->languages();

    for ( QMap<QString, QString>::iterator it = languages.begin(); it != languages.end(); it++ )
        m_languageComboBox->addItem( it.value(), it.key() );
}
