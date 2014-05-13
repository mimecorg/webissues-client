/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2014 WebIssues Team
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

#include "preferencesdialog.h"

#include "application.h"
#include "commands/updatebatch.h"
#include "commands/preferencesbatch.h"
#include "data/datamanager.h"
#include "data/entities.h"
#include "models/issuedetailsgenerator.h"
#include "utils/validator.h"
#include "utils/formatter.h"
#include "utils/iconloader.h"
#include "utils/definitioninfo.h"
#include "widgets/separatorcombobox.h"
#include "widgets/inputlineedit.h"

#include <QLayout>
#include <QLabel>
#include <QTabWidget>
#include <QPushButton>
#include <QCheckBox>
#include <QButtonGroup>
#include <QTimer>
#include <QDateTime>
#include <QGroupBox>

static QString formatTimeZone( const QString& zone )
{
    QString name = zone;
    name.replace( QChar( '_' ), QChar( ' ' ) );
    name.replace( QLatin1String( "/" ), QLatin1String( " / " ) );
    name.replace( QLatin1String( "St " ), QLatin1String( "St. " ) );
    return name;
}

PreferencesDialog::PreferencesDialog( int userId, QWidget* parent ) : CommandDialog( parent ),
    m_userId( userId ),
    m_projectPageComboBox( NULL ),
    m_emailEdit( NULL ),
    m_detailsCheckBox( NULL ),
    m_noReadCheckBox( NULL )
{
    UserEntity user = UserEntity::find( userId );

    setWindowTitle( tr( "User Preferences" ) );
    if ( userId == dataManager->currentUserId() )
        setPrompt( tr( "Configure your user preferences:" ) );
    else
        setPrompt( tr( "Configure preferences for user <b>%1</b>:" ).arg( user.name() ) );
    setPromptPixmap( IconLoader::pixmap( "preferences", 22 ) );

    showInfo( tr( "Edit user preferences." ) );

    QVBoxLayout* layout = new QVBoxLayout();

    m_tabWidget = new QTabWidget( this );
    layout->addWidget( m_tabWidget );

    QWidget* regionalTab = new QWidget( m_tabWidget );
    m_tabWidget->addTab( regionalTab, IconLoader::icon( "web" ), tr( "Regional" ) );

    QVBoxLayout* regionalTabLayout = new QVBoxLayout( regionalTab );

    QGroupBox* regionalGroup = new QGroupBox( tr( "Regional Options" ), regionalTab );
    QGridLayout* regionalLayout = new QGridLayout( regionalGroup );
    regionalTabLayout->addWidget( regionalGroup );

    QLabel* noteRegionalLabel = new QLabel( tr( "The following settings only affect the Web Client and email notifications.\n"
        "The Desktop Client will always use the language configured in program settings and the local time zone." ), regionalGroup );
    regionalLayout->addWidget( noteRegionalLabel, 0, 0, 1, 3 );

    QLabel* languageLabel = new QLabel( tr( "&Language:" ), regionalGroup );
    regionalLayout->addWidget( languageLabel, 1, 0 );

    m_languageComboBox = new SeparatorComboBox( regionalGroup );
    regionalLayout->addWidget( m_languageComboBox, 1, 1 );

    languageLabel->setBuddy( m_languageComboBox );

    QLabel* timeZoneLabel = new QLabel( tr( "Time &zone:" ), regionalGroup );
    regionalLayout->addWidget( timeZoneLabel, 2, 0 );

    m_timeZoneComboBox = new SeparatorComboBox( regionalGroup );
    m_timeZoneComboBox->setMaxVisibleItems( 20 );
    regionalLayout->addWidget( m_timeZoneComboBox, 2, 1 );

    timeZoneLabel->setBuddy( m_timeZoneComboBox );

    regionalLayout->setColumnStretch( 2, 1 );

    QGroupBox* formatsGroup = new QGroupBox( tr( "Formats" ), regionalTab );
    QGridLayout* formatsLayout = new QGridLayout( formatsGroup );
    regionalTabLayout->addWidget( formatsGroup );

    QLabel* noteFormatsLabel = new QLabel( tr( "Customize the format of numbers, date and time. "
        "Default formats depend on the selected language." ), regionalGroup );
    formatsLayout->addWidget( noteFormatsLabel, 0, 0, 1, 3 );

    QLabel* numberLabel = new QLabel( tr( "&Number format:" ), formatsGroup );
    formatsLayout->addWidget( numberLabel, 1, 0 );

    m_numberComboBox = new SeparatorComboBox( formatsGroup );
    formatsLayout->addWidget( m_numberComboBox, 1, 1 );

    numberLabel->setBuddy( m_numberComboBox );

    QLabel* dateLabel = new QLabel( tr( "&Date format:" ), formatsGroup );
    formatsLayout->addWidget( dateLabel, 2, 0 );

    m_dateComboBox = new SeparatorComboBox( formatsGroup );
    formatsLayout->addWidget( m_dateComboBox, 2, 1 );

    dateLabel->setBuddy( m_dateComboBox );

    QLabel* timeLabel = new QLabel( tr( "&Time format:" ), formatsGroup );
    formatsLayout->addWidget( timeLabel, 3, 0 );

    m_timeComboBox = new SeparatorComboBox( formatsGroup );
    formatsLayout->addWidget( m_timeComboBox, 3, 1 );

    timeLabel->setBuddy( m_timeComboBox );

    QLabel* firstDayLabel = new QLabel( tr( "&First day of week:" ), formatsGroup );
    formatsLayout->addWidget( firstDayLabel, 4, 0 );

    m_firstDayComboBox = new SeparatorComboBox( formatsGroup );
    formatsLayout->addWidget( m_firstDayComboBox, 4, 1 );

    firstDayLabel->setBuddy( m_firstDayComboBox );

    formatsLayout->setColumnStretch( 2, 1 );

    regionalTabLayout->addStretch( 1 );

    QWidget* viewTab = new QWidget( m_tabWidget );
    m_tabWidget->addTab( viewTab, IconLoader::icon( "view" ), tr( "View" ) );

    QVBoxLayout* viewTabLayout = new QVBoxLayout( viewTab );

    QGroupBox* pageGroup = new QGroupBox( tr( "Page Size" ), viewTab );
    QGridLayout* pageLayout = new QGridLayout( pageGroup );
    viewTabLayout->addWidget( pageGroup );

    QLabel* notePageLabel = new QLabel( tr( "The following settings only affect the Web Client." ), pageGroup );
    pageLayout->addWidget( notePageLabel, 0, 0, 1, 3 );

    int row = 1;

    if ( dataManager->checkServerVersion( "1.1.2" ) ) {
        QLabel* projectPageLabel = new QLabel( tr( "Projects tree:" ), pageGroup );
        pageLayout->addWidget( projectPageLabel, row, 0 );

        m_projectPageComboBox = new SeparatorComboBox( pageGroup );
        pageLayout->addWidget( m_projectPageComboBox, row++, 1 );

        projectPageLabel->setBuddy( m_projectPageComboBox );

        m_projectPageComboBox->addItem( tr( "Default (%1)", "page size" ).arg( dataManager->setting( "project_page_size" ) ) );
        m_projectPageComboBox->addSeparator();
        for ( int i = 5; i <= 30; i += 5 )
            m_projectPageComboBox->addItem( QString::number( i ), i );
    }

    QLabel* folderPageLabel = new QLabel( tr( "List of issues:" ), pageGroup );
    pageLayout->addWidget( folderPageLabel, row, 0 );

    m_folderPageComboBox = new SeparatorComboBox( pageGroup );
    pageLayout->addWidget( m_folderPageComboBox, row++, 1 );

    folderPageLabel->setBuddy( m_folderPageComboBox );

    m_folderPageComboBox->addItem( tr( "Default (%1)", "page size" ).arg( dataManager->setting( "folder_page_size" ) ) );
    m_folderPageComboBox->addSeparator();
    for ( int i = 5; i <= 30; i += 5 )
        m_folderPageComboBox->addItem( QString::number( i ), i );

    QLabel* historyPageLabel = new QLabel( tr( "Issue history:" ), pageGroup );
    pageLayout->addWidget( historyPageLabel, row, 0 );

    m_historyPageComboBox = new SeparatorComboBox( pageGroup );
    pageLayout->addWidget( m_historyPageComboBox, row++, 1 );

    m_historyPageComboBox->addItem( tr( "Default (%1)", "page size" ).arg( dataManager->setting( "history_page_size" ) ) );
    m_historyPageComboBox->addSeparator();
    for ( int i = 10; i <= 50; i += 10 )
        m_historyPageComboBox->addItem( QString::number( i ), i );

    historyPageLabel->setBuddy( m_historyPageComboBox );

    pageLayout->setColumnStretch( 2, 1 );

    QGroupBox* viewGroup = new QGroupBox( tr( "View Settings" ), viewTab );
    QGridLayout* viewLayout = new QGridLayout( viewGroup );
    viewTabLayout->addWidget( viewGroup );

    QLabel* noteViewLabel = new QLabel( tr( "Global view settings that affect all issue types." ), viewGroup );
    viewLayout->addWidget( noteViewLabel, 0, 0, 1, 3 );

    QLabel* orderLabel = new QLabel( tr( "Order of issue history:" ), viewGroup );
    viewLayout->addWidget( orderLabel, 1, 0 );

    m_orderComboBox = new SeparatorComboBox( viewGroup );
    viewLayout->addWidget( m_orderComboBox, 1, 1 );

    orderLabel->setBuddy( m_orderComboBox );

    QMap<QString, QString> orderItems;
    orderItems.insert( "asc", tr( "Oldest First" ) );
    orderItems.insert( "desc", tr( "Newest First" ) );

    QString defaultOrder = orderItems.value( dataManager->setting( "history_order" ) );

    m_orderComboBox->addItem( tr( "Default (%1)", "order" ).arg( defaultOrder ) );
    m_orderComboBox->addSeparator();
    for ( QMap<QString, QString>::const_iterator it = orderItems.begin(); it != orderItems.end(); ++it )
        m_orderComboBox->addItem( it.value(), it.key() );

    QLabel* filterLabel = new QLabel( tr( "Default filter in issue history:" ), viewGroup );
    viewLayout->addWidget( filterLabel, 2, 0 );

    m_filterComboBox = new SeparatorComboBox( viewGroup );
    viewLayout->addWidget( m_filterComboBox, 2, 1 );

    filterLabel->setBuddy( m_filterComboBox );

    QMap<int, QString> filterItems;
    filterItems.insert( IssueDetailsGenerator::AllHistory, tr( "All History" ) );
    filterItems.insert( IssueDetailsGenerator::CommentsAndFiles, tr( "Comments & Attachments" ) );

    QString defaultFilter = filterItems.value( dataManager->setting( "history_filter" ).toInt() );

    m_filterComboBox->addItem( tr( "Default (%1)", "filter" ).arg( defaultFilter ) );
    m_filterComboBox->addSeparator();
    for ( QMap<int, QString>::const_iterator it = filterItems.begin(); it != filterItems.end(); ++it )
        m_filterComboBox->addItem( it.value(), it.key() );

    viewLayout->setColumnStretch( 2, 1 );

    QGroupBox* editGroup = new QGroupBox( tr( "Editing" ), viewTab );
    QGridLayout* editLayout = new QGridLayout( editGroup );
    viewTabLayout->addWidget( editGroup );

    QLabel* editNoteLabel = new QLabel( tr( "The default format used for new comments and descriptions." ), editGroup );
    editLayout->addWidget( editNoteLabel, 0, 0, 1, 3 );

    QLabel* formatLabel = new QLabel( tr( "Default text format:" ), editGroup );
    editLayout->addWidget( formatLabel, 1, 0 );

    m_formatComboBox = new SeparatorComboBox( editGroup );
    editLayout->addWidget( m_formatComboBox, 1, 1 );

    formatLabel->setBuddy( m_formatComboBox );

    QMap<int, QString> formatItems;
    formatItems.insert( PlainText, tr( "Plain Text" ) );
    formatItems.insert( TextWithMarkup, tr( "Text with Markup" ) );

    QString defaultFormat = formatItems.value( dataManager->setting( "default_format" ).toInt() );

    m_formatComboBox->addItem( tr( "Default (%1)", "format" ).arg( defaultFormat ) );
    m_formatComboBox->addSeparator();
    for ( QMap<int, QString>::const_iterator it = formatItems.begin(); it != formatItems.end(); ++it )
        m_formatComboBox->addItem( it.value(), it.key() );

    editLayout->setColumnStretch( 2, 1 );

    viewTabLayout->addStretch( 1 );

    int emailEnabled = dataManager->setting( "email_enabled" ).toInt();

    if ( emailEnabled != 0 ) {
        QWidget* notifyTab = new QWidget( m_tabWidget );
        m_tabWidget->addTab( notifyTab, IconLoader::icon( "alert" ), tr( "Notifications" ) );

        QGridLayout* notifyLayout = new QGridLayout( notifyTab );

        QLabel* emailLabel = new QLabel( tr( "&Email address:" ), notifyTab );
        notifyLayout->addWidget( emailLabel, 0, 0 );

        m_emailEdit = new InputLineEdit( notifyTab );
        m_emailEdit->setMaxLength( 255 );
        m_emailEdit->setFormat( InputLineEdit::EmailFormat );
        notifyLayout->addWidget( m_emailEdit, 0, 1 );

        emailLabel->setBuddy( m_emailEdit );

        QLabel* noteEmailLabel = new QLabel( tr( "This address will only be visible to the system administrator. "
            "You will not receive notifications if you leave this field empty." ), notifyTab );
        notifyLayout->addWidget( noteEmailLabel, 1, 0, 1, 2 );

        notifyLayout->setRowMinimumHeight( 2, 5 );

        QGroupBox* alertGroup = new QGroupBox( tr( "Settings" ), notifyTab );
        QVBoxLayout* alertLayout = new QVBoxLayout( alertGroup );
        notifyLayout->addWidget( alertGroup, 3, 0, 1, 2 );

        m_detailsCheckBox = new QCheckBox( tr( "I&nclude issue details in notificatons and summary reports" ), alertGroup );
        alertLayout->addWidget( m_detailsCheckBox );

        m_noReadCheckBox = new QCheckBox( tr( "&Do not notify about issues that I have already read" ), alertGroup );
        alertLayout->addWidget( m_noReadCheckBox );

        notifyLayout->setRowStretch( notifyLayout->rowCount(), 1 );
    }

    LanguageEntity defaultLanguage = LanguageEntity::find( dataManager->setting( "language" ) );

    m_languageComboBox->addItem( tr( "Default (%1)", "language" ).arg( defaultLanguage.name() ) );
    m_languageComboBox->addSeparator();

    foreach ( const LanguageEntity& language, LanguageEntity::list() )
        m_languageComboBox->addItem( language.name(), language.code() );

    m_numberComboBox->addItem( tr( "Default", "format" ) );
    m_numberComboBox->addSeparator();

    QMap<QString, QString> numberFormats = application->formats( "number_format" );
    for ( QMap<QString, QString>::const_iterator it = numberFormats.begin(); it != numberFormats.end(); ++it ) {
        DefinitionInfo info = DefinitionInfo::fromString( it.value() );
        QString number = QLatin1String( "1" );
        number += info.metadata( "group-separator" ).toString();
        number += QLatin1String( "000" );
        number += info.metadata( "decimal-separator" ).toString();
        number += QLatin1String( "00" );
        m_numberComboBox->addItem( number, it.key() );
    }

    m_dateComboBox->addItem( tr( "Default", "format" ) );
    m_dateComboBox->addSeparator();

    QMap<QString, QString> dateFormats = application->formats( "date_format" );
    for ( QMap<QString, QString>::const_iterator it = dateFormats.begin(); it != dateFormats.end(); ++it ) {
        DefinitionInfo info = DefinitionInfo::fromString( it.value() );
        QMap<QChar, QString> parts;
        parts.insert( QChar( 'd' ), info.metadata( "pad-day" ).toBool() ? "dd" : "d" );
        parts.insert( QChar( 'm' ), info.metadata( "pad-month" ).toBool() ? "mm" : "m" );
        parts.insert( QChar( 'y' ), "yyyy" );
        QString separator = info.metadata( "date-separator" ).toString();
        QString order = info.metadata( "date-order" ).toString();
        QString date = parts.value( order.at( 0 ) );
        date += separator;
        date += parts.value( order.at( 1 ) );
        date += separator;
        date += parts.value( order.at( 2 ) );
        m_dateComboBox->addItem( date, it.key() );
    }

    m_timeComboBox->addItem( tr( "Default", "format" ) );
    m_timeComboBox->addSeparator();

    QMap<QString, QString> timeFormats = application->formats( "time_format" );
    for ( QMap<QString, QString>::const_iterator it = timeFormats.begin(); it != timeFormats.end(); ++it ) {
        DefinitionInfo info = DefinitionInfo::fromString( it.value() );
        int mode = info.metadata( "time-mode" ).toInt();
        QString time = ( mode == 12 ) ? "h" : "H";
        if ( info.metadata( "pad-hour" ).toBool() )
            time += time;
        time += info.metadata( "time-separator" ).toString();
        time += QLatin1String( "mm" );
        if ( mode == 12 )
            time += QLatin1String( " tt" );
        m_timeComboBox->addItem( time, it.key() );
    }

    m_firstDayComboBox->addItem( tr( "Default", "day of week" ) );
    m_firstDayComboBox->addSeparator();

    QLocale currentLocale;
    for ( int i = 0; i < 7; i++ )
        m_firstDayComboBox->addItem( currentLocale.dayName( i != 0 ? i : 7 ), i );

    QString defaultZone = formatTimeZone( dataManager->setting( "time_zone" ) );

    m_timeZoneComboBox->addItem( tr( "Default (%1)", "time zone" ).arg( defaultZone ) );
    m_timeZoneComboBox->addSeparator();

    QMap<int, QStringList> timeZones;
    foreach ( const TimeZoneEntity& timeZone, TimeZoneEntity::list() ) {
        QMap<int, QStringList>::iterator it = timeZones.find( timeZone.offset() );
        if ( it != timeZones.end() )
            it.value().append( timeZone.name() );
        else
            timeZones.insert( timeZone.offset(), QStringList() << timeZone.name() );
    }

    QDateTime now = QDateTime::currentDateTime().toUTC();

    for ( QMap<int, QStringList>::const_iterator it = timeZones.constBegin(); it != timeZones.constEnd(); ++it ) {
        int offset = it.key();
        QString time = now.addSecs( offset ).toString( "hh:mm" );
        char sign = offset < 0 ? '-' : '+';
        if ( offset < 0 )
            offset = -offset;
        QString zone = QString().sprintf( "%c%02d:%02d", sign, offset / 3600, ( offset / 60 ) % 60 );
        m_timeZoneComboBox->addParentItem( QString( "GMT%1 (%2)" ).arg( zone, time ) );
        for ( int i = 0; i < it.value().count(); i++ ) {
            QString zone = it.value().at( i );
            QString name = formatTimeZone( zone );
            m_timeZoneComboBox->addChildItem( name, zone );
        }
    }

    foreach ( const PreferenceEntity& preference, PreferenceEntity::list( m_userId ) )
        m_preferences.insert( preference.key(), preference.value() );

    int index;
    index = m_languageComboBox->findData( m_preferences.value( "language" ) );
    m_languageComboBox->setCurrentIndex( index >= 2 ? index : 0 );
    index = m_timeZoneComboBox->findData( m_preferences.value( "time_zone" ) );
    m_timeZoneComboBox->setCurrentIndex( index >= 2 ? index : 0 );
    index = m_numberComboBox->findData( m_preferences.value( "number_format" ) );
    m_numberComboBox->setCurrentIndex( index >= 2 ? index : 0 );
    index = m_dateComboBox->findData( m_preferences.value( "date_format" ) );
    m_dateComboBox->setCurrentIndex( index >= 2 ? index : 0 );
    index = m_timeComboBox->findData( m_preferences.value( "time_format" ) );
    m_timeComboBox->setCurrentIndex( index >= 2 ? index : 0 );
    index = m_firstDayComboBox->findData( m_preferences.value( "first_day_of_week" ) );
    m_firstDayComboBox->setCurrentIndex( index >= 2 ? index : 0 );
    if ( m_projectPageComboBox ) {
        index = m_projectPageComboBox->findData( m_preferences.value( "project_page_size" ) );
        m_projectPageComboBox->setCurrentIndex( index >= 2 ? index : 0 );
    }
    index = m_folderPageComboBox->findData( m_preferences.value( "folder_page_size" ) );
    m_folderPageComboBox->setCurrentIndex( index >= 2 ? index : 0 );
    index = m_historyPageComboBox->findData( m_preferences.value( "history_page_size" ) );
    m_historyPageComboBox->setCurrentIndex( index >= 2 ? index : 0 );
    index = m_orderComboBox->findData( m_preferences.value( "history_order" ) );
    m_orderComboBox->setCurrentIndex( index >= 2 ? index : 0 );
    index = m_filterComboBox->findData( m_preferences.value( "history_filter" ) );
    m_filterComboBox->setCurrentIndex( index >= 2 ? index : 0 );
    index = m_formatComboBox->findData( m_preferences.value( "default_format" ) );
    m_formatComboBox->setCurrentIndex( index >= 2 ? index : 0 );

    if ( m_emailEdit )
        m_emailEdit->setInputValue( m_preferences.value( "email" ) );

    if ( m_detailsCheckBox )
        m_detailsCheckBox->setChecked( m_preferences.value( "notify_details" ).toInt() != 0 );

    if ( m_noReadCheckBox )
        m_noReadCheckBox->setChecked( m_preferences.value( "notify_no_read" ).toInt() != 0 );

    setContentLayout( layout, true );
}

PreferencesDialog::~PreferencesDialog()
{
}

void PreferencesDialog::accept()
{
    if ( !validate() )
        return;

    QMap<QString, QString> preferences;

    preferences.insert( "language", m_languageComboBox->itemData( m_languageComboBox->currentIndex() ).toString() );
    preferences.insert( "number_format", m_numberComboBox->itemData( m_numberComboBox->currentIndex() ).toString() );
    preferences.insert( "date_format", m_dateComboBox->itemData( m_dateComboBox->currentIndex() ).toString() );
    preferences.insert( "time_format", m_timeComboBox->itemData( m_timeComboBox->currentIndex() ).toString() );
    preferences.insert( "first_day_of_week", m_firstDayComboBox->itemData( m_firstDayComboBox->currentIndex() ).toString() );
    preferences.insert( "time_zone", m_timeZoneComboBox->itemData( m_timeZoneComboBox->currentIndex() ).toString() );
    if ( m_projectPageComboBox )
        preferences.insert( "project_page_size", m_projectPageComboBox->itemData( m_projectPageComboBox->currentIndex() ).toString() );
    preferences.insert( "folder_page_size", m_folderPageComboBox->itemData( m_folderPageComboBox->currentIndex() ).toString() );
    preferences.insert( "history_page_size", m_historyPageComboBox->itemData( m_historyPageComboBox->currentIndex() ).toString() );
    preferences.insert( "history_order", m_orderComboBox->itemData( m_orderComboBox->currentIndex() ).toString() );
    preferences.insert( "history_filter", m_filterComboBox->itemData( m_filterComboBox->currentIndex() ).toString() );
    preferences.insert( "default_format", m_formatComboBox->itemData( m_formatComboBox->currentIndex() ).toString() );

    if ( m_emailEdit )
        preferences.insert( "email", m_emailEdit->inputValue() );

    if ( m_detailsCheckBox )
        preferences.insert( "notify_details", m_detailsCheckBox->isChecked() ? "1" : "" );

    if ( m_noReadCheckBox )
        preferences.insert( "notify_no_read", m_noReadCheckBox->isChecked() ? "1" : "" );

    SetPreferencesBatch* batch = NULL;

    for ( QMap<QString, QString>::const_iterator it = preferences.constBegin(); it != preferences.constEnd(); ++it ) {
        if ( m_preferences.value( it.key() ) != it.value() ) {
            if ( !batch )
                batch = new SetPreferencesBatch( m_userId );
            batch->setPreference( it.key(), it.value() );
        }
    }

    if ( batch )
        executeBatch( batch );
    else
        QDialog::accept();
}
