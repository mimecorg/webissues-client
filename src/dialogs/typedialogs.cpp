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

#include "typedialogs.h"
#include "metadatadialog.h"

#include "commands/typesbatch.h"
#include "commands/commandmanager.h"
#include "data/datamanager.h"
#include "rdb/utilities.h"
#include "utils/definitioninfo.h"
#include "utils/validator.h"
#include "utils/errorhelper.h"
#include "utils/iconloader.h"
#include "widgets/inputlineedit.h"
#include "widgets/abstractvalueeditor.h"
#include "widgets/valueeditorfactory.h"

#include <QLayout>
#include <QLabel>
#include <QGroupBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QComboBox>
#include <QPushButton>
#include <QToolButton>

AddTypeDialog::AddTypeDialog( QWidget* parent ) : CommandDialog( parent )
{
    setWindowTitle( tr( "Add Type" ) );
    setPrompt( tr( "Create a new type:" ) );
    setPromptPixmap( IconLoader::pixmap( "type-new", 22 ) );

    QHBoxLayout* layout = new QHBoxLayout();

    QLabel* label = new QLabel( tr( "&Name:" ), this );
    layout->addWidget( label, 0 );

    m_nameEdit = new InputLineEdit( this );
    m_nameEdit->setMaxLength( 40 );
    m_nameEdit->setRequired( true );
    layout->addWidget( m_nameEdit, 1 );

    label->setBuddy( m_nameEdit );

    setContentLayout( layout, true );

    m_nameEdit->setFocus();
}

AddTypeDialog::~AddTypeDialog()
{
}

void AddTypeDialog::accept()
{
    if ( !validate() )
        return;

    QString name = m_nameEdit->inputValue();

    RDB::IndexConstIterator<TypeRow> it( dataManager->types()->index() );
    if ( RDB::findRow( it, &TypeRow::name, name ) ) {
        showWarning( ErrorHelper::statusMessage( ErrorHelper::TypeAlreadyExists ) );
        return;
    }

    TypesBatch* batch = new TypesBatch();
    batch->addType( name );

    executeBatch( batch );
}

RenameTypeDialog::RenameTypeDialog( int typeId, QWidget* parent ) : CommandDialog( parent ),
    m_typeId( typeId )
{
    const TypeRow* type = dataManager->types()->find( typeId );
    m_oldName = type ? type->name() : QString();

    setWindowTitle( tr( "Rename Type" ) );
    setPrompt( tr( "Enter the new name of type <b>%1</b>:" ).arg( m_oldName ) );
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

RenameTypeDialog::~RenameTypeDialog()
{
}

void RenameTypeDialog::accept()
{
    if ( !validate() )
        return;

    QString name = m_nameEdit->inputValue();

    if ( name == m_oldName ) {
        QDialog::accept();
        return;
    }

    RDB::IndexConstIterator<TypeRow> it( dataManager->types()->index() );
    if ( RDB::findRow( it, &TypeRow::name, name ) ) {
        showWarning( ErrorHelper::statusMessage( ErrorHelper::TypeAlreadyExists ) );
        return;
    }

    TypesBatch* batch = new TypesBatch();
    batch->renameType( m_typeId, name );

    executeBatch( batch );
}

DeleteTypeDialog::DeleteTypeDialog( int typeId, QWidget* parent ) : CommandDialog( parent ),
    m_typeId( typeId ),
    m_force( false )
{
    const TypeRow* type = dataManager->types()->find( typeId );
    QString name = type ? type->name() : QString();

    setWindowTitle( tr( "Delete Type" ) );
    setPrompt( tr( "Are you sure you want to delete type <b>%1</b> and all its attributes?" ).arg( name ) );
    setPromptPixmap( IconLoader::pixmap( "edit-delete", 22 ) );

    RDB::IndexConstIterator<FolderRow> it( dataManager->folders()->index() );
    while ( it.next() ) {
        if ( it.get()->typeId() == typeId ) {
            showWarning( tr( "All folders and issues of this type will be permanently deleted." ) );
            m_force = true;
            break;
        }
    }

    setContentLayout( NULL, true );
}

DeleteTypeDialog::~DeleteTypeDialog()
{
}

void DeleteTypeDialog::accept()
{
    TypesBatch* batch = new TypesBatch();
    batch->deleteType( m_typeId, m_force );

    executeBatch( batch );
}

bool DeleteTypeDialog::batchFailed( AbstractBatch* /*batch*/ )
{
    if ( commandManager->error() == CommandManager::WebIssuesError && commandManager->errorCode() == ErrorHelper::CannotDeleteType ) {
        showWarning( tr( "All folders and issues of this type will be permanently deleted." ) );
        m_force = true;
    }

    return true;
}

AttributeDialog::AttributeDialog( QWidget* parent ) : CommandDialog( parent ),
    m_layout( NULL ),
    m_nameEdit( NULL ),
    m_typeCombo( NULL ),
    m_detailsEdit( NULL ),
    m_requiredCheck( NULL ),
    m_valueLabel( NULL ),
    m_editor( NULL ),
    m_type( InvalidAttribute )
{
}

AttributeDialog::~AttributeDialog()
{
}

void AttributeDialog::initialize( bool withName, const DefinitionInfo& info )
{
    int offset = withName ? 1 : 0;

    m_layout = new QGridLayout();
    m_layout->setColumnStretch( 1, 1 );

    if ( withName ) {
        QLabel* nameLabel = new QLabel( tr( "&Name:" ), this );
        m_layout->addWidget( nameLabel, 0, 0 );

        m_nameEdit = new InputLineEdit( this );
        m_nameEdit->setMaxLength( 40 );
        m_nameEdit->setRequired( true );
        m_layout->addWidget( m_nameEdit, 0, 1, 1, 2 );

        nameLabel->setBuddy( m_nameEdit );
    }

    QLabel* typeLabel = new QLabel( tr( "&Type:" ), this );
    m_layout->addWidget( typeLabel, 0 + offset, 0 );

    m_typeCombo = new QComboBox( this );
    m_layout->addWidget( m_typeCombo, 0 + offset, 1, 1, 2 );

    typeLabel->setBuddy( m_typeCombo );

    QLabel* detailsLabel = new QLabel( tr( "Details:" ), this );
    m_layout->addWidget( detailsLabel, 1 + offset, 0 );

    m_detailsEdit = new QLineEdit( this );
    m_detailsEdit->setReadOnly( true );
    m_layout->addWidget( m_detailsEdit, 1 + offset, 1 );

    QToolButton* detailsButton = new QToolButton( this );
    detailsButton->setAutoRaise( true );
    detailsButton->setIcon( IconLoader::icon( "edit-modify" ) );
    detailsButton->setIconSize( QSize( 16, 16 ) );
    detailsButton->setToolTip( tr( "Edit" ) );
    m_layout->addWidget( detailsButton, 1 + offset, 2 );

    m_requiredCheck = new QCheckBox( tr( "&Attribute is required" ), this );
    m_layout->addWidget( m_requiredCheck, 2 + offset, 0, 1, 3 );

    m_valueLabel = new QLabel( tr( "Default &value:" ), this );
    m_layout->addWidget( m_valueLabel, 3 + offset, 0 );

    connect( m_typeCombo, SIGNAL( activated( int ) ), this, SLOT( typeActivated( int ) ) );
    connect( detailsButton, SIGNAL( clicked() ), this, SLOT( detailsClicked() ) );
    connect( m_requiredCheck, SIGNAL( toggled( bool ) ), this, SLOT( requiredToggled( bool ) ) );

    if ( info.isEmpty() ) {
        m_type = FirstAttribute;

        for ( int i = FirstAttribute; i <= LastAttribute; i++ )
            m_typeCombo->addItem( AttributeHelper::typeName( (AttributeType)i ), i );
    } else {
        m_type = AttributeHelper::toAttributeType( info );
        m_metadata = info.metadata();

        QList<AttributeType> types = AttributeHelper::compatibleTypes( m_type );
        for ( int i = 0; i < types.count(); i++ )
            m_typeCombo->addItem( AttributeHelper::typeName( types.at( i ) ), (int)types.at( i ) );

        m_typeCombo->blockSignals( true );
        m_typeCombo->setCurrentIndex( m_typeCombo->findData( (int)m_type ) );
        m_typeCombo->blockSignals( false );
    }

    setContentLayout( m_layout, true );

    updateWidgets();

    if ( withName )
        m_nameEdit->setFocus();
    else
        m_typeCombo->setFocus();
}

DefinitionInfo AttributeDialog::definitionInfo()
{
    Validator validator;
    DefinitionInfo info = validator.createAttributeDefinition( m_type, m_metadata );

    if ( !validator.isValid() ) {
        showWarning( validator.errors().first() );
        return DefinitionInfo();
    }

    if ( m_requiredCheck->isChecked() )
        info.setMetadata( "required", 1 );

    QString value = m_editor->inputValue();
    if ( !value.isEmpty() )
        info.setMetadata( "default", value );

    return info;
}

QString AttributeDialog::attributeName() const
{
    return m_nameEdit->inputValue();
}

void AttributeDialog::typeActivated( int index )
{
    m_type = (AttributeType)m_typeCombo->itemData( index ).toInt();

    updateWidgets();
}

void AttributeDialog::detailsClicked()
{
    MetadataDialog dialog( m_type, this );
    dialog.setMetadata( m_metadata );

    if ( dialog.exec() == QDialog::Accepted ) {
        m_metadata = dialog.metadata();

        m_metadata.insert( "required", m_requiredCheck->isChecked() );
        m_metadata.insert( "default", m_editor ? m_editor->inputValue() : QString() );

        updateWidgets();
    }
}

void AttributeDialog::requiredToggled( bool on )
{
    m_metadata.insert( "required", on );
    m_metadata.insert( "default", m_editor ? m_editor->inputValue() : QString() );

    updateWidgets();
}

void AttributeDialog::updateWidgets()
{
    Validator validator;
    DefinitionInfo info = validator.createAttributeDefinition( m_type, m_metadata );

    m_detailsEdit->setText( AttributeHelper::metadataDetails( info ) );
    m_detailsEdit->setCursorPosition( 0 );

    delete m_editor;
    m_editor = NULL;

    int row, col, rspan, cspan;
    m_layout->getItemPosition( m_layout->indexOf( m_valueLabel ), &row, &col, &rspan, &cspan );

    if ( info.isEmpty() )
        return;

    bool required = m_metadata.value( "required" ).toBool();
    if ( required )
        info.setMetadata( "required", 1 );

    m_requiredCheck->blockSignals( true );
    m_requiredCheck->setChecked( required );
    m_requiredCheck->blockSignals( false );

    m_editor = ValueEditorFactory::createInitialValueEditor( info, this, this );
    m_layout->addWidget( m_editor->widget(), row, 1, 1, 2 );

    QString value = m_metadata.value( "default" ).toString();
    m_editor->setInputValue( value );

    m_valueLabel->setBuddy( m_editor->widget() );

    if ( isVisible() )
        m_editor->widget()->show();
}

bool AttributeDialog::event( QEvent* e )
{
    if ( e->type() == QEvent::LayoutRequest ) {
        setMaximumHeight( QWIDGETSIZE_MAX );
        resize( width(), minimumSizeHint().height() );
        setMaximumHeight( sizeHint().height() );
    }

    return CommandDialog::event( e );
}

AddAttributeDialog::AddAttributeDialog( int typeId, QWidget* parent ) : AttributeDialog( parent ),
    m_typeId( typeId )
{
    const TypeRow* type = dataManager->types()->find( typeId );
    QString name = type ? type->name() : QString();

    setWindowTitle( tr( "Add Attribute" ) );
    setPrompt( tr( "Create a new attribute in type <b>%1</b>:" ).arg( name ) );
    setPromptPixmap( IconLoader::pixmap( "attribute-new", 22 ) );

    initialize( true, DefinitionInfo() );
}

AddAttributeDialog::~AddAttributeDialog()
{
}

void AddAttributeDialog::accept()
{
    if ( !validate() )
        return;

    QString name = attributeName();

    RDB::ForeignConstIterator<AttributeRow> it( dataManager->attributes()->parentIndex(), m_typeId );
    if ( RDB::findRow( it, &AttributeRow::name, name ) ) {
        showWarning( ErrorHelper::statusMessage( ErrorHelper::AttributeAlreadyExists ) );
        return;
    }

    DefinitionInfo info = definitionInfo();
    if ( info.isEmpty() )
        return;

    TypesBatch* batch = new TypesBatch();
    batch->addAttribute( m_typeId, name, info.toString() );

    executeBatch( batch );
}

ModifyAttributeDialog::ModifyAttributeDialog( int attributeId, QWidget* parent ) : AttributeDialog( parent ),
    m_attributeId( attributeId )
{
    const AttributeRow* attribute = dataManager->attributes()->find( attributeId );
    QString name = attribute ? attribute->name() : QString();
    m_oldDefinition = attribute ? attribute->definition() : QString();

    setWindowTitle( tr( "Modify Attribute" ) );
    setPrompt( tr( "Modify attribute <b>%1</b>:" ).arg( name ) );
    setPromptPixmap( IconLoader::pixmap( "edit-modify", 22 ) );

    DefinitionInfo info = DefinitionInfo::fromString( m_oldDefinition );

    initialize( false, info );
}

ModifyAttributeDialog::~ModifyAttributeDialog()
{
}

void ModifyAttributeDialog::accept()
{
    if ( !validate() )
        return;

    DefinitionInfo info = definitionInfo();
    if ( info.isEmpty() )
        return;

    TypesBatch* batch = new TypesBatch();
    batch->modifyAttribute( m_attributeId, info.toString() );

    executeBatch( batch );
}

RenameAttributeDialog::RenameAttributeDialog( int attributeId, QWidget* parent ) : CommandDialog( parent ),
    m_attributeId( attributeId )
{
    const AttributeRow* attribute = dataManager->attributes()->find( attributeId );
    m_oldName = attribute ? attribute->name() : QString();

    setWindowTitle( tr( "Rename Attribute" ) );
    setPrompt( tr( "Enter the new name of attribute <b>%1</b>:" ).arg( m_oldName ) );
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

RenameAttributeDialog::~RenameAttributeDialog()
{
}

void RenameAttributeDialog::accept()
{
    if ( !validate() )
        return;

    QString name = m_nameEdit->inputValue();

    if ( name == m_oldName ) {
        QDialog::accept();
        return;
    }

    const AttributeRow* attribute = dataManager->attributes()->find( m_attributeId );
    int typeId = attribute ? attribute->typeId() : 0;

    RDB::ForeignConstIterator<AttributeRow> it( dataManager->attributes()->parentIndex(), typeId );
    if ( RDB::findRow( it, &AttributeRow::name, name ) ) {
        showWarning( ErrorHelper::statusMessage( ErrorHelper::AttributeAlreadyExists ) );
        return;
    }

    TypesBatch* batch = new TypesBatch();
    batch->renameAttribute( m_attributeId, name );

    executeBatch( batch );
}

DeleteAttributeDialog::DeleteAttributeDialog( int attributeId, QWidget* parent ) : CommandDialog( parent ),
    m_attributeId( attributeId ),
    m_force( false )
{
    const AttributeRow* attribute = dataManager->attributes()->find( attributeId );
    QString name = attribute ? attribute->name() : QString();

    setWindowTitle( tr( "Delete Attribute" ) );
    setPrompt( tr( "Are you sure you want to delete attribute <b>%1</b>?" ).arg( name ) );
    setPromptPixmap( IconLoader::pixmap( "edit-delete", 22 ) );

    RDB::ForeignConstIterator<ValueRow> it( dataManager->values()->index().first(), attributeId );
    if ( it.next() ) {
        showWarning( tr( "All current values of this attribute will be deleted." ) );
        m_force = true;
    }

    setContentLayout( NULL, true );
}

DeleteAttributeDialog::~DeleteAttributeDialog()
{
}

void DeleteAttributeDialog::accept()
{
    TypesBatch* batch = new TypesBatch();
    batch->deleteAttribute( m_attributeId, m_force );

    executeBatch( batch );
}

bool DeleteAttributeDialog::batchFailed( AbstractBatch* /*batch*/ )
{
    if ( commandManager->error() == CommandManager::WebIssuesError && commandManager->errorCode() == ErrorHelper::CannotDeleteAttribute ) {
        showWarning( tr( "All folders and issues of this type will be permanently deleted." ) );
        m_force = true;
    }

    return true;
}
