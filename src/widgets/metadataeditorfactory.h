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

#ifndef METADATAEDITORFACTORY_H
#define METADATAEDITORFACTORY_H

#include "utils/attributehelper.h"

class AbstractMetadataEditor;

class QObject;
class QWidget;

/**
* Function creating delegates for editing attribute metadata.
*/
class MetadataEditorFactory
{
public:
    /**
    * Create an attribute metadata editor delegate for given attribute type.
    * @param type Type of the attribute.
    * @param parent Parent object of the editor delegate.
    * @param parentWidget Parent widget of the editor widget.
    * @return The created object or @c NULL if the definition is invalid.
    */
    static AbstractMetadataEditor* createMetadataEditor( AttributeType type, QObject* parent, QWidget* parentWidget );
};

#endif
