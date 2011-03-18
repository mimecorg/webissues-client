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

#ifndef VALUEEDITORFACTORY_H
#define VALUEEDITORFACTORY_H

class DefinitionInfo;
class AbstractValueEditor;

class QObject;
class QWidget;

/**
* Function creating delegates for editing attribute values.
*/
class ValueEditorFactory
{
public:
    /**
    * Create an attribute value editor delegate for given attribute definition.
    * @param info Attribute definition.
    * @param projectId Optional project identifier for attribute value context.
    * @param parent Parent object of the editor delegate.
    * @param parentWidget Parent widget of the editor widget.
    * @return The created object or @c NULL if the definition is invalid.
    */
    static AbstractValueEditor* createValueEditor( const DefinitionInfo& info, int projectId, QObject* parent, QWidget* parentWidget );

    /**
    * Create an initial value editor delegate for given attribute definition.
    * @param info Attribute definition.
    * @param parent Parent object of the editor delegate.
    * @param parentWidget Parent widget of the editor widget.
    * @return The created object or @c NULL if the definition is invalid.
    */
    static AbstractValueEditor* createInitialValueEditor( const DefinitionInfo& info, QObject* parent, QWidget* parentWidget );
};

#endif
