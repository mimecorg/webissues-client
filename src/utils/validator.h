/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2017 WebIssues Team
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

#ifndef VALIDATOR_H
#define VALIDATOR_H

#include "definitioninfo.h"
#include "attributehelper.h"

#include <QStringList>

/**
* Helper functions for validating values and definition.
*/
class Validator : public QObject
{
    Q_OBJECT
public:
    /**
    * Default constructor.
    */
    Validator();

    /**
    * Destructor.
    */
    ~Validator();

public:
    /**
    * Return @c true if no validation errors occurred.
    */
    bool isValid() const { return m_errors.isEmpty(); }

    /**
    * Return a list of validation errors.
    */
    const QStringList& errors() const { return m_errors; }

    /**
    * Remove unnecessary spaces and check if string is valid.
    */
    QString normalizeString( const QString& string, int maxLength );

    /**
    * Remove unnecessary spaces and check if multi-line string is valid.
    */
    QString normalizeMultiLineString( const QString& string );

    /**
    * Remove unnecessary spaces and check if string list is valid.
    */
    QStringList normalizeStringList( const QStringList& list, int maxLength );

    /**
    * Check if the string is a valid email address.
    */
    bool checkEmail( const QString& email );

    /**
    * Create attribute definition using given type and metadata.
    */
    DefinitionInfo createAttributeDefinition( AttributeType type, const QVariantMap& metadata );

    /**
    * Parse a localized number.
    */
    double parseNumber( const QString& value, int decimal );

    /**
    * Convert a localized number to internal representation.
    */
    QString convertNumber( const QString& value, int decimal );

    /**
    * Parse a localized date.
    */
    QDate parseDate( const QString& value );

    /**
    * Convert a localized date to internal representation.
    */
    QString convertDate( const QString& value );

    /**
    * Parse a localized date and time.
    */
    QDateTime parseDateTime( const QString& value, bool fromLocal );

    /**
    * Convert a localized date and time to internal representation.
    */
    QString convertDateTime( const QString& value, bool fromLocal );

private:
    bool checkInteger( int value, int minValue, int maxValue );
    bool checkStringLength( const QString& string, const DefinitionInfo& info );

    bool setMinMaxLength( DefinitionInfo& info, const QVariantMap& metadata );

    QString buildDatePattern( const QString& separator, const QString& order ) const;

    void appendError( int code );

private:
    QStringList m_errors;
};

#endif
