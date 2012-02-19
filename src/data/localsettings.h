/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2012 WebIssues Team
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

#ifndef LOCALSETTINGS_H
#define LOCALSETTINGS_H

#include <QVariant>

/**
* Restore option settings.
*/
enum RestoreOption
{
    RestoreNever,
    RestoreAuto,
    RestoreAlways
};

/**
* Action performed on an attachment.
*/
enum AttachmentAction
{
    ActionAsk,
    ActionOpen,
    ActionSaveAs
};

/**
* Class storing local settings for application or connection.
*/
class LocalSettings : public QObject
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param path The path of the file containing settings.
    * @param parent The parent object.
    */
    LocalSettings( const QString& path, QObject* parent );

    /**
    * Destructor.
    */
    ~LocalSettings();

public:
    typedef QList<int> IntList;
    typedef QMap<int, int> IntMap;

public:
    /**
    * Check if the given setting exists.
    * @param key The key of the setting to check.
    * @return @c true if the setting exists.
    */
    bool contains( const QString& key ) const;

    /**
    * Modify the value of the given setting.
    * @param key The key of the setting to modify.
    * @param value The new value of the setting.
    */
    void setValue( const QString& key, const QVariant& value );

    /**
    * Return the value of the given setting.
    * @param key The key of the setting to return.
    * @param defaultValue The value returned if key does not exist.
    * @param The value of the given key or the default value.
    */
    QVariant value( const QString& key, const QVariant& defaultValue = QVariant() ) const;

    /**
    * Load settings from the configuration file.
    */
    void load();

    /**
    * Save settings from the configuration file.
    */
    void save();

signals:
    /**
    * Emitted when settings are saved.
    */
    void settingsChanged();

private:
    QString m_path;

    QVariantMap m_data;
};

Q_DECLARE_METATYPE( LocalSettings::IntList )
Q_DECLARE_METATYPE( LocalSettings::IntMap )

#endif
