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

#include "preferencesdialog.h"

#include "commands/preferencesbatch.h"
#include "data/datamanager.h"
#include "data/localecache.h"
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
#include <QGroupBox>
#include <QButtonGroup>
#include <QTimer>

PreferencesDialog::PreferencesDialog( int userId, QWidget* parent ) : CommandDialog( parent ),
    m_userId( userId ),
    m_initialized( false ),
    m_emailEdit( NULL ),
    m_noReadCheckBox( NULL ),
    m_daysGroup( NULL ),
    m_hoursGroup( NULL )
{
    const UserRow* user = dataManager->users()->find( userId );
    QString name = user ? user->name() : QString();

    setWindowTitle( tr( "User Preferences" ) );
    if ( userId == dataManager->currentUserId() )
        setPrompt( tr( "Configure your user preferences:" ) );
    else
        setPrompt( tr( "Configure preferences for user <b>%1</b>:" ).arg( name ) );
    setPromptPixmap( IconLoader::pixmap( "preferences", 22 ) );

    showInfo( tr( "Edit user preferences." ) );

    QVBoxLayout* layout = new QVBoxLayout();

    m_tabWidget = new QTabWidget( this );
    layout->addWidget( m_tabWidget );

    QWidget* regionalTab = new QWidget( m_tabWidget );
    m_tabWidget->addTab( regionalTab, IconLoader::icon( "regional" ), tr( "Regional" ) );

    QVBoxLayout* regionalLayout = new QVBoxLayout( regionalTab );

    QGroupBox* globalGroup = new QGroupBox( tr( "Global Options" ), regionalTab );
    QGridLayout* globalLayout = new QGridLayout( globalGroup );
    regionalLayout->addWidget( globalGroup );

    QLabel* numberLabel = new QLabel( tr( "&Number format:" ), globalGroup );
    globalLayout->addWidget( numberLabel, 0, 0 );

    m_numberComboBox = new SeparatorComboBox( globalGroup );
    globalLayout->addWidget( m_numberComboBox, 0, 1 );

    numberLabel->setBuddy( m_numberComboBox );

    QLabel* dateLabel = new QLabel( tr( "&Date format:" ), globalGroup );
    globalLayout->addWidget( dateLabel, 1, 0 );

    m_dateComboBox = new SeparatorComboBox( globalGroup );
    globalLayout->addWidget( m_dateComboBox, 1, 1 );

    dateLabel->setBuddy( m_dateComboBox );

    QLabel* timeLabel = new QLabel( tr( "&Time format:" ), globalGroup );
    globalLayout->addWidget( timeLabel, 2, 0 );

    m_timeComboBox = new SeparatorComboBox( globalGroup );
    globalLayout->addWidget( m_timeComboBox, 2, 1 );

    timeLabel->setBuddy( m_timeComboBox );

    QLabel* firstDayLabel = new QLabel( tr( "&First day of week:" ), globalGroup );
    globalLayout->addWidget( firstDayLabel, 3, 0 );

    m_firstDayComboBox = new SeparatorComboBox( globalGroup );
    globalLayout->addWidget( m_firstDayComboBox, 3, 1 );

    firstDayLabel->setBuddy( m_firstDayComboBox );

    globalLayout->setColumnStretch( 2, 1 );

    QGroupBox* webGroup = new QGroupBox( tr( "Web Client Options" ), regionalTab );
    QGridLayout* webLayout = new QGridLayout( webGroup );
    regionalLayout->addWidget( webGroup );

    QLabel* languageLabel = new QLabel( tr( "&Language:" ), webGroup );
    webLayout->addWidget( languageLabel, 0, 0 );

    m_languageComboBox = new SeparatorComboBox( webGroup );
    webLayout->addWidget( m_languageComboBox, 0, 1 );

    languageLabel->setBuddy( m_languageComboBox );

    QLabel* timeZoneLabel = new QLabel( tr( "Time &zone:" ), webGroup );
    webLayout->addWidget( timeZoneLabel, 1, 0 );

    m_timeZoneComboBox = new SeparatorComboBox( webGroup );
    m_timeZoneComboBox->setMaxVisibleItems( 20 );
    webLayout->addWidget( m_timeZoneComboBox, 1, 1 );

    timeZoneLabel->setBuddy( m_timeZoneComboBox );

    QLabel* noteWebLabel = new QLabel( tr( "These settings affect the WebIssues Web Client only. "
        "The Desktop Client will always use the language configured in program settings and the local time zone." ), webGroup );
    noteWebLabel->setWordWrap( true );
    webLayout->addWidget( noteWebLabel, 2, 0, 1, 3 );

    webLayout->setColumnStretch( 2, 1 );

    regionalLayout->addStretch( 1 );

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
        noteEmailLabel->setWordWrap( true );
        notifyLayout->addWidget( noteEmailLabel, 1, 0, 1, 2 );

        notifyLayout->setRowMinimumHeight( 2, 5 );

        QGroupBox* alertGroup = new QGroupBox( tr( "Alert Notifications" ), notifyTab );
        QVBoxLayout* alertLayout = new QVBoxLayout( alertGroup );
        notifyLayout->addWidget( alertGroup, 3, 0, 1, 2 );

        m_noReadCheckBox = new QCheckBox( tr( "&Do not include issues that I have already read" ), alertGroup );
        alertLayout->addWidget( m_noReadCheckBox );

        QGroupBox* summaryGroup = new QGroupBox( tr( "Summary Schedule" ), notifyTab );
        QVBoxLayout* summaryLayout = new QVBoxLayout( summaryGroup );
        notifyLayout->addWidget( summaryGroup, 4, 0, 1, 2 );

        QHBoxLayout* daysHeaderLayout = new QHBoxLayout();
        summaryLayout->addLayout( daysHeaderLayout );

        QLabel* daysLabel = new QLabel( tr( "Send on the following days:" ), summaryGroup );
        daysHeaderLayout->addWidget( daysLabel );

        daysHeaderLayout->addStretch( 1 );

        QLabel* allDaysLabel = new QLabel( "<a href=\"#\">" + tr( "Select All" ) + "</a>", summaryGroup );
        daysHeaderLayout->addWidget( allDaysLabel );

        QLabel* noDaysLabel = new QLabel( "<a href=\"#\">" + tr( "Unselect All" ) + "</a>", summaryGroup );
        daysHeaderLayout->addWidget( noDaysLabel );

        connect( allDaysLabel, SIGNAL( linkActivated( const QString& ) ), this, SLOT( allDaysActivated() ) );
        connect( noDaysLabel, SIGNAL( linkActivated( const QString& ) ), this, SLOT( noDaysActivated() ) );

        QHBoxLayout* daysLayout = new QHBoxLayout();
        summaryLayout->addLayout( daysLayout );

        m_daysGroup = new QButtonGroup( this );
        m_daysGroup->setExclusive( false );

        QLocale currentLocale;
        int firstDayOfWeek = dataManager->setting( "first_day_of_week" ).toInt();

        for ( int i = 0; i < 7; i++ ) {
            int day = ( i + firstDayOfWeek ) % 7;
            QCheckBox* checkBox = new QCheckBox( currentLocale.dayName( day != 0 ? day : 7 ), summaryGroup );
            m_daysGroup->addButton( checkBox, day );
            daysLayout->addWidget( checkBox );
        }

        daysLayout->addStretch( 1 );

        summaryLayout->addSpacing( 10 );

        QHBoxLayout* hoursHeaderLayout = new QHBoxLayout();
        summaryLayout->addLayout( hoursHeaderLayout );

        QLabel* hoursLabel = new QLabel( tr( "Send at the following hours:" ), summaryGroup );
        hoursHeaderLayout->addWidget( hoursLabel );

        hoursHeaderLayout->addStretch( 1 );

        QLabel* allHoursLabel = new QLabel( "<a href=\"#\">" + tr( "Select All" ) + "</a>", summaryGroup );
        hoursHeaderLayout->addWidget( allHoursLabel );

        QLabel* noHoursLabel = new QLabel( "<a href=\"#\">" + tr( "Unselect All" ) + "</a>", summaryGroup );
        hoursHeaderLayout->addWidget( noHoursLabel );

        connect( allHoursLabel, SIGNAL( linkActivated( const QString& ) ), this, SLOT( allHoursActivated() ) );
        connect( noHoursLabel, SIGNAL( linkActivated( const QString& ) ), this, SLOT( noHoursActivated() ) );

        QGridLayout* hoursLayout = new QGridLayout();
        summaryLayout->addLayout( hoursLayout );

        m_hoursGroup = new QButtonGroup( this );
        m_hoursGroup->setExclusive( false );

        Formatter formatter;

        for ( int row = 0; row < 3; row++ ) {
            for ( int col = 0; col < 8; col++ ) {
                int hour = 8 * row + col;
                QCheckBox* checkBox = new QCheckBox( formatter.formatTime( QTime( hour, 0 ) ), summaryGroup );
                m_hoursGroup->addButton( checkBox, hour );
                hoursLayout->addWidget( checkBox, row, col );
            }
        }

        hoursLayout->setColumnStretch( hoursLayout->columnCount(), 1 );

        summaryLayout->addSpacing( 10 );

        QLabel* noteSummaryLabel = new QLabel( tr( "You will not receive summary emails if you do not select any day and hour." ), summaryGroup );
        noteSummaryLabel->setWordWrap( true );
        summaryLayout->addWidget( noteSummaryLabel );

        notifyLayout->setRowStretch( notifyLayout->rowCount(), 1 );
    }

    setContentLayout( layout, true );

    LoadPreferencesBatch* batch = new LoadPreferencesBatch( userId );
    executeBatch( batch, tr( "Loading preferences..." ) );
}

PreferencesDialog::~PreferencesDialog()
{
}

bool PreferencesDialog::batchSuccessful( AbstractBatch* batch )
{
    if ( !m_initialized ) {
        LoadPreferencesBatch* loadBatch = (LoadPreferencesBatch*)batch;
        m_preferences = loadBatch->preferences();
        initialize();
        showInfo( tr( "Edit user preferences." ) );
        return false;
    }

    return true;
}

bool PreferencesDialog::batchFailed( AbstractBatch* /*batch*/ )
{
    if ( !m_initialized )
        return false;
    return true;
}

void PreferencesDialog::fixGeometry()
{
    m_languageComboBox->updateGeometry();
    m_numberComboBox->updateGeometry();
    m_dateComboBox->updateGeometry();
    m_timeComboBox->updateGeometry();
    m_firstDayComboBox->updateGeometry();
    m_timeZoneComboBox->updateGeometry();
}

void PreferencesDialog::initialize()
{
    LocaleCache* cache = dataManager->localeCache();

    m_languageComboBox->addItem( tr( "Default", "language" ) );
    m_languageComboBox->addSeparator();

    for ( int i = 0; i < cache->languages().count(); i++ ) {
        LocaleLanguage language = cache->languages().at( i );
        int index = 2;
        while ( index < m_languageComboBox->count() && QString::localeAwareCompare( language.name(), m_languageComboBox->itemText( index ) ) > 0 )
            index++;
        m_languageComboBox->insertItem( index, language.name(), language.code() );
    }

    m_numberComboBox->addItem( tr( "Default", "format" ) );
    m_numberComboBox->addSeparator();

    QList<LocaleFormat> numberFormats = cache->formats( "number_format" );
    for ( int i = 0; i < numberFormats.count(); i++ ) {
        DefinitionInfo info = DefinitionInfo::fromString( numberFormats.at( i ).definition() );
        QString number = QLatin1String( "1" );
        number += info.metadata( "group-separator" ).toString();
        number += QLatin1String( "000" );
        number += info.metadata( "decimal-separator" ).toString();
        number += QLatin1String( "00" );
        m_numberComboBox->addItem( number, numberFormats.at( i ).key() );
    }

    m_dateComboBox->addItem( tr( "Default", "format" ) );
    m_dateComboBox->addSeparator();

    QList<LocaleFormat> dateFormats = cache->formats( "date_format" );
    for ( int i = 0; i < dateFormats.count(); i++ ) {
        DefinitionInfo info = DefinitionInfo::fromString( dateFormats.at( i ).definition() );
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
        m_dateComboBox->addItem( date, dateFormats.at( i ).key() );
    }

    m_timeComboBox->addItem( tr( "Default", "format" ) );
    m_timeComboBox->addSeparator();

    QList<LocaleFormat> timeFormats = cache->formats( "time_format" );
    for ( int i = 0; i < timeFormats.count(); i++ ) {
        DefinitionInfo info = DefinitionInfo::fromString( timeFormats.at( i ).definition() );
        int mode = info.metadata( "time-mode" ).toInt();
        QString time = ( mode == 12 ) ? "h" : "H";
        if ( info.metadata( "pad-hour" ).toBool() )
            time += time;
        time += info.metadata( "time-separator" ).toString();
        time += QLatin1String( "mm" );
        if ( mode == 12 )
            time += QLatin1String( " tt" );
        m_timeComboBox->addItem( time, timeFormats.at( i ).key() );
    }

    m_firstDayComboBox->addItem( tr( "Default", "day of week" ) );
    m_firstDayComboBox->addSeparator();

    QLocale currentLocale;
    for ( int i = 0; i < 7; i++ )
        m_firstDayComboBox->addItem( currentLocale.dayName( i != 0 ? i : 7 ), i );

    m_timeZoneComboBox->addItem( tr( "Default", "time zone" ) );
    m_timeZoneComboBox->addSeparator();

    QMap<int, QStringList> timeZones;
    for ( int i = 0; i < cache->timeZones().count(); i++ ) {
        LocaleTimeZone timeZone = cache->timeZones().at( i );
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
            QString name = zone;
            name.replace( QChar( '_' ), QChar( ' ' ) );
            name.replace( QLatin1String( "/" ), QLatin1String( " / " ) );
            name.replace( QLatin1String( "St " ), QLatin1String( "St. " ) );
            m_timeZoneComboBox->addChildItem( name, zone );
        }
    }

    int index;
    index = m_languageComboBox->findData( m_preferences.value( "language" ) );
    m_languageComboBox->setCurrentIndex( index >= 2 ? index : 0 );
    index = m_numberComboBox->findData( m_preferences.value( "number_format" ) );
    m_numberComboBox->setCurrentIndex( index >= 2 ? index : 0 );
    index = m_dateComboBox->findData( m_preferences.value( "date_format" ) );
    m_dateComboBox->setCurrentIndex( index >= 2 ? index : 0 );
    index = m_timeComboBox->findData( m_preferences.value( "time_format" ) );
    m_timeComboBox->setCurrentIndex( index >= 2 ? index : 0 );
    index = m_firstDayComboBox->findData( m_preferences.value( "first_day_of_week" ) );
    m_firstDayComboBox->setCurrentIndex( index >= 2 ? index : 0 );
    index = m_timeZoneComboBox->findData( m_preferences.value( "time_zone" ) );
    m_timeZoneComboBox->setCurrentIndex( index >= 2 ? index : 0 );

    m_languageComboBox->setSizeAdjustPolicy( QComboBox::AdjustToContents );
    m_numberComboBox->setSizeAdjustPolicy( QComboBox::AdjustToContents );
    m_dateComboBox->setSizeAdjustPolicy( QComboBox::AdjustToContents );
    m_timeComboBox->setSizeAdjustPolicy( QComboBox::AdjustToContents );
    m_firstDayComboBox->setSizeAdjustPolicy( QComboBox::AdjustToContents );
    m_timeZoneComboBox->setSizeAdjustPolicy( QComboBox::AdjustToContents );

#if QT_VERSION < 0x040800
    // work around QTBUG-2559
    QTimer::singleShot( 25, this, SLOT( fixGeometry() ) );
#endif

    if ( m_emailEdit )
        m_emailEdit->setInputValue( m_preferences.value( "email" ) );

    if ( m_noReadCheckBox )
        m_noReadCheckBox->setChecked( m_preferences.value( "notify_no_read" ).toInt() != 0 );

    if ( m_daysGroup ) {
        QStringList list = m_preferences.value( "summary_days" ).split( ',' );
        for ( int i = 0; i < list.count(); i++ ) {
            bool ok;
            int day = list.at( i ).toInt( &ok );
            if ( ok && day >= 0 && day < 7 )
                m_daysGroup->button( day )->setChecked( true );
        }
    }

    if ( m_hoursGroup ) {
        QStringList list = m_preferences.value( "summary_hours" ).split( ',' );
        for ( int i = 0; i < list.count(); i++ ) {
            bool ok;
            int day = list.at( i ).toInt( &ok );
            if ( ok && day >= 0 && day < 24 )
                m_hoursGroup->button( day )->setChecked( true );
        }
    }

    m_initialized = true;
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

    if ( m_emailEdit )
        preferences.insert( "email", m_emailEdit->inputValue() );

    if ( m_noReadCheckBox )
        preferences.insert( "notify_no_read", m_noReadCheckBox->isChecked() ? "1" : "0" );

    if ( m_daysGroup ) {
        QStringList list;
        for ( int i = 0; i < 7; i++ ) {
            if ( m_daysGroup->button( i )->isChecked() )
                list.append( QString::number( i ) );
        }
        preferences.insert( "summary_days", list.join( "," ) );
    }

    if ( m_hoursGroup ) {
        QStringList list;
        for ( int i = 0; i < 24; i++ ) {
            if ( m_hoursGroup->button( i )->isChecked() )
                list.append( QString::number( i ) );
        }
        preferences.insert( "summary_hours", list.join( "," ) );
    }

    QMap<QString, QString> changes;

    for ( QMap<QString, QString>::const_iterator it = preferences.constBegin(); it != preferences.constEnd(); ++it ) {
        if ( m_preferences.value( it.key() ) != it.value() )
            changes.insert( it.key(), it.value() );
    }

    if ( changes.isEmpty() ) {
        QDialog::accept();
        return;
    }

    SavePreferencesBatch* batch = new SavePreferencesBatch( m_userId, changes );
    executeBatch( batch );
}

void PreferencesDialog::allDaysActivated()
{
    for ( int i = 0; i < 7; i++ )
        m_daysGroup->button( i )->setChecked( true );
}

void PreferencesDialog::noDaysActivated()
{
    for ( int i = 0; i < 7; i++ )
        m_daysGroup->button( i )->setChecked( false );
}

void PreferencesDialog::allHoursActivated()
{
    for ( int i = 0; i < 24; i++ )
        m_hoursGroup->button( i )->setChecked( true );
}

void PreferencesDialog::noHoursActivated()
{
    for ( int i = 0; i < 24; i++ )
        m_hoursGroup->button( i )->setChecked( false );
}
