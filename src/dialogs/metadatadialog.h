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

#ifndef METADATADIALOG_H
#define METADATADIALOG_H

#include "dialogs/commanddialog.h"
#include "utils/attributehelper.h"

#include <QVariantMap>

class AbstractMetadataEditor;

class QLabel;

/**
* Dialog for editing attribute metadata.
*/
class MetadataDialog : public CommandDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent The parent widget.
    */
    MetadataDialog( AttributeType type, QWidget* parent );

    /**
    * Destructor.
    */
    ~MetadataDialog();

public:
    /**
    * Initialize metadata to @p metadata.
    */
    void setMetadata( const QVariantMap& metadata );

    /**
    * Return the edited metadata.
    */
    const QVariantMap& metadata() const { return m_metadata; }

public: // overrides
    void accept();

private:
    AttributeType m_type;
    QVariantMap m_metadata;

    AbstractMetadataEditor* m_editor;
};

#endif
