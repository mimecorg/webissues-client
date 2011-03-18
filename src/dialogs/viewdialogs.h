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

#ifndef VIEWDIALOGS_H
#define VIEWDIALOGS_H

#include "commanddialog.h"

#include <QMap>

class DefinitionInfo;
class InputLineEdit;

class QCheckBox;
class QComboBox;
class QButtonGroup;
class QSignalMapper;

/**
* Base class for dialogs for adding or modifying views.
*/
class ViewDialog : public CommandDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent The parent widget.
    */
    ViewDialog( QWidget* parent );

    /**
    * Destructor.
    */
    ~ViewDialog();

protected:
    /**
    * Initialize widgets for defining the view.
    * @param withName If @c true, also the widget for editing the name will be created.
    * @param withFilters If @c true, the tab for editing filter conditions will be created.
    * @param typeId Identifier of the type associated with the view.
    * @param info The initial definition of the view.
    */
    void initialize( bool withName, bool withFilters, int typeId, const DefinitionInfo& info );

    /**
    * Create the view definition from the widgets.
    */
    DefinitionInfo definitionInfo();

    /**
    * Return the name of the view.
    */
    QString viewName() const;

private slots:
    void columnToggled( int column );
    void columnIndexChanged( int column );

    void conditionToggled( int index );
    void filterToggled( int column );
    void conditionIndexChanged( int index );
    void filterIndexChanged( int column );

private:
    void updateColumnsLayout();

    void updateSortComboBox( int current = -1 );

    void appendCondition( int column, const QString& type, const QString& value );
    InputLineEdit* createEditor( int column );
    QStringList userItems();

    void updateFiltersLayout();

private:
    InputLineEdit* m_nameEdit;

    QWidget* m_columnsPanel;
    QList<QWidget*> m_columnWidgets;
    QMap<int, QCheckBox*> m_columnCheckBoxes;
    QMap<int, QComboBox*> m_columnComboBoxes;

    QList<int> m_selectedColumns;
    QMap<int, int> m_remainingOrder;

    QComboBox* m_sortComboBox;
    QButtonGroup* m_orderGroup;

    QWidget* m_filtersPanel;
    QList<QWidget*> m_filterWidgets;
    QList<QCheckBox*> m_conditionCheckBoxes;
    QList<QComboBox*> m_conditionOperators;
    QList<InputLineEdit*> m_conditionEditors;

    QList<int> m_availableColumns;
    QList<int> m_conditions;

    QSignalMapper* m_conditionToggledMapper;
    QSignalMapper* m_conditionIndexChangedMapper;

    QMap<int, QCheckBox*> m_filterCheckBoxes;
    QMap<int, QComboBox*> m_filterOperators;
    QMap<int, InputLineEdit*> m_filterEditors;

    bool m_updatingLayout;
};

/**
* Dialog for executing the <tt>ADD VIEW</tt> command.
*/
class AddViewDialog : public ViewDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param typeId Identifier of the type associated with the view.
    * @param parent The parent widget.
    */
    AddViewDialog( int typeId, bool isPublic, QWidget* parent );

    /**
    * Destructor.
    */
    ~AddViewDialog();

public:
    int viewId() const { return m_viewId; }

public: // overrides
    void accept();

protected: // overrides
    bool batchSuccessful( AbstractBatch* batch );

private:
    int m_typeId;
    bool m_isPublic;

    int m_viewId;
};

/**
* Dialog for executing the <tt>MODIFY VIEW</tt> command.
*/
class ModifyViewDialog : public ViewDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param viewId Identifier of the view to modify.
    * @param parent The parent widget.
    */
    ModifyViewDialog( int viewId, QWidget* parent );

    /**
    * Destructor.
    */
    ~ModifyViewDialog();

public: // overrides
    void accept();

private:
    int m_viewId;

    QString m_oldDefinition;
};

/**
* Dialog for modifying the default view setting.
*/
class DefaultViewDialog : public ViewDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param typeId Identifier of the type associated with view setting.
    * @param parent The parent widget.
    */
    DefaultViewDialog( int typeId, QWidget* parent );

    /**
    * Destructor.
    */
    ~DefaultViewDialog();

public: // overrides
    void accept();

private:
    int m_typeId;

    QString m_oldDefinition;
};

/**
* Dialog for executing the <tt>RENAME VIEW</tt> command.
*/
class RenameViewDialog : public CommandDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param viewId Identifier of the view to rename.
    * @param parent The parent widget.
    */
    RenameViewDialog( int viewId, QWidget* parent );

    /**
    * Destructor.
    */
    ~RenameViewDialog();

public: // overrides
    void accept();

private:
    int m_viewId;
    QString m_oldName;

    InputLineEdit* m_nameEdit;
};

/**
* Dialog for executing the <tt>DELETE VIEW</tt> command.
*/
class DeleteViewDialog : public CommandDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param viewId Identifier of the view to delete.
    * @param parent The parent widget.
    */
    DeleteViewDialog( int viewId, QWidget* parent );

    /**
    * Destructor.
    */
    ~DeleteViewDialog();

public: // overrides
    void accept();

private:
    int m_viewId;
};

/**
* Dialog for executing the <tt>PUBLISH VIEW</tt> command.
*/
class PublishViewDialog : public CommandDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param viewId Identifier of the view to publish or unpublished.
    * @param isPublic @c true if the view is published, @c false otherwise.
    * @param parent The parent widget.
    */
    PublishViewDialog( int viewId, bool isPublic, QWidget* parent );

    /**
    * Destructor.
    */
    ~PublishViewDialog();

public: // overrides
    void accept();

private:
    int m_viewId;
    bool m_isPublic;
};

/**
* Dialog for modifying the order of attributes.
*/
class AttributeOrderDialog : public CommandDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param typeId Identifier of the type associated with view setting.
    * @param parent The parent widget.
    */
    AttributeOrderDialog( int typeId, QWidget* parent );

    /**
    * Destructor.
    */
    ~AttributeOrderDialog();

public: // overrides
    void accept();

private slots:
    void indexChanged( int attributeId );

private:
    void updateLayout();

private:
    int m_typeId;

    QWidget* m_panel;
    QList<QWidget*> m_widgets;
    QMap<int, QLabel*> m_labels;
    QMap<int, QComboBox*> m_comboBoxes;

    QList<int> m_attributes;

    bool m_updatingLayout;
};

#endif
