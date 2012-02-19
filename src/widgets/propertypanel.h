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

#ifndef PROPERTYPANEL_H
#define PROPERTYPANEL_H

#include <QWidget>
#include <QMap>

class QGridLayout;
class QTextBrowser;

/**
* A panel containing pairs of labels and values.
*
* Values are displayed in read only KLineEdit controls so they can be selected
* and copied, but they look like regular labels.
*/
class PropertyPanel : public QWidget
{
    Q_OBJECT
public:
    /**
    * Conctructor.
    * @param parent The parent widget.
    */
    PropertyPanel( QWidget* parent );

    /**
    * Destructor.
    */
    ~PropertyPanel();

public:
    /**
    * Remove all properties.
    */
    void clear();

    /**
    * Add a property.
    * @param key A unique key of the property.
    * @param name The label of the property.
    */
    void addProperty( const QString& key, const QString& name );

    /**
    * Add a separator between properties.
    */
    void addSeparator();

    /**
    * Clear the values of all properties.
    */
    void clearValues();

    /**
    * Set the values of a given property.
    * @param key A unique key of the property.
    * @param value The value of the property.
    */
    void setValue( const QString& key, const QString& value );

    /**
    * Set the inner margin size.
    */
    void setInnerMargin( int margin );

public: // overrides
    bool eventFilter( QObject* watched, QEvent* e ); 

private slots:
    void browserContextMenu( const QPoint& pos );

private:
    QGridLayout* m_layout;

    QMap<QString, QTextBrowser*> m_browserMap;

    bool m_menu;
};

#endif
