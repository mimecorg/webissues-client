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

#ifndef TYPEDIALOGS_H
#define TYPEDIALOGS_H

#include "commanddialog.h"

#include "utils/definitioninfo.h"
#include "utils/attributehelper.h"

class AbstractValueEditor;
class InputLineEdit;

class QCheckBox;
class QComboBox;
class QGridLayout;
class QLineEdit;

/**
* Dialog for executing the <tt>ADD TYPE</tt> command.
*/
class AddTypeDialog : public CommandDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent The parent widget.
    */
    AddTypeDialog( QWidget* parent );

    /**
    * Destructor.
    */
    ~AddTypeDialog();

public: // overrides
    void accept();

private:
    InputLineEdit* m_nameEdit;
};

/**
* Dialog for executing the <tt>RENAME TYPE</tt> command.
*/
class RenameTypeDialog : public CommandDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param typeId Identifier of the type to rename.
    * @param parent The parent widget.
    */
    RenameTypeDialog( int typeId, QWidget* parent );

    /**
    * Destructor.
    */
    ~RenameTypeDialog();

public: // overrides
    void accept();

private:
    int m_typeId;
    QString m_oldName;

    InputLineEdit* m_nameEdit;
};

/**
* Dialog for executing the <tt>DELETE TYPE</tt> command.
*/
class DeleteTypeDialog : public CommandDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param typeId Identifier of the type to delete.
    * @param parent The parent widget.
    */
    DeleteTypeDialog( int typeId, QWidget* parent );

    /**
    * Destructor.
    */
    ~DeleteTypeDialog();

public: // overrides
    void accept();

protected: // overrides
    bool batchFailed( AbstractBatch* batch );

private:
    int m_typeId;

    bool m_force;
};

/**
* Base class for dialogs for adding or modifying attributes.
*/
class AttributeDialog : public CommandDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent The parent widget.
    */
    AttributeDialog( QWidget* parent );

    /**
    * Destructor.
    */
    ~AttributeDialog();

public: // overrides
    bool event( QEvent* e );

protected:
    /**
    * Initialize widgets for defining the attribute.
    * @param withName If @c true, also the widget for editing the name will be created.
    * @param info The initial definition of the attribute.
    */
    void initialize( bool withName, const DefinitionInfo& info );

    /**
    * Create the attribute definition from the widgets.
    */
    DefinitionInfo definitionInfo();

    /**
    * Return the name of the attribute.
    */
    QString attributeName() const;

private slots:
    void typeActivated( int index );
    void detailsClicked();
    void requiredToggled( bool on );

private:
    void updateWidgets();

private:
    QGridLayout* m_layout;
    InputLineEdit* m_nameEdit;
    QComboBox* m_typeCombo;
    QLineEdit* m_detailsEdit;
    QCheckBox* m_requiredCheck;
    QLabel* m_valueLabel;
    AbstractValueEditor* m_editor;

    AttributeType m_type;
    QVariantMap m_metadata;
};

/**
* Dialog for executing the <tt>ADD ATTRIBUTE</tt> command.
*/
class AddAttributeDialog : public AttributeDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param typeId Identifier of the type containing the attribute.
    * @param parent The parent widget.
    */
    AddAttributeDialog( int typeId, QWidget* parent );

    /**
    * Destructor.
    */
    ~AddAttributeDialog();

public: // overrides
    void accept();

private:
    int m_typeId;
};

/**
* Dialog for executing the <tt>MODIFY ATTRIBUTE</tt> command.
*/
class ModifyAttributeDialog : public AttributeDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param attributeId Identifier of the attribute to modify.
    * @param parent The parent widget.
    */
    ModifyAttributeDialog( int attributeId, QWidget* parent );

    /**
    * Destructor.
    */
    ~ModifyAttributeDialog();

public: // overrides
    void accept();

private:
    int m_attributeId;

    QString m_oldDefinition;
};

/**
* Dialog for executing the <tt>RENAME ATTRIBUTE</tt> command.
*/
class RenameAttributeDialog : public CommandDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param attributeId Identifier of the attribute to rename.
    * @param parent The parent widget.
    */
    RenameAttributeDialog( int attributeId, QWidget* parent );

    /**
    * Destructor.
    */
    ~RenameAttributeDialog();

public: // overrides
    void accept();

private:
    int m_attributeId;
    QString m_oldName;

    InputLineEdit* m_nameEdit;
};

/**
* Dialog for executing the <tt>DELETE ATTRIBUTE</tt> command.
*/
class DeleteAttributeDialog : public CommandDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param attributeId Identifier of the attribute to delete.
    * @param parent The parent widget.
    */
    DeleteAttributeDialog( int attributeId, QWidget* parent );

    /**
    * Destructor.
    */
    ~DeleteAttributeDialog();

public: // overrides
    void accept();

protected: // overrides
    bool batchFailed( AbstractBatch* batch );

private:
    int m_attributeId;

    bool m_force;
};

#endif
