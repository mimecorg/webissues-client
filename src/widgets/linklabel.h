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

#ifndef LINKLABEL_H
#define LINKLABEL_H

#include <QLabel>

/**
* A label which shows a link that is correctly grayed out when disabled.
*/
class LinkLabel : public QLabel
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param caption The caption of the link.
    * @param parent The parent widget.
    */
    LinkLabel( const QString& caption, QWidget* parent );

    /**
    * Destructor.
    */
    ~LinkLabel();

public:
    void setLinkUrl( const QString& url );
    const QString& linkUrl() const { return m_linkUrl; }

    void setLinkCaption( const QString& caption );
    const QString& linkCaption() const { return m_linkCaption; }

protected: // overrides
    void changeEvent( QEvent* e );

private:
    void updateLink();

private:
    QString m_linkUrl;
    QString m_linkCaption;
};

#endif
