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

#ifndef SEPARATORCOMBOBOX_H
#define SEPARATORCOMBOBOX_H

#include <QComboBox>

/**
* A combo box with separators between items.
*/
class SeparatorComboBox : public QComboBox
{
    Q_OBJECT
public:
    /**
    * Conctructor.
    * @param parent The parent widget.
    */
    SeparatorComboBox( QWidget* parent );

    /**
    * Destructor.
    */
    ~SeparatorComboBox();

public:
    /**
    * Add a separator to the end of the list of existing items.
    */
    void addSeparator();

    /**
    * Add a parent item to the end of the list of existing items.
    */
    void addParentItem( const QString& text );

    /**
    * Add a child item to the end of the list of existing items.
    */
    void addChildItem( const QString& text, const QVariant& data = QVariant() );
};

#endif
