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

#include "valueeditorfactory.h"
#include "valueeditors.h"

#include "utils/definitioninfo.h"
#include "utils/attributehelper.h"

AbstractValueEditor* ValueEditorFactory::createValueEditor( const DefinitionInfo& info, int projectId, QObject* parent, QWidget* parentWidget )
{
    switch ( AttributeHelper::toAttributeType( info ) ) {
        case TextAttribute:
            if ( info.metadata( "multi-line" ).toBool() )
                return new InputTextValueEditor( info, parent, parentWidget );
            break;

        case EnumAttribute:
            if ( !info.metadata( "editable" ).toBool() && !info.metadata( "multi-select" ).toBool() )
                return new ComboBoxValueEditor( info, projectId, parent, parentWidget );
            break;

        case UserAttribute:
            return new ComboBoxValueEditor( info, projectId, parent, parentWidget );

        default:
            break;
    }

    return new InputLineValueEditor( info, false, parent, parentWidget );
}

AbstractValueEditor* ValueEditorFactory::createInitialValueEditor( const DefinitionInfo& info, QObject* parent, QWidget* parentWidget )
{
    switch ( AttributeHelper::toAttributeType( info ) ) {
        case TextAttribute:
            if ( info.metadata( "multi-line" ).toBool() )
                return new InputTextValueEditor( info, parent, parentWidget );
            break;

        default:
            break;
    }

    return new InputLineValueEditor( info, true, parent, parentWidget );
}
