/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2013 WebIssues Team
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

#include "viewdialogs.h"

#include "application.h"
#include "commands/viewsbatch.h"
#include "data/datamanager.h"
#include "data/entities.h"
#include "data/issuetypecache.h"
#include "data/localsettings.h"
#include "models/foldermodel.h"
#include "utils/definitioninfo.h"
#include "utils/viewsettingshelper.h"
#include "utils/validator.h"
#include "utils/errorhelper.h"
#include "utils/iconloader.h"
#include "widgets/inputlineedit.h"
#include "widgets/separatorcombobox.h"

#include <QLayout>
#include <QLabel>
#include <QTabWidget>
#include <QGroupBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QComboBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSignalMapper>

ViewDialog::ViewDialog( QWidget* parent ) : CommandDialog( parent ),
    m_typeId( 0 ),
    m_nameEdit( NULL ),
    m_filtersPanel( NULL ),
    m_updatingLayout( false )
{
}

ViewDialog::~ViewDialog()
{
    application->applicationSettings()->setValue( "ViewDialogSize", size() );
}

static QFrame* makeSeparator( QWidget* parent )
{
    QFrame* separator = new QFrame( parent );
    separator->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    return separator;
}

void ViewDialog::initialize( bool withName, bool withFilters, int typeId, const DefinitionInfo& info )
{
    m_typeId = typeId;

    QGridLayout* layout = new QGridLayout();
    layout->setColumnStretch( 1, 1 );

    if ( withName ) {
        QLabel* nameLabel = new QLabel( tr( "&Name:" ), this );
        layout->addWidget( nameLabel, 0, 0 );

        m_nameEdit = new InputLineEdit( this );
        m_nameEdit->setMaxLength( 40 );
        m_nameEdit->setRequired( true );
        layout->addWidget( m_nameEdit, 0, 1 );

        nameLabel->setBuddy( m_nameEdit );
    }

    QTabWidget* tabWidget = new QTabWidget( this );
    layout->addWidget( tabWidget, withName ? 1 : 0, 0, 1, 2 );

    QWidget* generalTab = new QWidget( tabWidget );
    QVBoxLayout* generalLayout = new QVBoxLayout( generalTab );
    tabWidget->addTab( generalTab, IconLoader::icon( "view" ), tr( "General" ) );

    QGroupBox* columnsGroup = new QGroupBox( tr( "Columns" ), this );
    QVBoxLayout* columnsLayout = new QVBoxLayout( columnsGroup );
    generalLayout->addWidget( columnsGroup );

    QLabel* columnsLabel = new QLabel( tr( "Select columns which will be displayed in the list of issues:" ), columnsGroup );
    columnsLayout->addWidget( columnsLabel );

    QScrollArea* columnsScroll = new QScrollArea( columnsGroup );
    columnsScroll->setWidgetResizable( true );
    columnsLayout->addWidget( columnsScroll );

    QPalette scrollPalette = palette();
    scrollPalette.setColor( QPalette::Window, QColor::fromRgb( 255, 255, 255 ) );
    columnsScroll->setPalette( scrollPalette );

    m_columnsPanel = new QWidget( columnsScroll );
    columnsScroll->setWidget( m_columnsPanel );

    m_columnWidgets.append( new QLabel( tr( "Selected columns:" ), m_columnsPanel ) );
    m_columnWidgets.append( new QLabel( tr( "Order" ), m_columnsPanel ) );
    m_columnWidgets.append( makeSeparator( m_columnsPanel ) );
    m_columnWidgets.append( new QLabel( tr( "Available columns:" ), m_columnsPanel ) );
    m_columnWidgets.append( makeSeparator( m_columnsPanel ) );

    IssueTypeCache* cache = dataManager->issueTypeCache( typeId );

    m_availableColumns = cache->availableColumns();
    m_selectedColumns = cache->viewColumns( info );

    QSignalMapper* columnToggledMapper = new QSignalMapper( this );
    connect( columnToggledMapper, SIGNAL( mapped( int ) ), this, SLOT( columnToggled( int ) ) );

    QSignalMapper* columnIndexChangedMapper = new QSignalMapper( this );
    connect( columnIndexChangedMapper, SIGNAL( mapped( int ) ), this, SLOT( columnIndexChanged( int ) ) );

    ViewSettingsHelper helper( typeId );

    for ( int i = 0; i < m_availableColumns.count(); i++ ) {
        int column = m_availableColumns.at( i );

        QCheckBox* columnCheckBox = new QCheckBox( helper.columnName( column ), m_columnsPanel );
        m_columnCheckBoxes.insert( column, columnCheckBox );

        columnToggledMapper->setMapping( columnCheckBox, column );
        connect( columnCheckBox, SIGNAL( toggled( bool ) ), columnToggledMapper, SLOT( map() ) );

        QComboBox* columnComboBox = new QComboBox( m_columnsPanel );
        m_columnComboBoxes.insert( column, columnComboBox );

        for ( int j = 1; j <= m_availableColumns.count(); j++ )
            columnComboBox->addItem( QString::number( j ) );

        columnIndexChangedMapper->setMapping( columnComboBox, column );
        connect( columnComboBox, SIGNAL( currentIndexChanged( int ) ), columnIndexChangedMapper, SLOT( map() ) );

        if ( i < 2 ) {
            columnCheckBox->setEnabled( false );
            columnComboBox->setEnabled( false );
        }
    }

    updateColumnsLayout();

    QGroupBox* sortGroup = new QGroupBox( tr( "Sort Order" ), this );
    QGridLayout* sortLayout = new QGridLayout( sortGroup );
    generalLayout->addWidget( sortGroup );

    QLabel* sortLabel = new QLabel( tr( "Select the default sort order:" ), sortGroup );
    sortLayout->addWidget( sortLabel, 0, 0, 1, 2 );

    QLabel* sortColumnLabel = new QLabel( tr( "Colu&mn:" ), sortGroup );
    sortLayout->addWidget( sortColumnLabel, 1, 0 );

    QHBoxLayout* sortColumnLayout = new QHBoxLayout();
    sortLayout->addLayout( sortColumnLayout, 1, 1 );

    m_sortComboBox = new QComboBox( sortGroup );
    m_sortComboBox->setMinimumWidth( 150 );
    sortColumnLayout->addWidget( m_sortComboBox );

    sortColumnLabel->setBuddy( m_sortComboBox );

    sortColumnLayout->addStretch( 1 );

    QLabel* sortOrderLabel = new QLabel( tr( "Order:" ), sortGroup );
    sortLayout->addWidget( sortOrderLabel, 2, 0 );

    QHBoxLayout* sortOrderLayout = new QHBoxLayout();
    sortLayout->addLayout( sortOrderLayout, 2, 1 );

    QRadioButton* ascendingRadioButton = new QRadioButton( tr( "&Ascending" ), sortGroup );
    sortOrderLayout->addWidget( ascendingRadioButton );

    QRadioButton* descendingRadioButton = new QRadioButton( tr( "&Descending" ), sortGroup );
    sortOrderLayout->addWidget( descendingRadioButton );

    sortOrderLayout->addStretch( 1 );

    m_orderGroup = new QButtonGroup( sortGroup );
    m_orderGroup->addButton( ascendingRadioButton, Qt::AscendingOrder );
    m_orderGroup->addButton( descendingRadioButton, Qt::DescendingOrder );

    QPair<int, Qt::SortOrder> order = cache->viewSortOrder( info );
    updateSortComboBox( order.first );
    m_orderGroup->button( order.second )->setChecked( true );

    if ( withFilters ) {
        QWidget* filtersTab = new QWidget( tabWidget );
        QVBoxLayout* filtersTabLayout = new QVBoxLayout( filtersTab );
        tabWidget->addTab( filtersTab, IconLoader::icon( "filter" ), tr( "Filters" ) );

        QGroupBox* filtersGroup = new QGroupBox( tr( "Filter Conditions" ), this );
        QVBoxLayout* filtersLayout = new QVBoxLayout( filtersGroup );
        filtersTabLayout->addWidget( filtersGroup );

        QLabel* filtersLabel = new QLabel( tr( "Select conditions which will be used to filter displayed issues:" ), filtersGroup );
        filtersLayout->addWidget( filtersLabel );

        QScrollArea* filtersScroll = new QScrollArea( filtersGroup );
        filtersScroll->setWidgetResizable( true );
        filtersLayout->addWidget( filtersScroll );

        filtersScroll->setPalette( scrollPalette );

        m_filtersPanel = new QWidget( filtersScroll );
        filtersScroll->setWidget( m_filtersPanel );

        m_filterWidgets.append( new QLabel( tr( "Active conditions:" ), m_filtersPanel ) );
        m_filterWidgets.append( makeSeparator( m_filtersPanel ) );
        m_filterWidgets.append( new QLabel( tr( "Available conditions:" ), m_filtersPanel ) );
        m_filterWidgets.append( makeSeparator( m_filtersPanel ) );

        m_conditionToggledMapper = new QSignalMapper( this );
        connect( m_conditionToggledMapper, SIGNAL( mapped( int ) ), this, SLOT( conditionToggled( int ) ) );

        m_conditionIndexChangedMapper = new QSignalMapper( this );
        connect( m_conditionIndexChangedMapper, SIGNAL( mapped( int ) ), this, SLOT( conditionIndexChanged( int ) ) );

        QList<DefinitionInfo> filters = cache->viewFilters( info );

        for ( int i = 0; i < filters.count(); i++ ) {
            DefinitionInfo filter = filters.at( i );
            appendCondition( filter.metadata( "column" ).toInt(), filter.type(), filter.metadata( "value" ).toString() );
        }

        QSignalMapper* filterToggledMapper = new QSignalMapper( this );
        connect( filterToggledMapper, SIGNAL( mapped( int ) ), this, SLOT( filterToggled( int ) ) );

        QSignalMapper* filterIndexChangedMapper = new QSignalMapper( this );
        connect( filterIndexChangedMapper, SIGNAL( mapped( int ) ), this, SLOT( filterIndexChanged( int ) ) );

        for ( int i = 0; i < m_availableColumns.count(); i++ ) {
            int column = m_availableColumns.at( i );

            QCheckBox* checkBox = new QCheckBox( helper.columnName( column ), m_filtersPanel );
            m_filterCheckBoxes.insert( column, checkBox );

            filterToggledMapper->setMapping( checkBox, column );
            connect( checkBox, SIGNAL( toggled( bool ) ), filterToggledMapper, SLOT( map() ) );

            QComboBox* operatorsComboBox = new QComboBox( m_filtersPanel );
            QStringList operators = cache->availableOperators( column );
            for ( int j = 0; j < operators.count(); j++ )
                operatorsComboBox->addItem( helper.operatorName( operators.at( j ) ), operators.at( j ) );
            m_filterOperators.insert( column, operatorsComboBox );

            filterIndexChangedMapper->setMapping( operatorsComboBox, column );
            connect( operatorsComboBox, SIGNAL( currentIndexChanged( int ) ), filterIndexChangedMapper, SLOT( map() ) );

            InputLineEdit* editor = createEditor( column );
            m_filterEditors.insert( column, editor );
        }

        updateFiltersLayout();
    }

    setContentLayout( layout, false );

    showInfo( tr( "Edit view settings." ) );

    if ( withName )
        m_nameEdit->setFocus();

    resize( application->applicationSettings()->value( "ViewDialogSize", QSize( 600, 600 ) ).toSize() );
}

void ViewDialog::columnToggled( int column )
{
    if ( m_updatingLayout )
        return;

    bool checked = m_columnCheckBoxes.value( column )->isChecked();

    if ( checked ) {
        int index = m_columnComboBoxes.value( column )->currentIndex();
        index = qBound( 2, index, m_selectedColumns.count() );

        m_selectedColumns.insert( index, column );

        m_remainingOrder.remove( column );
    } else {
        m_selectedColumns.removeOne( column );
    }

    updateColumnsLayout();
    updateSortComboBox();
}

void ViewDialog::columnIndexChanged( int column )
{
    if ( m_updatingLayout )
        return;

    bool checked = m_columnCheckBoxes.value( column )->isChecked();
    int index = m_columnComboBoxes.value( column )->currentIndex();

    if ( checked ) {
        index = qBound( 2, index, m_selectedColumns.count() - 1 );

        m_selectedColumns.removeOne( column );
        m_selectedColumns.insert( index, column );

        updateColumnsLayout();
        updateSortComboBox();
    } else {
        m_remainingOrder.insert( column, index );
    }
}

void ViewDialog::updateColumnsLayout()
{
    m_updatingLayout = true;

    delete m_columnsPanel->layout();

    QGridLayout* layout = new QGridLayout( m_columnsPanel );

    layout->setColumnMinimumWidth( 0, 150 ),
    layout->setColumnMinimumWidth( 1, 70 ),
    layout->setColumnStretch( 2, 1 );

    layout->addWidget( m_columnWidgets.at( 0 ), 0, 0 );
    layout->addWidget( m_columnWidgets.at( 1 ), 0, 1 );
    layout->addWidget( m_columnWidgets.at( 2 ), 1, 0, 1, 2 );

    int row = 2;
    int order = 0;

    QList<int> remainingColumns = m_availableColumns;

    for ( int i = 0; i < m_selectedColumns.count(); i++ ) {
        int column = m_selectedColumns.at( i );

        QCheckBox* checkBox = m_columnCheckBoxes.value( column );
        checkBox->setChecked( true );
        layout->addWidget( checkBox, row, 0 );

        QComboBox* comboBox = m_columnComboBoxes.value( column );
        comboBox->setCurrentIndex( order++ );
        layout->addWidget( comboBox, row++, 1 );

        remainingColumns.removeOne( column );
    }

    if ( !remainingColumns.isEmpty() ) {
        layout->setRowMinimumHeight( row++, 10 );
        layout->addWidget( m_columnWidgets.at( 3 ), row++, 0 );
        layout->addWidget( m_columnWidgets.at( 4 ), row++, 0, 1, 2 );

        for ( int i = 0; i < remainingColumns.count(); i++ ) {
            int column = remainingColumns.at( i );

            QCheckBox* checkBox = m_columnCheckBoxes.value( column );
            checkBox->setChecked( false );
            layout->addWidget( checkBox, row, 0 );

            int index = m_remainingOrder.value( column, order++ );

            QComboBox* comboBox = m_columnComboBoxes.value( column );
            comboBox->setCurrentIndex( index );
            layout->addWidget( comboBox, row++, 1 );
        }
    }

    layout->setRowStretch( row, 1 );

    m_columnWidgets.at( 3 )->setVisible( !remainingColumns.isEmpty() );
    m_columnWidgets.at( 4 )->setVisible( !remainingColumns.isEmpty() );

    m_updatingLayout = false;
}

void ViewDialog::updateSortComboBox( int current )
{
    if ( current < 0 )
        current = m_sortComboBox->itemData( m_sortComboBox->currentIndex() ).toInt();

    m_sortComboBox->clear();

    ViewSettingsHelper helper( m_typeId );

    for ( int i = 0; i < m_selectedColumns.count(); i++ ) {
        int column = m_selectedColumns.at( i );
        m_sortComboBox->addItem( helper.columnName( column ), column );
        if ( column == current )
            m_sortComboBox->setCurrentIndex( i );
    }
}

void ViewDialog::conditionToggled( int index )
{
    if ( m_updatingLayout )
        return;

    bool checked = m_conditionCheckBoxes.at( index )->isChecked();
    if ( checked )
        return;

    m_conditionCheckBoxes.takeAt( index )->deleteLater();
    m_conditionOperators.takeAt( index )->deleteLater();
    m_conditionEditors.takeAt( index )->deleteLater();
    m_conditions.removeAt( index );

    updateFiltersLayout();
}

void ViewDialog::filterToggled( int column )
{
    if ( m_updatingLayout )
        return;

    QCheckBox* checkBox = m_filterCheckBoxes.value( column );
    if ( !checkBox->isChecked() )
        return;

    QComboBox* operatorsComboBox = m_filterOperators.value( column );
    InputLineEdit* editor = m_filterEditors.value( column );

    appendCondition( column, operatorsComboBox->itemData( operatorsComboBox->currentIndex() ).toString(), editor->inputValue() );

    checkBox->setChecked( false );
    operatorsComboBox->setCurrentIndex( 0 );
    editor->setInputValue( QString() );

    updateFiltersLayout();
}

static void updateEditor( InputLineEdit* editor, const QString& type )
{
    editor->setRequired( type != QLatin1String( "EQ" ) && type != QLatin1String( "NEQ" ) );

    if ( EnumLineEdit* enumEditor = qobject_cast<EnumLineEdit*>( editor ) )
        enumEditor->setMultiSelect( type == QLatin1String( "IN" ) );
}

void ViewDialog::conditionIndexChanged( int index )
{
    QComboBox* operatorsComboBox = m_conditionOperators.at( index );
    QString type = operatorsComboBox->itemData( operatorsComboBox->currentIndex() ).toString();
    updateEditor( m_conditionEditors.at( index ), type );
}

void ViewDialog::filterIndexChanged( int column )
{
    QComboBox* operatorsComboBox = m_filterOperators.value( column );
    QString type = operatorsComboBox->itemData( operatorsComboBox->currentIndex() ).toString();
    updateEditor( m_filterEditors.value( column ), type );
}

void ViewDialog::appendCondition( int column, const QString& type, const QString& value )
{
    IssueTypeCache* cache = dataManager->issueTypeCache( m_typeId );
    ViewSettingsHelper helper( m_typeId );

    QCheckBox* checkBox = new QCheckBox( helper.columnName( column ), m_filtersPanel );
    checkBox->setChecked( true );
    m_conditionCheckBoxes.append( checkBox );

    connect( checkBox, SIGNAL( toggled( bool ) ), m_conditionToggledMapper, SLOT( map() ) );

    QComboBox* operatorsComboBox = new QComboBox( m_filtersPanel );
    QStringList operators = cache->availableOperators( column );
    for ( int j = 0; j < operators.count(); j++ )
        operatorsComboBox->addItem( helper.operatorName( operators.at( j ) ), operators.at( j ) );
    operatorsComboBox->setCurrentIndex( operators.indexOf( type ) );
    m_conditionOperators.append( operatorsComboBox );

    connect( operatorsComboBox, SIGNAL( currentIndexChanged( int ) ), m_conditionIndexChangedMapper, SLOT( map() ) );

    InputLineEdit* editor = createEditor( column );
    updateEditor( editor, type );
    editor->setInputValue( value );
    m_conditionEditors.append( editor );

    m_conditions.append( column );
}

InputLineEdit* ViewDialog::createEditor( int column )
{
    if ( column > Column_UserDefined ) {
        int attributeId = column - Column_UserDefined;

        IssueTypeCache* cache = dataManager->issueTypeCache( m_typeId );
        DefinitionInfo attributeInfo = cache->attributeDefinition( attributeId );

        if ( !attributeInfo.isEmpty() ) {
            switch ( AttributeHelper::toAttributeType( attributeInfo ) ) {
                case TextAttribute: {
                    EnumLineEdit* editor = new EnumLineEdit( m_filtersPanel );
                    editor->setFunctions( InputLineEdit::MeFunction );
                    return editor;
                }

                case EnumAttribute: {
                    EnumLineEdit* editor = new EnumLineEdit( m_filtersPanel );
                    editor->setFunctions( InputLineEdit::MeFunction );
                    editor->setItems( attributeInfo.metadata( "items" ).toStringList() );
                    return editor;
                }

                case UserAttribute: {
                    EnumLineEdit* editor = new EnumLineEdit( m_filtersPanel );
                    editor->setFunctions( InputLineEdit::MeFunction );
                    editor->setItems( userItems() );
                    return editor;
                }

                case NumericAttribute: {
                    NumericLineEdit* editor = new NumericLineEdit( m_filtersPanel );
                    editor->setDecimal( attributeInfo.metadata( "decimal" ).toInt() );
                    editor->setStripZeros( attributeInfo.metadata( "strip" ).toBool() );
                    return editor;
                }

                case DateTimeAttribute: {
                    DateTimeLineEdit* editor = new DateTimeLineEdit( m_filtersPanel );
                    editor->setFunctions( DateTimeLineEdit::TodayFunction );
                    return editor;
                }

                default:
                    break;
            }
        }
    }

    switch ( column ) {
        case Column_ID:
            return new NumericLineEdit( m_filtersPanel );

        case Column_Name: {
            EnumLineEdit* editor = new EnumLineEdit( m_filtersPanel );
            editor->setFunctions( InputLineEdit::MeFunction );
            return editor;
        }

        case Column_CreatedBy:
        case Column_ModifiedBy: {
            EnumLineEdit* editor = new EnumLineEdit( m_filtersPanel );
            editor->setFunctions( InputLineEdit::MeFunction );
            editor->setItems( userItems() );
            return editor;
        }

        case Column_CreatedDate:
        case Column_ModifiedDate: {
            DateTimeLineEdit* editor = new DateTimeLineEdit( m_filtersPanel );
            editor->setFunctions( DateTimeLineEdit::TodayFunction );
            return editor;
        }

        default:
            break;
    }

    return new InputLineEdit( m_filtersPanel );
}

QStringList ViewDialog::userItems()
{
    QStringList items;

    items.append( QString( "[%1]" ).arg( tr( "Me" ) ) );

    QList<UserEntity> users = UserEntity::list();

    foreach ( const UserEntity& user, users ) {
        if ( user.access() == NoAccess )
            continue;
        items.append( user.name() );
    }

    return items;
}

void ViewDialog::updateFiltersLayout()
{
    m_updatingLayout = true;

    delete m_filtersPanel->layout();

    QGridLayout* layout = new QGridLayout( m_filtersPanel );

    layout->setColumnMinimumWidth( 0, 150 ),
    layout->setColumnStretch( 2, 1 );

    int row = 0;

    if ( !m_conditions.isEmpty() ) {
        layout->addWidget( m_filterWidgets.at( 0 ), row++, 0 );
        layout->addWidget( m_filterWidgets.at( 1 ), row++, 0, 1, 3 );

        for ( int i = 0; i < m_conditions.count(); i++ ) {
            QCheckBox* checkBox = m_conditionCheckBoxes.at( i );
            layout->addWidget( checkBox, row, 0 );

            m_conditionToggledMapper->setMapping( checkBox, i );

            QComboBox* operatorsComboBox = m_conditionOperators.at( i );
            layout->addWidget( operatorsComboBox, row, 1 );

            m_conditionIndexChangedMapper->setMapping( operatorsComboBox, i );

            InputLineEdit* editor = m_conditionEditors.at( i );
            layout->addWidget( editor, row++, 2 );
        }
    }

    layout->addWidget( m_filterWidgets.at( 2 ), row++, 0 );
    layout->addWidget( m_filterWidgets.at( 3 ), row++, 0, 1, 3 );

    for ( int i = 0; i < m_availableColumns.count(); i++ ) {
        int column = m_availableColumns.at( i );

        QCheckBox* checkBox = m_filterCheckBoxes.value( column );
        layout->addWidget( checkBox, row, 0 );

        QComboBox* operatorsComboBox = m_filterOperators.value( column );
        layout->addWidget( operatorsComboBox, row, 1 );

        InputLineEdit* editor = m_filterEditors.value( column );
        layout->addWidget( editor, row++, 2 );
    }

    layout->setRowStretch( row, 1 );

    m_filterWidgets.at( 0 )->setVisible( !m_conditions.isEmpty() );
    m_filterWidgets.at( 1 )->setVisible( !m_conditions.isEmpty() );

    for ( int i = 0; i < m_conditions.count(); i++ ) {
        m_conditionCheckBoxes.at( i )->show();
        m_conditionOperators.at( i )->show();
        m_conditionEditors.at( i )->show();
    }

    m_updatingLayout = false;
}

DefinitionInfo ViewDialog::definitionInfo()
{
    DefinitionInfo info;
    info.setType( "VIEW" );

    QStringList columns;
    for ( int i = 0; i < m_selectedColumns.count(); i++ )
        columns.append( QString::number( m_selectedColumns.at( i ) ) );

    info.setMetadata( "columns", columns.join( "," ) );

    info.setMetadata( "sort-column", m_sortComboBox->itemData( m_sortComboBox->currentIndex() ).toInt() );
    if ( m_orderGroup->button( Qt::DescendingOrder )->isChecked() )
        info.setMetadata( "sort-desc", 1 );

    if ( m_filtersPanel != NULL && !m_conditions.isEmpty() ) {
        QStringList filters;
        for ( int i = 0; i < m_conditions.count(); i++ ) {
            DefinitionInfo filter;
            QComboBox* operatorsComboBox = m_conditionOperators.at( i );
            filter.setType( operatorsComboBox->itemData( operatorsComboBox->currentIndex() ).toString() );
            filter.setMetadata( "column", m_conditions.at( i ) );
            filter.setMetadata( "value", m_conditionEditors.at( i )->inputValue() );
            filters.append( filter.toString() );
        }
        info.setMetadata( "filters", filters );
    }

    return info;
}

void ViewDialog::setViewName( const QString& name )
{
    m_nameEdit->setInputValue( name );
}

QString ViewDialog::viewName() const
{
    return m_nameEdit->inputValue();
}

AddViewDialog::AddViewDialog( int typeId, bool isPublic, QWidget* parent ) : ViewDialog( parent ),
    m_typeId( typeId ),
    m_isPublic( isPublic ),
    m_viewId( 0 )
{
    TypeEntity type = TypeEntity::find( typeId );

    if ( isPublic ) {
        setWindowTitle( tr( "Add Public View" ) );
        setPrompt( tr( "Create a new public view for type <b>%1</b>:" ).arg( type.name() ) );
    } else {
        setWindowTitle( tr( "Add Personal View" ) );
        setPrompt( tr( "Create a new personal view for type <b>%1</b>:" ).arg( type.name() ) );
    }
    setPromptPixmap( IconLoader::pixmap( "view-new", 22 ) );

    IssueTypeCache* cache = dataManager->issueTypeCache( typeId );

    initialize( true, true, typeId, cache->defaultView() );
}

AddViewDialog::~AddViewDialog()
{
}

void AddViewDialog::accept()
{
    if ( !validate() )
        return;

    QString name = viewName();

    if ( ViewEntity::exists( m_typeId, name, m_isPublic ) ) {
        showWarning( ErrorHelper::ViewAlreadyExists );
        return;
    }

    DefinitionInfo info = definitionInfo();

    ViewsBatch* batch = new ViewsBatch();
    batch->addView( m_typeId, name, info.toString(), m_isPublic );

    executeBatch( batch );
}

bool AddViewDialog::batchSuccessful( AbstractBatch* batch )
{
    m_viewId = ( (ViewsBatch*)batch )->viewId();

    return true;
}

CloneViewDialog::CloneViewDialog( int viewId, bool isPublic, QWidget* parent ) : ViewDialog( parent ),
    m_viewId( viewId ),
    m_isPublic( isPublic )
{
    ViewEntity view = ViewEntity::find( viewId );

    setWindowTitle( tr( "Clone View" ) );
    if ( isPublic )
        setPrompt( tr( "Clone view <b>%1</b> as a new public view:" ).arg( view.name() ) );
    else
        setPrompt( tr( "Clone view <b>%1</b> as a new personal  view:" ).arg( view.name() ) );
    setPromptPixmap( IconLoader::pixmap( "view-clone", 22 ) );

    m_typeId = view.typeId();

    initialize( true, true, m_typeId, view.definition() );
    setViewName( view.name() );
}

CloneViewDialog::~CloneViewDialog()
{
}

void CloneViewDialog::accept()
{
    if ( !validate() )
        return;

    QString name = viewName();

    if ( ViewEntity::exists( m_typeId, name, m_isPublic ) ) {
        showWarning( ErrorHelper::ViewAlreadyExists );
        return;
    }

    DefinitionInfo info = definitionInfo();

    ViewsBatch* batch = new ViewsBatch();
    batch->addView( m_typeId, name, info.toString(), m_isPublic );

    executeBatch( batch );
}

bool CloneViewDialog::batchSuccessful( AbstractBatch* batch )
{
    m_viewId = ( (ViewsBatch*)batch )->viewId();

    return true;
}

ModifyViewDialog::ModifyViewDialog( int viewId, QWidget* parent ) : ViewDialog( parent ),
    m_viewId( viewId )
{
    ViewEntity view = ViewEntity::find( viewId );

    if ( view.isPublic() ) {
        setWindowTitle( tr( "Modify Public View" ) );
        setPrompt( tr( "Modify the public view <b>%1</b>:" ).arg( view.name() ) );
    } else {
        setWindowTitle( tr( "Modify Personal View" ) );
        setPrompt( tr( "Modify your personal view <b>%1</b>:" ).arg( view.name() ) );
    }
    setPromptPixmap( IconLoader::pixmap( "edit-modify", 22 ) );

    initialize( false, true, view.typeId(), view.definition() );
}

ModifyViewDialog::~ModifyViewDialog()
{
}

void ModifyViewDialog::accept()
{
    if ( !validate() )
        return;

    DefinitionInfo info = definitionInfo();

    ViewsBatch* batch = new ViewsBatch();
    batch->modifyView( m_viewId, info.toString() );

    executeBatch( batch );
}

DefaultViewDialog::DefaultViewDialog( int typeId, QWidget* parent ) : ViewDialog( parent ),
    m_typeId( typeId )
{
    TypeEntity type = TypeEntity::find( typeId );

    setWindowTitle( tr( "Default View" ) );
    setPrompt( tr( "Modify the default view for type <b>%1</b>:" ).arg( type.name() ) );
    setPromptPixmap( IconLoader::pixmap( "edit-modify", 22 ) );

    IssueTypeCache* cache = dataManager->issueTypeCache( typeId );

    initialize( false, false, typeId, cache->defaultView() );
}

DefaultViewDialog::~DefaultViewDialog()
{
}

void DefaultViewDialog::accept()
{
    if ( !validate() )
        return;

    DefinitionInfo info = definitionInfo();

    ViewsBatch* batch = new ViewsBatch();
    batch->setViewSetting( m_typeId, "default_view", info.toString() );

    executeBatch( batch );
}

RenameViewDialog::RenameViewDialog( int viewId, QWidget* parent ) : CommandDialog( parent ),
    m_viewId( viewId )
{
    ViewEntity view = ViewEntity::find( viewId );
    m_oldName = view.name();

    setWindowTitle( tr( "Rename View" ) );
    setPrompt( tr( "Enter the new name of view <b>%1</b>:" ).arg( m_oldName ) );
    setPromptPixmap( IconLoader::pixmap( "edit-rename", 22 ) );

    QHBoxLayout* layout = new QHBoxLayout();

    QLabel* label = new QLabel( tr( "&Name:" ), this );
    layout->addWidget( label, 0 );

    m_nameEdit = new InputLineEdit( this );
    m_nameEdit->setMaxLength( 40 );
    m_nameEdit->setRequired( true );
    m_nameEdit->setInputValue( m_oldName );
    layout->addWidget( m_nameEdit, 1 );

    label->setBuddy( m_nameEdit );

    setContentLayout( layout, true );

    m_nameEdit->setFocus();
}

RenameViewDialog::~RenameViewDialog()
{
}

void RenameViewDialog::accept()
{
    if ( !validate() )
        return;

    QString name = m_nameEdit->inputValue();

    if ( name == m_oldName ) {
        QDialog::accept();
        return;
    }

    ViewEntity view = ViewEntity::find( m_viewId );
    if ( ViewEntity::exists( view.typeId(), name, view.isPublic() ) ) {
        showWarning( ErrorHelper::ViewAlreadyExists );
        return;
    }

    ViewsBatch* batch = new ViewsBatch();
    batch->renameView( m_viewId, name );

    executeBatch( batch );
}

DeleteViewDialog::DeleteViewDialog( int viewId, QWidget* parent ) : CommandDialog( parent ),
    m_viewId( viewId )
{
    ViewEntity view = ViewEntity::find( viewId );

    setWindowTitle( tr( "Delete View" ) );
    if ( view.isPublic() )
        setPrompt( tr( "Are you sure you want to delete public view <b>%1</b>?" ).arg( view.name() ) );
    else
        setPrompt( tr( "Are you sure you want to delete your personal view <b>%1</b>?" ).arg( view.name() ) );
    setPromptPixmap( IconLoader::pixmap( "edit-delete", 22 ) );

    setContentLayout( NULL, true );
}

DeleteViewDialog::~DeleteViewDialog()
{
}

void DeleteViewDialog::accept()
{
    ViewsBatch* batch = new ViewsBatch();
    batch->deleteView( m_viewId );

    executeBatch( batch );
}

PublishViewDialog::PublishViewDialog( int viewId, bool isPublic, QWidget* parent ) : CommandDialog( parent ),
    m_viewId( viewId ),
    m_isPublic( isPublic )
{
    ViewEntity view = ViewEntity::find( viewId );

    if ( isPublic ) {
        setWindowTitle( tr( "Publish View" ) );
        setPrompt( tr( "Are you sure you want to convert your personal view <b>%1</b> to a public view?" ).arg( view.name() ) );
        setPromptPixmap( IconLoader::pixmap( "edit-publish", 22 ) );
    } else {
        setWindowTitle( tr( "Unpublish View" ) );
        setPrompt( tr( "Are you sure you want to convert public view <b>%1</b> to your personal view?" ).arg( view.name() ) );
        setPromptPixmap( IconLoader::pixmap( "edit-unpublish", 22 ) );
    }

    setContentLayout( NULL, true );
}

PublishViewDialog::~PublishViewDialog()
{
}

void PublishViewDialog::accept()
{
    ViewEntity view = ViewEntity::find( m_viewId );

    if ( view.isPublic() == m_isPublic ) {
        QDialog::accept();
        return;
    }

    if ( ViewEntity::exists( view.typeId(), view.name(), m_isPublic ) ) {
        showWarning( ErrorHelper::ViewAlreadyExists );
        return;
    }

    ViewsBatch* batch = new ViewsBatch();
    batch->publishView( m_viewId, m_isPublic );

    executeBatch( batch );
}

AttributeOrderDialog::AttributeOrderDialog( int typeId, QWidget* parent ) : CommandDialog( parent ),
    m_typeId( typeId ),
    m_updatingLayout( false )
{
    TypeEntity type = TypeEntity::find( typeId );

    setWindowTitle( tr( "Order of Attributes" ) );
    setPrompt( tr( "Modify order of attributes for type <b>%1</b>:" ).arg( type.name() ) );
    setPromptPixmap( IconLoader::pixmap( "edit-modify", 22 ) );

    QVBoxLayout* layout = new QVBoxLayout();

    QScrollArea* scroll = new QScrollArea( this );
    scroll->setWidgetResizable( true );
    layout->addWidget( scroll );

    QPalette scrollPalette = palette();
    scrollPalette.setColor( QPalette::Window, QColor::fromRgb( 255, 255, 255 ) );
    scroll->setPalette( scrollPalette );

    m_panel = new QWidget( scroll );
    scroll->setWidget( m_panel );

    m_widgets.append( new QLabel( tr( "Name" ), m_panel ) );
    m_widgets.append( new QLabel( tr( "Order" ), m_panel ) );
    m_widgets.append( makeSeparator( m_panel ) );

    IssueTypeCache* cache = dataManager->issueTypeCache( typeId );

    m_attributes = cache->attributes();

    QSignalMapper* indexChangedMapper = new QSignalMapper( this );
    connect( indexChangedMapper, SIGNAL( mapped( int ) ), this, SLOT( indexChanged( int ) ) );

    for ( int i = 0; i < m_attributes.count(); i++ ) {
        int attributeId = m_attributes.at( i );

        QLabel* label = new QLabel( cache->attributeName( attributeId ), m_panel );
        m_labels.insert( attributeId, label );

        QComboBox* comboBox = new QComboBox( m_panel );
        m_comboBoxes.insert( attributeId, comboBox );

        for ( int j = 1; j <= m_attributes.count(); j++ )
            comboBox->addItem( QString::number( j ) );

        indexChangedMapper->setMapping( comboBox, attributeId );
        connect( comboBox, SIGNAL( currentIndexChanged( int ) ), indexChangedMapper, SLOT( map() ) );
    }

    updateLayout();

    setContentLayout( layout, false );

    showInfo( tr( "Edit view settings." ) );

    resize( 350, 400 );
}

AttributeOrderDialog::~AttributeOrderDialog()
{
}

void AttributeOrderDialog::accept()
{
    QStringList order;
    for ( int i = 0; i < m_attributes.count(); i++ )
        order.append( QString::number( m_attributes.at( i ) ) );

    ViewsBatch* batch = new ViewsBatch();
    batch->setViewSetting( m_typeId, "attribute_order", order.join( "," ) );

    executeBatch( batch );
}

void AttributeOrderDialog::indexChanged( int attributeId )
{
    if ( m_updatingLayout )
        return;

    int index = m_comboBoxes.value( attributeId )->currentIndex();

    m_attributes.removeOne( attributeId );
    m_attributes.insert( index, attributeId );

    updateLayout();
}

void AttributeOrderDialog::updateLayout()
{
    m_updatingLayout = true;

    delete m_panel->layout();

    QGridLayout* layout = new QGridLayout( m_panel );

    layout->setColumnMinimumWidth( 0, 150 ),
    layout->setColumnMinimumWidth( 1, 70 ),
    layout->setColumnStretch( 2, 1 );

    layout->addWidget( m_widgets.at( 0 ), 0, 0 );
    layout->addWidget( m_widgets.at( 1 ), 0, 1 );
    layout->addWidget( m_widgets.at( 2 ), 1, 0, 1, 2 );

    int row = 2;
    int order = 0;

    for ( int i = 0; i < m_attributes.count(); i++ ) {
        int attributeId = m_attributes.at( i );

        QLabel* label = m_labels.value( attributeId );
        layout->addWidget( label, row, 0 );

        QComboBox* comboBox = m_comboBoxes.value( attributeId );
        comboBox->setCurrentIndex( order++ );
        layout->addWidget( comboBox, row++, 1 );
    }

    layout->setRowStretch( row, 1 );

    m_updatingLayout = false;
}

InitialViewDialog::InitialViewDialog( int typeId, QWidget* parent ) : CommandDialog( parent ),
    m_typeId( typeId ),
    m_oldViewId( 0 )
{
    TypeEntity type = TypeEntity::find( typeId );

    QList<ViewEntity> views = type.views();
    QList<ViewEntity> publicViews;

    foreach ( ViewEntity view, views ) {
        if ( view.isPublic() )
            publicViews.append( view );
    }

    IssueTypeCache* cache = dataManager->issueTypeCache( typeId );
    m_oldViewId = cache->initialViewId();

    setWindowTitle( tr( "Initial View" ) );
    setPrompt( tr( "Select initial view for type <b>%1</b>:" ).arg( type.name() ) );
    setPromptPixmap( IconLoader::pixmap( "edit-modify", 22 ) );

    QHBoxLayout* layout = new QHBoxLayout();

    QLabel* label = new QLabel( tr( "&View:" ), this );
    layout->addWidget( label, 0 );

    m_comboBox = new SeparatorComboBox( this );
    m_comboBox->addItem( tr( "All Issues" ), 0 );

    if ( !publicViews.isEmpty() ) {
        m_comboBox->addSeparator();
        foreach ( ViewEntity view, publicViews )
            m_comboBox->addItem( view.name(), view.id() );
    }

    m_comboBox->setCurrentIndex( m_comboBox->findData( m_oldViewId ) );

    layout->addWidget( m_comboBox, 1 );

    label->setBuddy( m_comboBox );

    setContentLayout( layout, true );

    m_comboBox->setFocus();
}

InitialViewDialog::~InitialViewDialog()
{
}

void InitialViewDialog::accept()
{
    int viewId = m_comboBox->itemData( m_comboBox->currentIndex() ).toInt();

    if ( viewId == m_oldViewId ) {
        QDialog::accept();
        return;
    }

    ViewsBatch* batch = new ViewsBatch();
    batch->setViewSetting( m_typeId, "initial_view", viewId != 0 ? QString::number( viewId ) : "" );

    executeBatch( batch );
}
