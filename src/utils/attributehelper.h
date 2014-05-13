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

#ifndef ATTRIBUTEHELPER_H
#define ATTRIBUTEHELPER_H

#include <QObject>

class DefinitionInfo;

/**
* Type of an attribute
*/
enum AttributeType
{
    /** Invalid attribute definition. */
    InvalidAttribute,
    /** A <tt>TEXT</tt> attribute type. */
    TextAttribute,
    /** An <tt>ENUM</tt> attribute type. */
    EnumAttribute,
    /** A <tt>NUMERIC</tt> attribute type. */
    NumericAttribute,
    /** A <tt>DATETIME</tt> attribute type. */
    DateTimeAttribute,
    /** A <tt>USER</tt> attribute type. */
    UserAttribute,

    FirstAttribute = TextAttribute,
    LastAttribute = UserAttribute
};

/**
* Helper object providing attribute type names and metadata details.
*
* Its functions return information used by the UI to display user friendly
* attribute type names and metadata details.
*
* @see DefinitionInfo
*/
class AttributeHelper : public QObject
{
    Q_OBJECT
public:
    /**
    * Default constructor.
    */
    AttributeHelper();

    /**
    * Destructor.
    */
    ~AttributeHelper();

public:
    /**
    * Return the attribute type from its definition.
    */
    static AttributeType toAttributeType( const DefinitionInfo& info );

    /**
    * Return attribute definition based on its type.
    */
    static DefinitionInfo fromAttributeType( AttributeType type );

public:
    /**
    * Convert expression in initial value of an attribute to actual value.
    * @param info Definition of the attribute type.
    * @param value The initial value of the attribute type.
    * @return The converted value.
    */
    QString convertInitialValue( const DefinitionInfo& info, const QString& value ) const;

    /**
    * Format a value or expression.
    * @param info Definition of the attribute.
    * @param value The standardized value to format.
    * @return The formatted value.
    */
    QString formatExpression( const DefinitionInfo& info, const QString& value ) const;

    /**
    * Return the name of an attribute type.
    * @param type The attribute type.
    * @return A user friendly name of the type.
    */
    QString typeName( AttributeType type ) const;

    /**
    * Return the details of an attribute definition.
    * @param info The attribute definition.
    * @return User friendly information about the attribute definition metadata.
    */
    QString metadataDetails( const DefinitionInfo& info ) const;

    /**
    * Return a list of attribute types compatible with given type.
    */
    QList<AttributeType> compatibleTypes( AttributeType type ) const;

private:
    QString textMetadataDetails( const DefinitionInfo& info ) const;
    QString enumMetadataDetails( const DefinitionInfo& info ) const;
    QString numericMetadataDetails( const DefinitionInfo& info ) const;
    QString dateTimeMetadataDetails( const DefinitionInfo& info ) const;
    QString userMetadataDetails( const DefinitionInfo& info ) const;
};

#endif
