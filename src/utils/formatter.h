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

#ifndef FORMATTER_H
#define FORMATTER_H

#include <QObject>

class DefinitionInfo;

class QDate;
class QTime;
class QDateTime;

/**
* Helper functions for formatting values.
*/
class Formatter : public QObject
{
    Q_OBJECT
public:
    /**
    * Default constructor.
    */
    Formatter();

    /**
    * Destructor.
    */
    ~Formatter();

public:
    /**
    * Format number using localized format.
    */
    QString formatNumber( double number, int decimal, bool strip ) const;

    /**
    * Convert number in internal representation to localized format.
    */
    QString convertNumber( const QString& value, int decimal, bool strip ) const;

    /**
    * Format date using localized format.
    */
    QString formatDate( const QDate& date ) const;

    /**
    * Convert date in internal representation to localized format.
    */
    QString convertDate( const QString& value ) const;

    /**
    * Format date and time using localized format.
    */
    QString formatDateTime( const QDateTime& dateTime, bool toLocal ) const;

    /**
    * Convert date and time in internal representation to localized format.
    */
    QString convertDateTime( const QString& value, bool toLocal ) const;

    /**
    * Format an attribute value according to its definition.
    */
    QString convertAttributeValue( const DefinitionInfo& info, const QString& value, bool multiLine ) const;

    /**
    * Format time using localized format.
    */
    QString formatTime( const QTime& time ) const;

    /**
    * Format the size of a file.
    */
    QString formatSize( int size ) const;

private:
    QString dateFormat() const;
    QString timeFormat() const;
    QString dateTimeFormat() const;
};

#endif
