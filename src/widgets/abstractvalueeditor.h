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

#ifndef ABSTRACTVALUEEDITOR_H
#define ABSTRACTVALUEEDITOR_H

#include <QWidget>
#include <QPointer>

class QWidget;

/**
* Delegate for editing attribute values.
*
* This delegate creates a widget for editing attribute values and provides
* an interface to read and change the current value.
*
* This is an abstract class. Use the ValueEditorFactory to create
* delegates for given attribute types.
*
* Inherited classes must implement the setInputValue() and inputValue() methods.
*/
class AbstractValueEditor : public QObject
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent The parent object.
    */
    AbstractValueEditor( QObject* parent );

    /**
    * Destructor.
    */
    ~AbstractValueEditor();

public:
    /**
    * Return the widget created by this delegate.
    */
    QWidget* widget() const { return m_widget; };

public:
    /**
    * Set the current value of the editor's widget to @p value.
    */
    virtual void setInputValue( const QString& value ) = 0;

    /**
    * Return the current value of the editor's widget.
    */
    virtual QString inputValue() = 0;

    /**
    * Set the identifier of the project associated with the editor.
    */
    virtual void setProjectId( int projectId ) = 0;

protected:
    /**
    * Called by inherited classes to set the editor's widget.
    */
    void setWidget( QWidget* widget );

private:
    QPointer<QWidget> m_widget;
};

#endif
