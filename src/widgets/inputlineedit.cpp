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

#include "inputlineedit.h"
#include "edittoolbutton.h"

#include "data/datamanager.h"
#include "utils/formatter.h"
#include "utils/validator.h"
#include "utils/errorhelper.h"
#include "utils/multiselectcompleter.h"
#include "utils/iconloader.h"

#include <QLayout>
#include <QStyle>
#include <QCompleter>
#include <QValidator>
#include <QCalendarWidget>
#include <QDesktopWidget>
#include <QApplication>
#include <QUrl>
#include <QPushButton>
#include <QLabel>

InputLineEdit::InputLineEdit( QWidget* parent ) : QLineEdit( parent ),
    m_error( false ),
    m_popup( false ),
    m_required( false ),
    m_minLength( 0 ),
    m_format( NoFormat ),
    m_functions( 0 ),
    m_empty( false ),
    m_errorIfEmpty( false )
{
    m_errorLabel = new QLabel( this );

    m_popupButton = new EditToolButton( this );
    m_popupButton->setIcon( IconLoader::icon( "arrow-down" ) );

    connect( m_popupButton, SIGNAL( clicked() ), this, SLOT( popup() ) );

    calculateLayout();
 }

InputLineEdit::~InputLineEdit()
{
}

void InputLineEdit::setInputValue( const QString& value )
{
    clearError();
    setText( valueToText( value ) );
    if ( !m_error )
        textToValue( text() );
}

QString InputLineEdit::inputValue()
{
    clearError();
    return textToValue( text() );
}

bool InputLineEdit::validate()
{
    updateInput();

    if ( m_empty && !m_errorIfEmpty )
        setError( ErrorHelper::EmptyValue );
    m_errorIfEmpty = true;

    return !m_error;
}

void InputLineEdit::setRequired( bool required )
{
    if ( m_required != required ) {
        m_required = required;
        updateInput();
    }
}

void InputLineEdit::setMinLength( int length )
{
    m_minLength = length;
}

void InputLineEdit::setFormat( int format )
{
    if ( m_format != format ) {
        m_format = format;
        updateValidator();
    }
}

void InputLineEdit::setFunctions( int functions )
{
    if ( m_functions != functions ) {
        m_functions = functions;
        updateValidator();
    }
}

void InputLineEdit::resizeEvent( QResizeEvent* e )
{
    QLineEdit::resizeEvent( e );

    calculateLayout();
}

void InputLineEdit::focusOutEvent( QFocusEvent* e )
{
    QLineEdit::focusOutEvent( e );

    updateInput();
}

void InputLineEdit::updateInput()
{
    int start = selectionStart();
    int length = selectedText().length();
    int position = cursorPosition();

    QString value = inputValue();

    if ( !m_error || m_empty ) {
        setInputValue( value );

        if ( start >= 0 ) {
            if ( position > start )
                setSelection( start, length );
            else
                setSelection( start + length, -length );
        }
    }
}

void InputLineEdit::calculateLayout()
{
    QSize buttonSize = m_popupButton->sizeHint();
    int frameWidth = style()->pixelMetric( QStyle::PM_DefaultFrameWidth );

    int x = rect().right() + 1;
    int padding = frameWidth + 1;

    if ( m_popup ) {
        padding += buttonSize.width();
        m_popupButton->move( x - padding, ( rect().bottom() + 1 - buttonSize.height() ) / 2 );
        m_popupButton->show();
    } else {
        m_popupButton->hide();
    }

    if ( m_error )
        m_errorLabel->setPixmap( IconLoader::pixmap( "status-warning" ) );
    else if ( m_empty )
        m_errorLabel->setPixmap( IconLoader::pixmap( "status-required" ) );

    if ( m_error || m_empty ) {
        padding += 16;
        m_errorLabel->move( x - padding, ( rect().bottom() - 15 ) / 2 );
        m_errorLabel->show();
    } else {
        m_errorLabel->hide();
    }

#if ( QT_VERSION >= 0x040500 )
    setTextMargins( 0, 0, padding, 0 );
#else
    setStyleSheet( QString( "QLineEdit { padding-right: %1px; }" ).arg( padding ) );
#endif

    setMinimumHeight( qMax( minimumSizeHint().height(), buttonSize.height() + 2 * frameWidth ) );
}

void InputLineEdit::setError( const QString& error )
{
    m_errorLabel->setToolTip( error );
    m_error = true;

    calculateLayout();
}

void InputLineEdit::setError( int code )
{
    ErrorHelper helper;
    setError( helper.errorMessage( (ErrorHelper::ErrorCode)code ) );
}

void InputLineEdit::clearError()
{
    m_error = false;
    m_empty = false;

    calculateLayout();
}

void InputLineEdit::setPopupVisible( bool visible )
{
    m_popup = visible;

    calculateLayout();
}

QString InputLineEdit::valueToText( const QString& value )
{
    QString text = value.simplified();

    if ( ( m_functions & MeFunction ) && text.startsWith( QLatin1String( "[Me]" ) ) )
        return QString( "[%1]" ).arg( tr( "Me" ) );

    return text;
}

QString InputLineEdit::textToValue( const QString& text )
{
    Validator validator;
    QString value = validator.normalizeString( text, 255 );

    if ( !validator.isValid() ) {
        setError( validator.errors().first() );
        return QString();
    }

    if ( value.isEmpty() ) {
        if ( m_required ) {
            if ( m_errorIfEmpty ) {
                setError( ErrorHelper::EmptyValue );
            } else {
                m_errorLabel->setToolTip( tr( "Field is required" ) );
                m_empty = true;
                calculateLayout();
            }
        }
        return QString();
    }

    if ( m_minLength > 0 && value.length() < m_minLength ) {
        setError( ErrorHelper::StringTooShort );
        return QString();
    }

    if ( m_format == EmailFormat ) {
        if ( !validator.checkEmail( value ) )
            setError( validator.errors().first() );
    }

    if ( m_format == UrlFormat ) {
        QUrl url = QUrl::fromEncoded( value.toUtf8(), QUrl::TolerantMode );
        QUrl urlPrepended = QUrl::fromEncoded( ( QLatin1String( "http://" ) + value ).toUtf8(), QUrl::TolerantMode );

        if ( url.isValid() && !url.scheme().isEmpty() && ( !url.host().isEmpty() || !url.path().isEmpty() ) && urlPrepended.port() == -1 ) {
            value = url.toString();
        } else if ( urlPrepended.isValid() && ( !urlPrepended.host().isEmpty() || !urlPrepended.path().isEmpty() ) ) {
            value = urlPrepended.toString();
        } else {
            setError( tr( "Invalid URL address" ) );
            return QString();
        }
    }

    if ( m_format == FileNameFormat ) {
        if ( value.startsWith( '.' ) || value.contains( QRegExp( "[\\\\/:*?\"<>|]" ) ) ) {
            setError( tr( "Invalid file name" ) );
            return QString();
        }
    }

    if ( m_functions & MeFunction ) {
        QString me = QString( "[%1]" ).arg( tr( "Me" ) );
        if ( value.startsWith( me, Qt::CaseInsensitive ) ) {
            if ( value.length() > me.length() ) {
                setError( ErrorHelper::InvalidFormat );
                return QString();
            }
            return "[Me]";
        }
    }

    return value;
}

void InputLineEdit::updateValidator()
{
}

void InputLineEdit::popup()
{
}

EnumLineEdit::EnumLineEdit( QWidget* parent ) : InputLineEdit( parent ),
    m_multiSelect( false ),
    m_editable( true )
{
}

EnumLineEdit::~EnumLineEdit()
{
}

void EnumLineEdit::setItems( const QStringList& items )
{
    m_items = items;

    delete completer();

    MultiSelectCompleter* completer = new MultiSelectCompleter( items, this );
    completer->setCaseSensitivity( Qt::CaseInsensitive );
    completer->setMultiSelect( m_multiSelect );

#if ( QT_VERSION >= 0x040600 )
    completer->setMaxVisibleItems( 10 );
#endif

    setCompleter( completer );

    setPopupVisible( !items.isEmpty() );
}

void EnumLineEdit::setMultiSelect( bool multi )
{
    if ( m_multiSelect != multi ) {
        m_multiSelect = multi;

        if ( MultiSelectCompleter* multiCompleter = qobject_cast<MultiSelectCompleter*>( completer() ) )
            multiCompleter->setMultiSelect( multi );

        updateInput();
    }
}

void EnumLineEdit::setEditable( bool editable )
{
    m_editable = editable;
}

QString EnumLineEdit::textToValue( const QString& text )
{
    QString value = InputLineEdit::textToValue( text );
    if ( value.isEmpty() )
        return QString();

    if ( ( functions() & MeFunction ) && value == QLatin1String( "[Me]" ) )
        return value;

    if ( m_multiSelect ) {
        QStringList parts = value.split( QLatin1Char( ',' ) );
        QStringList result;

        foreach ( const QString& part, parts ) {
            QString trimmed = part.trimmed();
            if ( trimmed.isEmpty() )
                continue;

            if ( !m_editable ) {
                if ( !m_items.contains( trimmed ) ) {
                    setError( ErrorHelper::NoMatchingItem );
                    return QString();
                }
                if ( result.contains( trimmed ) ) {
                    setError( ErrorHelper::DuplicateItems );
                    return QString();
                }
            }

            result.append( trimmed );
        }

        value = result.join( QLatin1String( ", " ) );
    } else if ( !m_editable ) {
        if ( !m_items.contains( value ) ) {
            setError( ErrorHelper::NoMatchingItem );
            return QString();
        }
    }

    return value;
}

void EnumLineEdit::popup()
{
    completer()->setCompletionPrefix( QString() );
    completer()->complete();
}

NumericLineEdit::NumericLineEdit( QWidget* parent ) : InputLineEdit( parent ),
    m_decimal( 0 ),
    m_stripZeros( false ),
    m_minValue( -1e14 ),
    m_maxValue( 1e14 )
{
    updateValidator();
}

NumericLineEdit::~NumericLineEdit()
{
}

void NumericLineEdit::setDecimal( int decimal )
{
    if ( m_decimal != decimal ) {
        m_decimal = decimal;
        updateValidator();
        updateInput();
    }
}

void NumericLineEdit::setStripZeros( bool strip )
{
    if ( m_stripZeros != strip ) {
        m_stripZeros = strip;
        updateInput();
    }
}

void NumericLineEdit::setMinValue( double value )
{
    m_minValue = value;
}

void NumericLineEdit::setMaxValue( double value )
{
    m_maxValue = value;
}

void NumericLineEdit::updateValidator()
{
    DefinitionInfo info = DefinitionInfo::fromString( dataManager->setting( "number_format" ) );

    QString pattern;

    if ( format() == Identifier )
        pattern = "#?";

    QString groupSeparator = info.metadata( "group-separator" ).toString();
    if ( !groupSeparator.isEmpty() )
        pattern += "-?(\\d*|\\d{0,3}(" + QRegExp::escape( groupSeparator ) + "\\d{0,3})*)";
    else
        pattern += "-?\\d*";

    if ( m_decimal > 0 )
        pattern += "(" + QRegExp::escape( info.metadata( "decimal-separator" ).toString() ) + QString( "\\d{0,%1})?" ).arg( m_decimal );

    delete validator();

    setValidator( new QRegExpValidator( QRegExp( pattern ), this ) );
}

QString NumericLineEdit::valueToText( const QString& value )
{
    if ( format() == Identifier ) {
        bool ok;
        int number = value.toInt( &ok );
        if ( !ok )
            return QString();
        return QString( "#%1" ).arg( number );
    } else {
        Formatter formatter;
        return formatter.convertNumber( value, m_decimal, m_stripZeros );
    }
}

QString NumericLineEdit::textToValue( const QString& text )
{
    QString value = InputLineEdit::textToValue( text );
    if ( value.isEmpty() )
        return QString();

    if ( format() == Identifier && value.at( 0 ) == QLatin1Char( '#' ) )
        value = value.mid( 1 );

    Validator validator;
    double number = validator.parseNumber( value, m_decimal );

    if ( !validator.isValid() ) {
        setError( validator.errors().first() );
        return QString();
    }

    if ( number < m_minValue ) {
        setError( ErrorHelper::NumberTooLittle );
        return QString();
    }

    if ( number > m_maxValue ) {
        setError( ErrorHelper::NumberTooGreat );
        return QString();
    }

    return QString().setNum( number, 'f', m_decimal );
}

DateTimeLineEdit::DateTimeLineEdit( QWidget* parent ) : InputLineEdit( parent ),
    m_popup( NULL ),
    m_calendar( NULL ),
    m_withTime( false ),
    m_localTime( false )
{
    setPopupVisible( true );
    updateValidator();
}

DateTimeLineEdit::~DateTimeLineEdit()
{
}

void DateTimeLineEdit::setWithTime( bool time )
{
    if ( m_withTime != time ) {
        m_withTime = time;
        updateValidator();
    }
}

void DateTimeLineEdit::setLocalTime( bool local )
{
    m_localTime = local;
}

void DateTimeLineEdit::updateValidator()
{
    DefinitionInfo dateFormat = DefinitionInfo::fromString( dataManager->setting( "date_format" ) );

    QString dateSeparator = dateFormat.metadata( "date-separator" ).toString();
    QString order = dateFormat.metadata( "date-order" ).toString();

    QMap<QChar, QString> parts;
    parts.insert( QChar( 'd' ), "\\d{0,2}" );
    parts.insert( QChar( 'm' ), "\\d{0,2}" );
    parts.insert( QChar( 'y' ), "\\d{0,4}" );

    QString pattern = parts.value( order.at( 0 ) );
    pattern += QRegExp::escape( dateSeparator );
    pattern += parts.value( order.at( 1 ) );
    pattern += QRegExp::escape( dateSeparator );
    pattern += parts.value( order.at( 2 ) );

    if ( m_withTime ) {
        DefinitionInfo timeFormat = DefinitionInfo::fromString( dataManager->setting( "time_format" ) );

        pattern += QLatin1String( " \\d{0,2}" );
        pattern += QRegExp::escape( timeFormat.metadata( "time-separator" ).toString() );
        pattern += QLatin1String( "\\d{0,2}" );

        int mode = timeFormat.metadata( "time-mode" ).toInt();
        if ( mode == 12 )
            pattern += QLatin1String( " ?[ap]?m?" );
    }

    if ( functions() & TodayFunction )
        pattern = QString( "(\\[%1\\][+-]?\\d*)|%2" ).arg( tr( "Today" ), pattern );

    delete validator();

    setValidator( new QRegExpValidator( QRegExp( pattern, Qt::CaseInsensitive ), this ) );
}

QString DateTimeLineEdit::valueToText( const QString& value )
{
    if ( ( functions() & TodayFunction ) && value.startsWith( QLatin1String( "[Today]" ) ) )
        return "[" + tr( "Today" ) + "]" + value.mid( 7 );

    Formatter formatter;
    if ( m_withTime )
        return formatter.convertDateTime( value, m_localTime );
    else
        return formatter.convertDate( value );
}

QString DateTimeLineEdit::textToValue( const QString& text )
{
    QString value = InputLineEdit::textToValue( text );
    if ( value.isEmpty() )
        return QString();

    QString today = QString( "[%1]" ).arg( tr( "Today" ) );
    if ( ( functions() & TodayFunction ) && value.startsWith( today, Qt::CaseInsensitive ) ) {
        if ( value.length() == today.length() )
            return "[Today]";
        QChar sign = value.at( today.length() );
        bool ok;
        int offset = value.mid( today.length() + 1 ).toInt( &ok );
        if ( ( sign != '-' && sign != '+' ) || !ok || offset < 1 ) {
            setError( ErrorHelper::InvalidFormat );
            return QString();
        }
        return QString( "[Today]%1%2" ).arg( sign ).arg( offset );
    }

    QString result;

    Validator validator;
    if ( m_withTime )
        result = validator.convertDateTime( value, m_localTime );
    else
        result = validator.convertDate( value );

    if ( !validator.isValid() ) {
        setError( validator.errors().first() );
        return QString();
    }

    return result;
}

void DateTimeLineEdit::popup()
{
    if ( !m_popup ) {
        m_popup = new QWidget( this, Qt::Popup );
        m_popup->setAttribute( Qt::WA_WindowPropagation );

        m_calendar = new QCalendarWidget( m_popup );
        m_calendar->setVerticalHeaderFormat( QCalendarWidget::NoVerticalHeader );
        m_calendar->setMinimumDate( QDate( 1, 1, 1 ) );
        m_calendar->setMaximumDate( QDate( 9999, 12, 31 ) );

        int firstDayOfWeek = dataManager->setting( "first_day_of_week" ).toInt();
        m_calendar->setFirstDayOfWeek( (Qt::DayOfWeek)( firstDayOfWeek != 0 ? firstDayOfWeek : 7 ) );

        connect( m_calendar, SIGNAL( activated( QDate ) ), this, SLOT( setDate( QDate ) ) );
        connect( m_calendar, SIGNAL( clicked( QDate ) ), this, SLOT( setDate( QDate ) ) );

        QVBoxLayout* layout = new QVBoxLayout( m_popup );
        layout->setMargin( 0 );
        layout->setSpacing( 0 );
        layout->addWidget( m_calendar );

        QPushButton* todayButton = new QPushButton( tr( "Today" ), m_popup );

        connect( todayButton, SIGNAL( clicked() ), this, SLOT( setToday() ) );

        layout->addWidget( todayButton, 0, Qt::AlignHCenter );
    }

    QString value = text();
    int index = value.indexOf( ' ' );
    if ( index >= 0 )
        value.truncate( index );

    Validator validator;
    QDate date = validator.parseDate( value );
    if ( date.isValid() )
        m_calendar->setSelectedDate( date );

    QPoint pos = rect().bottomLeft();
    QPoint pos2 = rect().topLeft();
    pos = mapToGlobal( pos );
    pos2 = mapToGlobal( pos2 );
    QSize size = m_popup->sizeHint();
    QRect screen = QApplication::desktop()->availableGeometry( pos );
    if ( pos.x() + size.width() > screen.right() )
        pos.setX( screen.right() - size.width() );
    pos.setX( qMax( pos.x(), screen.left() ) );
    if ( pos.y() + size.height() > screen.bottom() )
        pos.setY( pos2.y() - size.height() );
    else if ( pos.y() < screen.top() )
        pos.setY( screen.top() );
    if ( pos.y() < screen.top() )
        pos.setY( screen.top() );
    if ( pos.y() + size.height() > screen.bottom() )
        pos.setY( screen.bottom() - size.height() );
    m_popup->move( pos );

    m_popup->show();
}

void DateTimeLineEdit::setDate( const QDate& date )
{
    QString suffix;
    int index = text().indexOf( ' ' );
    if ( index >= 0 )
        suffix = text().mid( index );

    if ( suffix.isEmpty() && m_withTime ) {
        DefinitionInfo format = DefinitionInfo::fromString( dataManager->setting( "time_format" ) );
        if ( format.metadata( "time-mode" ).toInt() == 12 )
            suffix = " 12:00 am";
        else if ( format.metadata( "pad-hour" ).toBool() )
            suffix = " 00:00";
        else
            suffix = " 0:00";
    }

    Formatter formatter;
    setText( formatter.formatDate( date ) + suffix );

    m_popup->hide();
}

void DateTimeLineEdit::setToday()
{
    if ( ( functions() & TodayFunction ) ) {
        setInputValue( "[Today]" );
    } else {
        Formatter formatter;
        if ( m_withTime )
            setText( formatter.formatDateTime( QDateTime::currentDateTime(), false ) );
        else
            setText( formatter.formatDate( QDate::currentDate() ) );
    }

    m_popup->hide();
}
