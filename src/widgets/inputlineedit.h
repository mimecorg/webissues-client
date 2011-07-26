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

#ifndef INPUTLINEEDIT_H
#define INPUTLINEEDIT_H

#include <QLineEdit>

class EditToolButton;

class QCalendarWidget;
class QDate;

class InputLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    InputLineEdit( QWidget* parent );
    ~InputLineEdit();

public:
    enum Format
    {
        NoFormat = 0,
        EmailFormat = 1,
        UrlFormat = 2,
        FileNameFormat = 3
    };

    enum Functions
    {
        MeFunction = 1
    };

public:
    void setInputValue( const QString& value );
    QString inputValue();

    bool validate();

    void setRequired( bool required );
    bool isRequired() const { return m_required; }

    void setMinLength( int length );
    int minLength() const { return m_minLength; }

    void setFormat( int format );
    int format() const { return m_format; }

    void setFunctions( int functions );
    int functions() const { return m_functions; }

protected: // overrides
    void resizeEvent( QResizeEvent* e );
    void focusOutEvent( QFocusEvent* e );

protected:
    void updateInput();

    void setError( const QString& error );
    void setError( int code );
    void clearError();

    void setPopupVisible( bool visible );

    virtual QString valueToText( const QString& value );
    virtual QString textToValue( const QString& text );

    virtual void updateValidator();

protected slots:
    virtual void popup();

private:
    void calculateLayout();

private:
    EditToolButton* m_errorButton;
    EditToolButton* m_popupButton;

    bool m_error;
    bool m_popup;

    bool m_required;
    int m_minLength;
    int m_format;
    int m_functions;

    bool m_empty;
    bool m_errorIfEmpty;
};

class EnumLineEdit : public InputLineEdit
{
    Q_OBJECT
public:
    EnumLineEdit( QWidget* parent );
    ~EnumLineEdit();

public:
    void setItems( const QStringList& items );
    const QStringList& items() const { return m_items; }

    void setMultiSelect( bool multi );
    bool isMultiSelect() const { return m_multiSelect; }

    void setEditable( bool editable );
    bool editable() const { return m_editable; }

protected: // overrides
    QString textToValue( const QString& text );
    void popup();

private:
    QStringList m_items;

    bool m_multiSelect;
    bool m_editable;
};

class NumericLineEdit : public InputLineEdit
{
    Q_OBJECT
public:
    NumericLineEdit( QWidget* parent );
    ~NumericLineEdit();

public:
    enum NumericFormat
    {
        Identifier = 4
    };

public:
    void setDecimal( int decimal );
    int decimal() const { return m_decimal; }

    void setStripZeros( bool strip );
    bool stripZeros() const { return m_stripZeros; }

    void setMinValue( double value );
    double minValue() const { return m_minValue; }

    void setMaxValue( double value );
    double maxValue() const { return m_maxValue; }

protected: // overrides
    QString valueToText( const QString& value );
    QString textToValue( const QString& text );
    void updateValidator();

private:
    int m_decimal;
    bool m_stripZeros;

    double m_minValue;
    double m_maxValue;
};

class DateTimeLineEdit : public InputLineEdit
{
    Q_OBJECT
public:
    DateTimeLineEdit( QWidget* parent );
    ~DateTimeLineEdit();

public:
    enum DateTimeFunctions
    {
        TodayFunction = 2
    };

public:
    void setWithTime( bool time );
    bool withTime() const { return m_withTime; }

    void setLocalTime( bool local );
    bool localTime() const { return m_localTime; }

protected: // overrides
    QString valueToText( const QString& value );
    QString textToValue( const QString& text );
    void updateValidator();
    void popup();

private slots:
    void setDate( const QDate& date );
    void setToday();

private:
    QWidget* m_popup;
    QCalendarWidget* m_calendar;

    bool m_withTime;
    bool m_localTime;
};

#endif
