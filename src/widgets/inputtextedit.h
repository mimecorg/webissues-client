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

#ifndef INPUTTEXTEDIT_H
#define INPUTTEXTEDIT_H

#include <QPlainTextEdit>

class EditToolButton;

class QLabel;

class InputTextEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    InputTextEdit( QWidget* parent );
    ~InputTextEdit();

public:
    enum Format
    {
        NoFormat,
        ItemsFormat
    };

public:
    void setInputValue( const QString& value );
    QString inputValue();

    void setError( const QString& error );
    void clearError();

    bool validate();

    void setRequired( bool required );
    bool isRequired() const { return m_required; }

    void setMinLength( int length );
    int minLength() const { return m_minLength; }

    void setMaxLength( int length );
    int maxLength() const { return m_maxLength; }

    void setFormat( Format format );
    Format format() const { return m_format; }

    void setAutoValidate( bool validate );
    bool autoValidate() const { return m_autoValidate; }

protected: // overrides
    void resizeEvent( QResizeEvent* e );
    void focusOutEvent( QFocusEvent* e );
    void changeEvent( QEvent* e );

private:
    void updateInput();

    QString valueToText( const QString& value );
    QString textToValue( const QString& text );

    void calculateLayout();

    void setError( int code );

    void updateTabStopWidth();

private:
    QLabel* m_errorLabel;

    bool m_error;
    bool m_required;

    int m_minLength;
    int m_maxLength;

    Format m_format;

    bool m_empty;
    bool m_errorIfEmpty;

    bool m_autoValidate;
};

#endif
