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

#ifndef METADATAEDITORS_H
#define METADATAEDITORS_H

#include "abstractmetadataeditor.h"

class DefinitionInfo;

class QButtonGroup;
class NumericLineEdit;
class InputTextEdit;
class QCheckBox;
class QSpinBox;

/**
* Editor delegate for <tt>TEXT</tt> attribute metadata.
*/
class TextMetadataEditor : public AbstractMetadataEditor
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent Parent of the delegate.
    * @param parentWidget Parent widget of the editor widget.
    */
    TextMetadataEditor( QObject* parent, QWidget* parentWidget );

    /**
    * Destructor.
    */
    ~TextMetadataEditor();

public: // overrides
    void setMetadata( const QVariantMap& metadata );
    void updateMetadata( QVariantMap& metadata );

private:
    QCheckBox* m_multiLineCheckBox;
    NumericLineEdit* m_minLengthEdit;
    NumericLineEdit* m_maxLengthEdit;
};

/**
* Editor delegate for <tt>ENUM</tt> attribute metadata.
*/
class EnumMetadataEditor : public AbstractMetadataEditor
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent Parent of the delegate.
    * @param parentWidget Parent widget of the editor widget.
    */
    EnumMetadataEditor( QObject* parent, QWidget* parentWidget );

    /**
    * Destructor.
    */
    ~EnumMetadataEditor();

public: // overrides
    void setMetadata( const QVariantMap& metadata );
    void updateMetadata( QVariantMap& metadata );

private slots:
    void checkBoxToggled();

private:
    QCheckBox* m_editableCheckBox;
    QCheckBox* m_multiSelectCheckBox;
    InputTextEdit* m_itemsEdit;
    NumericLineEdit* m_minLengthEdit;
    NumericLineEdit* m_maxLengthEdit;
};

/**
* Editor delegate for <tt>NUMERIC</tt> attribute metadatas.
*/
class NumericMetadataEditor : public AbstractMetadataEditor
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent Parent of the delegate.
    * @param parentWidget Parent widget of the editor widget.
    */
    NumericMetadataEditor( QObject* parent, QWidget* parentWidget );

    /**
    * Destructor.
    */
    ~NumericMetadataEditor();

public: // overrides
    void setMetadata( const QVariantMap& metadata );
    void updateMetadata( QVariantMap& metadata );

private slots:
    void decimalValueChanged( int value );
    void stripToggled( bool checked );

private:
    QSpinBox* m_decimalSpinBox;
    QCheckBox* m_stripCheckBox;

    NumericLineEdit* m_minValueEdit;
    NumericLineEdit* m_maxValueEdit;
};

/**
* Editor delegate for <tt>DATETIME</tt> attribute metadatas.
*/
class DateTimeMetadataEditor : public AbstractMetadataEditor
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent Parent of the delegate.
    * @param parentWidget Parent widget of the editor widget.
    */
    DateTimeMetadataEditor( QObject* parent, QWidget* parentWidget );

    /**
    * Destructor.
    */
    ~DateTimeMetadataEditor();

public: // overrides
    void setMetadata( const QVariantMap& metadata );
    void updateMetadata( QVariantMap& metadata );

private:
    QButtonGroup* m_timeGroup;
};

/**
* Editor delegate for <tt>USER</tt> attribute metadatas.
*/
class UserMetadataEditor : public AbstractMetadataEditor
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent Parent of the delegate.
    * @param parentWidget Parent widget of the editor widget.
    */
    UserMetadataEditor( QObject* parent, QWidget* parentWidget );

    /**
    * Destructor.
    */
    ~UserMetadataEditor();

public: // overrides
    void setMetadata( const QVariantMap& metadata );
    void updateMetadata( QVariantMap& metadata );

private:
    QCheckBox* m_membersCheckBox;
    QCheckBox* m_multiSelectCheckBox;
};

#endif
