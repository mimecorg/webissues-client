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

#ifndef LOCALE_H
#define LOCALE_H

#include <QString>

/**
* A language in locale information.
*/
class LocaleLanguage
{
public:
    /**
    * Default constructor.
    */
    LocaleLanguage();

    /**
    * Constructor.
    */
    LocaleLanguage( const QString& code, const QString& name );

    /**
    * Destructor.
    */
    ~LocaleLanguage();

public:
    /**
    * Return the language code.
    */
    const QString& code() const { return m_code; }

    /**
    * Return the name of the language.
    */
    const QString& name() const { return m_name; }

private:
    QString m_code;
    QString m_name;
};

/**
* A format definition in locale information.
*/
class LocaleFormat
{
public:
    /**
    * Default constructor.
    */
    LocaleFormat();

    /**
    * Constructor.
    */
    LocaleFormat( const QString& type, const QString& key, const QString& definition );

    /**
    * Destructor.
    */
    ~LocaleFormat();

public:
    /**
    * Return the type of the format.
    */
    const QString& type() const { return m_type; }

    /**
    * Return the key of the format.
    */
    const QString& key() const { return m_key; }

    /**
    * Return the definition of the format.
    */
    const QString& definition() const { return m_definition; }

private:
    QString m_type;
    QString m_key;
    QString m_definition;
};

/**
* A time zone in locale information.
*/
class LocaleTimeZone
{
public:
    /**
    * Default constructor.
    */
    LocaleTimeZone();

    /**
    * Constructor.
    */
    LocaleTimeZone( const QString& name, int offset );

    /**
    * Destructor.
    */
    ~LocaleTimeZone();

public:
    /**
    * Return the name of the time zone.
    */
    const QString& name() const { return m_name; }

    /**
    * Return the offset of the time zone.
    */
    int offset() const { return m_offset; }

private:
    QString m_name;
    int m_offset;
};

#endif
