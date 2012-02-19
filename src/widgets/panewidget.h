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

#ifndef PANEWIDGET_H
#define PANEWIDGET_H

#include <QStackedWidget>

class QLabel;

/**
* Pane for displaying a widget or a placeholder text.
*/
class PaneWidget : public QStackedWidget
{
    Q_OBJECT
public:
    /**
    * Conctructor.
    * @param parent The parent widget.
    */
    PaneWidget( QWidget* parent );

    /**
    * Destructor.
    */
    ~PaneWidget();

public:
    /*
    * Set text displayed when widget is hidden.
    */
    void setPlaceholderText( const QString& text );

    /**
    * Set the widget contained in this pane.
    * The PaneWidget takes ownership of this widget.
    */
    void setMainWidget( QWidget* widget );

    /**
    * Show or hide the contained widget.
    */
    void setMainWidgetVisible( bool visible );

private:
    QLabel* m_label;
    QWidget* m_labelWidget;
    QWidget* m_mainWidget;
};

#endif
