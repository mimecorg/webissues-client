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

#include "schedulewidget.h"

#include "data/datamanager.h"
#include "utils/formatter.h"
#include "widgets/linklabel.h"

#include <QLayout>
#include <QLabel>
#include <QCheckBox>
#include <QButtonGroup>
#include <QDateTime>
#include <QLocale>

ScheduleWidget::ScheduleWidget( QWidget* parent ) : QWidget( parent )
{
    QVBoxLayout* layout = new QVBoxLayout( this );
    layout->setMargin( 0 );

    QHBoxLayout* daysHeaderLayout = new QHBoxLayout();
    layout->addLayout( daysHeaderLayout );

    QLabel* daysLabel = new QLabel( tr( "Send summary emails on the following days:" ), this );
    daysHeaderLayout->addWidget( daysLabel );

    daysHeaderLayout->addStretch( 1 );

    QLabel* allDaysLabel = new LinkLabel( tr( "Select All" ), this );
    daysHeaderLayout->addWidget( allDaysLabel );

    QLabel* noDaysLabel = new LinkLabel( tr( "Unselect All" ), this );
    daysHeaderLayout->addWidget( noDaysLabel );

    connect( allDaysLabel, SIGNAL( linkActivated( const QString& ) ), this, SLOT( allDaysActivated() ) );
    connect( noDaysLabel, SIGNAL( linkActivated( const QString& ) ), this, SLOT( noDaysActivated() ) );

    QHBoxLayout* daysLayout = new QHBoxLayout();
    layout->addLayout( daysLayout );

    m_daysGroup = new QButtonGroup( this );
    m_daysGroup->setExclusive( false );

    QLocale currentLocale;
    int firstDayOfWeek = dataManager->localeSetting( "first_day_of_week" ).toInt();

    for ( int i = 0; i < 7; i++ ) {
        int day = ( i + firstDayOfWeek ) % 7;
        QCheckBox* checkBox = new QCheckBox( currentLocale.dayName( day != 0 ? day : 7 ), this );
        m_daysGroup->addButton( checkBox, day );
        daysLayout->addWidget( checkBox );
    }

    daysLayout->addStretch( 1 );

    layout->addSpacing( 10 );

    QHBoxLayout* hoursHeaderLayout = new QHBoxLayout();
    layout->addLayout( hoursHeaderLayout );

    QLabel* hoursLabel = new QLabel( tr( "Send summary emails at the following hours:" ), this );
    hoursHeaderLayout->addWidget( hoursLabel );

    hoursHeaderLayout->addStretch( 1 );

    QLabel* allHoursLabel = new LinkLabel( tr( "Select All" ), this );
    hoursHeaderLayout->addWidget( allHoursLabel );

    QLabel* noHoursLabel = new LinkLabel( tr( "Unselect All" ), this );
    hoursHeaderLayout->addWidget( noHoursLabel );

    connect( allHoursLabel, SIGNAL( linkActivated( const QString& ) ), this, SLOT( allHoursActivated() ) );
    connect( noHoursLabel, SIGNAL( linkActivated( const QString& ) ), this, SLOT( noHoursActivated() ) );

    QGridLayout* hoursLayout = new QGridLayout();
    layout->addLayout( hoursLayout );

    m_hoursGroup = new QButtonGroup( this );
    m_hoursGroup->setExclusive( false );

    Formatter formatter;

    for ( int row = 0; row < 3; row++ ) {
        for ( int col = 0; col < 8; col++ ) {
            int hour = 8 * row + col;
            QCheckBox* checkBox = new QCheckBox( formatter.formatTime( QTime( hour, 0 ) ), this );
            m_hoursGroup->addButton( checkBox, hour );
            hoursLayout->addWidget( checkBox, row, col );
        }
    }

    hoursLayout->setColumnStretch( hoursLayout->columnCount(), 1 );
}

ScheduleWidget::~ScheduleWidget()
{
}

void ScheduleWidget::setDays( const QString& days )
{
    noDaysActivated();

    QStringList list = days.split( ',' );
    for ( int i = 0; i < list.count(); i++ ) {
        bool ok;
        int day = list.at( i ).toInt( &ok );
        if ( ok && day >= 0 && day < 7 )
            m_daysGroup->button( day )->setChecked( true );
    }
}

QString ScheduleWidget::days() const
{
    QStringList list;
    for ( int i = 0; i < 7; i++ ) {
        if ( m_daysGroup->button( i )->isChecked() )
            list.append( QString::number( i ) );
    }
    return list.join( "," );
}

void ScheduleWidget::setHours( const QString& hours )
{
    noHoursActivated();

    QStringList list = hours.split( ',' );
    for ( int i = 0; i < list.count(); i++ ) {
        bool ok;
        int day = list.at( i ).toInt( &ok );
        if ( ok && day >= 0 && day < 24 )
            m_hoursGroup->button( day )->setChecked( true );
    }
}

QString ScheduleWidget::hours() const
{
    QStringList list;
    for ( int i = 0; i < 24; i++ ) {
        if ( m_hoursGroup->button( i )->isChecked() )
            list.append( QString::number( i ) );
    }
    return list.join( "," );
}

void ScheduleWidget::allDaysActivated()
{
    for ( int i = 0; i < 7; i++ )
        m_daysGroup->button( i )->setChecked( true );
}

void ScheduleWidget::noDaysActivated()
{
    for ( int i = 0; i < 7; i++ )
        m_daysGroup->button( i )->setChecked( false );
}

void ScheduleWidget::allHoursActivated()
{
    for ( int i = 0; i < 24; i++ )
        m_hoursGroup->button( i )->setChecked( true );
}

void ScheduleWidget::noHoursActivated()
{
    for ( int i = 0; i < 24; i++ )
        m_hoursGroup->button( i )->setChecked( false );
}
