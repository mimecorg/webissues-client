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

#ifndef DEFINITIONINFO_H
#define DEFINITIONINFO_H

#include <QVariant>

/**
* Structure storing a definition.
*
* The definition consists of a type keyword and metadata.
*/
class DefinitionInfo
{
public:
    /**
    * Default constructor.
    */
    DefinitionInfo();

    /**
    * Destructor.
    */
    ~DefinitionInfo();

public:
    /**
    * Check if the definition is empty.
    * @return @c true if the definition is empty.
    */
    bool isEmpty() const { return m_type.isEmpty(); }

    /**
    * Set the type.
    */
    void setType( const QString& type ) { m_type = type; }

    /**
    * Return the type.
    */
    const QString& type() const { return m_type; }

    /**
    * Set the metadata of the attribute.
    * @param key Name of the metadata to set. It can consist of lowercase letters and hyphens.
    * @param value Value of the metadata. It can be an integer, string or an array of strings.
    *              If it's a null value, the metadata is cleared.
    */
    void setMetadata( const QString& key, const QVariant& value );

    /**
    * Return the metadata of the attribute.
    * @param key Name of the metadata to return.
    * @return Value of the metadata or a null value if the metadata is not defined.
    */
    QVariant metadata( const QString& key ) const;

    /**
    * Return all metadata of the attribute.
    */
    const QVariantMap& metadata() const { return m_metadata; }

    /**
    * Build the attribute definition string according to the WebIssues protocol.
    */
    QString toString() const;

public:
    /**
    * Parse the attribute definition string according to the WebIssues protocol.
    */
    static DefinitionInfo fromString( const QString& text );

private:
    static QString quoteString( const QString& string );
    static QString unquoteString( const QString& string );

private:
    QString m_type;
    QVariantMap m_metadata;
};

#endif
