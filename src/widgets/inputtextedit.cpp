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

#include "inputtextedit.h"
#include "edittoolbutton.h"

#include "utils/formatter.h"
#include "utils/validator.h"
#include "utils/errorhelper.h"
#include "utils/iconloader.h"

#include <QStyle>
#include <QScrollBar>

InputTextEdit::InputTextEdit( QWidget* parent ) : QPlainTextEdit( parent ),
    m_error( false ),
    m_required( false ),
    m_minLength( 0 ),
    m_maxLength( 0 ),
    m_format( NoFormat ),
    m_empty( false ),
    m_errorIfEmpty( false ),
    m_autoValidate( true )
{
    m_errorButton = new EditToolButton( this );

    calculateLayout();
 }

InputTextEdit::~InputTextEdit()
{
}

void InputTextEdit::setInputValue( const QString& value )
{
    clearError();
    setPlainText( valueToText( value ) );
    if ( !m_error )
        textToValue( toPlainText() );
}

QString InputTextEdit::inputValue()
{
    clearError();
    return textToValue( toPlainText() );
}

bool InputTextEdit::validate()
{
    updateInput();

    if ( m_empty && !m_errorIfEmpty )
        setError( ErrorHelper::EmptyValue );
    m_errorIfEmpty = true;

    return !m_error;
}

void InputTextEdit::setRequired( bool required )
{
    if ( m_required != required ) {
        m_required = required;
        updateInput();
    }
}

void InputTextEdit::setMinLength( int length )
{
    m_minLength = length;
}

void InputTextEdit::setMaxLength( int length )
{
    m_maxLength = length;
}

void InputTextEdit::setFormat( Format format )
{
    m_format = format;
}

void InputTextEdit::setAutoValidate( bool validate )
{
    m_autoValidate = validate;
}

void InputTextEdit::resizeEvent( QResizeEvent* e )
{
    QPlainTextEdit::resizeEvent( e );

    calculateLayout();
}

void InputTextEdit::focusOutEvent( QFocusEvent* e )
{
    QPlainTextEdit::focusOutEvent( e );

    if ( m_autoValidate )
        updateInput();
}

void InputTextEdit::updateInput()
{
    QTextCursor oldCursor = textCursor();
    int anchor = oldCursor.anchor();
    int position = oldCursor.position();

    QString value = inputValue();

    if ( !m_error || m_empty ) {
        setInputValue( value );

        QTextCursor newCursor = textCursor();
        newCursor.setPosition( anchor );
        newCursor.setPosition( position, QTextCursor::KeepAnchor );
        setTextCursor( newCursor );
    }
}

void InputTextEdit::calculateLayout()
{
    QSize buttonSize = m_errorButton->sizeHint();
    int frameWidth = style()->pixelMetric( QStyle::PM_DefaultFrameWidth );

    if ( m_error )
        m_errorButton->setIcon( IconLoader::icon( "status-warning" ) );
    else if ( m_empty )
        m_errorButton->setIcon( IconLoader::icon( "status-required" ) );

    if ( m_error || m_empty ) {
        int scrollWidth = verticalScrollBar()->isVisible() ? verticalScrollBar()->width() : 0;
        m_errorButton->move( rect().right() - buttonSize.width() - frameWidth - scrollWidth, frameWidth );
        m_errorButton->show();
    } else {
        m_errorButton->hide();
    }
}

void InputTextEdit::setError( const QString& error )
{
    m_errorButton->setToolTip( error );
    m_error = true;

    calculateLayout();
}

void InputTextEdit::clearError()
{
    m_error = false;
    m_empty = false;

    calculateLayout();
}

QString InputTextEdit::valueToText( const QString& value )
{
    return value;
}

QString InputTextEdit::textToValue( const QString& text )
{
    Validator validator;

    QString value = validator.normalizeMultiLineString( text );

    if ( !validator.isValid() ) {
        setError( validator.errors().first() );
        return QString();
    }

    if ( value.isEmpty() ) {
        if ( m_required ) {
            if ( m_errorIfEmpty ) {
                setError( ErrorHelper::EmptyValue );
            } else {
                m_errorButton->setToolTip( tr( "Field is required" ) );
                m_empty = true;
                calculateLayout();
            }
        }
        return QString();
    }

    if ( m_maxLength > 0 && value.length() > m_maxLength ) {
        setError( ErrorHelper::StringTooLong );
        return QString();
    }

    if ( m_minLength > 0 && value.length() < m_minLength ) {
        setError( ErrorHelper::StringTooShort );
        return QString();
    }

    if ( m_format == ItemsFormat ) {
        value = validator.normalizeStringList( value.split( '\n' ), 255 ).join( "\n" );
        if ( !validator.isValid() ) {
            setError( validator.errors().first() );
            return QString();
        }
    }

    return value;
}

void InputTextEdit::setError( int code )
{
    ErrorHelper helper;
    setError( helper.errorMessage( (ErrorHelper::ErrorCode)code ) );
}

