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

#include "metadatadialog.h"

#include "utils/validator.h"
#include "utils/iconloader.h"
#include "widgets/abstractmetadataeditor.h"
#include "widgets/metadataeditorfactory.h"

#include <QLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>

MetadataDialog::MetadataDialog( AttributeType type, QWidget* parent ) : CommandDialog( parent ),
    m_type( type )
{
    AttributeHelper helper;

    setWindowTitle( tr( "Attribute Details" ) );
    setPromptPixmap( IconLoader::pixmap( "status-info", 22 ) );
    setPrompt( tr( "Specify details of <b>%1</b> attribute:" ).arg( helper.typeName( type ) ) );

    QVBoxLayout* layout = new QVBoxLayout();

    m_editor = MetadataEditorFactory::createMetadataEditor( type, this, this );

    layout->addWidget( m_editor->widget() );

    setContentLayout( layout, m_editor->isFixedHeight() );
}

MetadataDialog::~MetadataDialog()
{
}

void MetadataDialog::setMetadata( const QVariantMap& metadata )
{
    m_metadata = metadata;
    m_editor->setMetadata( metadata );
}

void MetadataDialog::accept()
{
    if ( !validate() )
        return;

    m_editor->updateMetadata( m_metadata );

    Validator validator;
    validator.createAttributeDefinition( m_type, m_metadata );
    if ( !validator.isValid() ) {
        showWarning( QString( "Incorrect value: %1." ).arg( validator.errors().first() ) );
        return;
    }

    QDialog::accept();
}
