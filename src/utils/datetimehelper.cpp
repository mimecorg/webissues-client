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

#include "datetimehelper.h"

QString DateTimeHelper::formatDate( const QDate& date )
{
    return date.toString( "yyyy-MM-dd" );
}

QString DateTimeHelper::formatDateTime( const QDateTime& dateTime )
{
    return dateTime.toString( "yyyy-MM-dd hh:mm" );
}

QDate DateTimeHelper::parseDate( const QString& text )
{
    QDate date;
    if ( text.length() >= 10 ) {
        int year = text.mid( 0, 4 ).toInt();
        int month = text.mid( 5, 2 ).toInt();
        int day = text.mid( 8, 2 ).toInt();
        date.setDate( year, month, day );
    }
    return date;
}

QDateTime DateTimeHelper::parseDateTime( const QString& text )
{
    QDate date;
    if ( text.length() >= 10 ) {
        int year = text.mid( 0, 4 ).toInt();
        int month = text.mid( 5, 2 ).toInt();
        int day = text.mid( 8, 2 ).toInt();
        date.setDate( year, month, day );
    }
    QTime time( 0, 0 );
    if ( text.length() >= 16 ) {
        int hours = text.mid( 11, 2 ).toInt();
        int minutes = text.mid( 14, 2 ).toInt();
        time.setHMS( hours, minutes, 0 );
    }
    return QDateTime( date, time, Qt::UTC );
}

int DateTimeHelper::compareDateTime( const QDateTime& dateTime1, const QDateTime& dateTime2 )
{
    if ( dateTime1 < dateTime2 )
        return -1;
    if ( dateTime1 > dateTime2 )
        return 1;
    return 0;
}
