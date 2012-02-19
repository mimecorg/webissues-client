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

#ifndef VALUEEDITORS_H
#define VALUEEDITORS_H

#include "abstractvalueeditor.h"

class DefinitionInfo;

/**
* Editor delegate using a single-line text control.
*/
class InputLineValueEditor : public AbstractValueEditor
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param info Attribute definition.
    * @param intial True if this is an initial value editor.
    * @param parent Parent of the delegate.
    * @param parentWidget Parent widget of the editor widget.
    */
    InputLineValueEditor( const DefinitionInfo& info, bool initial, int projectId, QObject* parent, QWidget* parentWidget );

    /**
    * Destructor.
    */
    ~InputLineValueEditor();

public: // overrides
    void setInputValue( const QString& value );
    QString inputValue();

private:
    void initializeText( const DefinitionInfo& info, bool initial, QWidget* parentWidget );
    void initializeEnum( const DefinitionInfo& info, bool initial, QWidget* parentWidget );
    void initializeNumeric( const DefinitionInfo& info, bool initial, QWidget* parentWidget );
    void initializeDateTime( const DefinitionInfo& info, bool initial, QWidget* parentWidget );
    void initializeUser( const DefinitionInfo& info, bool initial, int projectId, QWidget* parentWidget );
};

/**
* Editor delegate using a multi-line text control.
*/
class InputTextValueEditor : public AbstractValueEditor
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param info Attribute definition.
    * @param parent Parent of the delegate.
    * @param parentWidget Parent widget of the editor widget.
    */
    InputTextValueEditor( const DefinitionInfo& info, QObject* parent, QWidget* parentWidget );

    /**
    * Destructor.
    */
    ~InputTextValueEditor();

public: // overrides
    void setInputValue( const QString& value );
    QString inputValue();
};

#endif
