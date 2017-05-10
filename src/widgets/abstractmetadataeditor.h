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

#ifndef ABSTRACTMETADATAEDITOR_H
#define ABSTRACTMETADATAEDITOR_H

#include <QWidget>
#include <QPointer>
#include <QVariant>

/**
* Delegate for editing attribute metadata.
*
* This delegate creates a widget for editing attribute metadata and provides
* an interface to read and update the metadata.
*
* This is an abstract class. Use the MetadataEditorFactory to create
* delegates for given attribute types.
*/
class AbstractMetadataEditor : public QObject
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent The parent object.
    */
    AbstractMetadataEditor( QObject* parent );

    /**
    * Destructor.
    */
    ~AbstractMetadataEditor();

public:
    /**
    * Return the widget created by this delegate.
    */
    QWidget* widget() const { return m_widget; };

    /**
    * Return @c true if the widget has fixed height.
    */
    bool isFixedHeight() const { return m_fixedHeight; }

public:
    /**
    * Initialize metadata to @p metadata.
    */
    virtual void setMetadata( const QVariantMap& metadata ) = 0;

    /**
    * Read metadata into @p metadata.
    * @return @c true if successful or @c false if an error occurred.
    */
    virtual void updateMetadata( QVariantMap& metadata ) = 0;

protected:
    /**
    * Called by inherited classes to set the editor's widget.
    */
    void setWidget( QWidget* widget );

    /**
    * Make the layout use fixed height for the widget.
    */
    void setFixedHeight( bool fixed );

private:
    QPointer<QWidget> m_widget;
    bool m_fixedHeight;
};

#endif
