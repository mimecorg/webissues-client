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

#ifndef LOCALECACHE_H
#define LOCALECACHE_H

#include "locale.h"

#include <QObject>

class Command;
class Reply;

/**
* Cache for locale information retrieved from the server.
*/
class LocaleCache : public QObject
{
    Q_OBJECT
public:
    /**
    * Constructor.
    */
    LocaleCache( QObject* parent );

    /**
    * Destructor.
    */
    ~LocaleCache();

public:
    bool isPopulated() const { return m_populated; }

    const QList<LocaleLanguage>& languages() const { return m_languages; }

    QList<LocaleFormat> formats( const QString& type ) const;

    const QList<LocaleTimeZone>& timeZones() const { return m_timeZones; }

    Command* updateLocale();

private slots:
    void updateLocaleReply( const Reply& reply );

private:
    bool m_populated;
    QList<LocaleLanguage> m_languages;
    QList<LocaleFormat> m_formats;
    QList<LocaleTimeZone> m_timeZones;
};

#endif
