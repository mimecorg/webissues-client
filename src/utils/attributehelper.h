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

#ifndef ATTRIBUTEHELPER_H
#define ATTRIBUTEHELPER_H

#include <QList>

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
* Functions providing attribute type names and metadata details.
*
* These functions return information used by the UI to display user friendly
* attribute type names and metadata details.
*
* @see DefinitionInfo
*/
class AttributeHelper
{
public:
    /**
    * Return the attribute type from its definition.
    */
    static AttributeType toAttributeType( const DefinitionInfo& info );

    /**
    * Return attribute definition based on its type.
    */
    static DefinitionInfo fromAttributeType( AttributeType type );

    /**
    * Convert expression in initial value of an attribute to actual value.
    * @param info Definition of the attribute type.
    * @param value The initial value of the attribute type.
    * @return The converted value.
    */
    static QString convertInitialValue( const DefinitionInfo& info, const QString& value );

    /**
    * Format a value or expression.
    * @param info Parsed definition of the attribute.
    * @param definition Definition of the attribute.
    * @param value The standardized value to format.
    * @return The formatted value.
    */
    static QString formatExpression( const DefinitionInfo& info, const QString& definition, const QString& value );

    /**
    * Return the name of an attribute type.
    * @param type The attribute type.
    * @return A user friendly name of the type.
    */
    static QString typeName( AttributeType type );

    /**
    * Return the details of an attribute definition.
    * @param info The attribute definition.
    * @return User friendly information about the attribute definition metadata.
    */
    static QString metadataDetails( const DefinitionInfo& info );

    /**
    * Return a list of attribute types compatible with given type.
    */
    static QList<AttributeType> compatibleTypes( AttributeType type );

private:
    static QString textMetadataDetails( const DefinitionInfo& info );
    static QString enumMetadataDetails( const DefinitionInfo& info );
    static QString numericMetadataDetails( const DefinitionInfo& info );
    static QString dateTimeMetadataDetails( const DefinitionInfo& info );
    static QString userMetadataDetails( const DefinitionInfo& info );

    static QString tr( const char* text );
};

#endif
