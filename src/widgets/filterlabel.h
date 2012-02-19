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

#ifndef FILTERLABEL_H
#define FILTERLABEL_H

#include <QLabel>

/**
* A label constisting of mutually exclusive links.
*/
class FilterLabel : public QLabel
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent The parent widget.
    */
    FilterLabel( QWidget* parent );

    /**
    * Destructor.
    */
    ~FilterLabel();

public:
    /**
    * Add a link item.
    */
    void addItem( const QString& item );

    /**
    * Set the index of the current link.
    */
    void setCurrentIndex( int index );

    /**
    * Return the index of the current link.
    */
    int currentIndex() const { return m_currentIndex; }

signals:
    /**
    * Emitted when the current link changed.
    */
    void currentIndexChanged( int index );

private slots:
    void itemActivated( const QString& link );

private:
    void updateText();

private:
    QStringList m_items;

    int m_currentIndex;
};

#endif
