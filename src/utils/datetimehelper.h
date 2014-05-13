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

#ifndef DATETIMEHELPER_H
#define DATETIMEHELPER_H

#include <QDateTime>

/**
* Functions for formatting and parsing dates.
*
* Helper functions for handling <tt>DATETIME</tt> attribute values formatted according to the WebIssues protocol.
* They are also used to display creation and modification dates in the UI.
*
* The date format is <tt>yyyy-MM-dd</tt>. The date and time format is <tt>yyyy-MM-dd HH:mm</tt>.
*/
class DateTimeHelper
{
public:
    /**
    * Format date to an attribute value.
    * @param date The date to format.
    * @return Attribute value in a <tt>yyyy-MM-dd</tt> format.
    */
    static QString formatDate( const QDate& date );

    /**
    * Format date and time to an attribute value.
    * @param dateTime The date and time to format.
    * @return Attribute value in a <tt>yyyy-MM-dd HH:mm</tt> format.
    */
    static QString formatDateTime( const QDateTime& dateTime );

    /**
    * Parse attribute value to a date.
    * @param text The attribute value to parse. It may be either a date or a date and time.
    * @return The parsed date or an invalid QDate if text cannot be parsed.
    */
    static QDate parseDate( const QString& text );

    /**
    * Parse attribute value to a date and time.
    * @param text The attribute value to parse. It may be either a date or a date and time.
    * @return The parsed date and time or an invalid QDateTime if text cannot be parsed.
    */
    static QDateTime parseDateTime( const QString& text );

    /**
    * Compare two date and time values.
    * This is a helper method used for sorting by date and time.
    * @param dateTime1 The first date and time to compare.
    * @param dateTime2 The second date and time to compare.
    * @return 0 if values are equal, -1 if the first date is earlier, 1 if the second date is earlier.
    */
    static int compareDateTime( const QDateTime& dateTime1, const QDateTime& dateTime2 );
};

#endif
