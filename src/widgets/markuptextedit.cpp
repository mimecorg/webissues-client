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

#include "markuptextedit.h"

#include "application.h"
#include "data/datamanager.h"
#include "dialogs/finditemdialog.h"
#include "utils/htmlwriter.h"
#include "utils/markupprocessor.h"
#include "utils/iconloader.h"
#include "views/viewmanager.h"
#include "widgets/inputtextedit.h"

#include <QLayout>
#include <QComboBox>
#include <QToolButton>

static void createButton( const QIcon& icon, const QString& text, QWidget* parent, QObject* receiver, const char* method, const QKeySequence& shortcut = QKeySequence() )
{
    QToolButton* button = new QToolButton( parent );
    
    button->setAutoRaise( true );

    button->setIcon( icon );
    button->setIconSize( QSize( 16, 16 ) );
    
    if ( !shortcut.isEmpty() ) {
        button->setToolTip( QString( "%1 (%2)" ).arg( text, shortcut.toString( QKeySequence::NativeText ) ) );
        button->setShortcut( shortcut );
    } else {
        button->setToolTip( text );
    }

    parent->layout()->addWidget( button );

    QObject::connect( button, SIGNAL( clicked() ), receiver, method );
}

static void createSeparator( QWidget* parent )
{
    QFrame* separator = new QFrame( parent );
    separator->setFrameStyle( QFrame::VLine | QFrame::Sunken );
    separator->setFixedWidth( 6 );

    parent->layout()->addWidget( separator );
}

MarkupTextEdit::MarkupTextEdit( QWidget* parent ) : QWidget( parent ),
    m_preview( NULL ),
    m_previewWindow( NULL )
{
    QVBoxLayout* layout = new QVBoxLayout( this );
    layout->setMargin( 0 );

    QVBoxLayout* editLayout = new QVBoxLayout();
    editLayout->setMargin( 0 );
    editLayout->setSpacing( 1 );
    layout->addLayout( editLayout );

    m_toolBar = new QWidget( this );
    editLayout->addWidget( m_toolBar );

    QHBoxLayout* toolLayout = new QHBoxLayout( m_toolBar );
    toolLayout->setMargin( 0 );
    toolLayout->setSpacing( 0 );

    createButton( IconLoader::icon( "markup-bold" ), tr( "Bold" ), m_toolBar, this, SLOT( markupBold() ), Qt::CTRL + Qt::Key_B );
    createButton( IconLoader::icon( "markup-italic" ), tr( "Italic" ), m_toolBar, this, SLOT( markupItalic() ), Qt::CTRL + Qt::Key_I );
    createButton( IconLoader::icon( "markup-monospace" ), tr( "Monospace" ), m_toolBar, this, SLOT( markupMonospace() ) );
    createSeparator( m_toolBar );
    createButton( IconLoader::icon( "markup-link" ), tr( "Hyperlink" ), m_toolBar, this, SLOT( markupLink() ), Qt::CTRL + Qt::Key_K );
    createSeparator( m_toolBar );
    createButton( IconLoader::icon( "markup-list" ), tr( "Bullet List" ), m_toolBar, this, SLOT( markupList() ) );
    createButton( IconLoader::icon( "comment" ), tr( "Quote Block" ), m_toolBar, this, SLOT( markupQuote() ) );
    createButton( IconLoader::icon( "markup-code" ), tr( "Code Block" ), m_toolBar, this, SLOT( markupCode() ) );
    createSeparator( m_toolBar );
    createButton( IconLoader::icon( "print-preview" ), tr( "Preview" ), m_toolBar, this, SLOT( showPreview() ) );

    toolLayout->addStretch( 1 );

    m_toolBar->setVisible( false );

    m_edit = new InputTextEdit( this );
    editLayout->addWidget( m_edit, 1 );

    m_edit->setRequired( true );
    m_edit->setMaxLength( dataManager->setting( "comment_max_length" ).toInt() );

    QHBoxLayout* formatLayout = new QHBoxLayout();
    layout->addLayout( formatLayout );

    QLabel* label = new QLabel( tr( "&Text format:" ), this );
    formatLayout->addWidget( label );

    m_comboBox = new QComboBox( this );
    formatLayout->addWidget( m_comboBox );

    m_comboBox->addItem( tr( "Plain text" ) );
    m_comboBox->addItem( tr( "Text with markup" ) );

    label->setBuddy( m_comboBox );
    
    formatLayout->addStretch( 1 );

    connect( m_comboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( formatChanged( int ) ) );

    setFocusProxy( m_edit );
}

MarkupTextEdit::~MarkupTextEdit()
{
}

void MarkupTextEdit::setInputValue( const QString& value )
{
    m_edit->setInputValue( value );
}

QString MarkupTextEdit::inputValue()
{
    return m_edit->inputValue();
}

bool MarkupTextEdit::validate()
{
    return m_edit->validate();
}

void MarkupTextEdit::setRequired( bool required )
{
    m_edit->setRequired( required );
}

bool MarkupTextEdit::isRequired() const
{
    return m_edit->isRequired();
}

void MarkupTextEdit::setTextFormat( TextFormat format )
{
    m_comboBox->setCurrentIndex( (int)format );
}

TextFormat MarkupTextEdit::textFormat() const
{
    return (TextFormat)m_comboBox->currentIndex();
}

void MarkupTextEdit::markupBold()
{
    markup( "**", "**" );
}

void MarkupTextEdit::markupItalic()
{
    markup( "__", "__" );
}

void MarkupTextEdit::markupMonospace()
{
    markup( "`", "`" );
}

void MarkupTextEdit::markupLink()
{
    QString url = QInputDialog::getText( this, tr( "Hyperlink" ), tr( "Enter link URL:" ), QLineEdit::Normal, "http://" );
    if ( !url.isEmpty() )
        markup( QString( "[%1 " ).arg( url ), "]", tr( "Link text" ) );
}

void MarkupTextEdit::markupList()
{
    markupBlock( "[list]\n", "\n[/list]", "* ", "" );
}

void MarkupTextEdit::markupQuote()
{
    markup( "[quote]\n", "\n[/quote]" );
}

void MarkupTextEdit::markupCode()
{
    markup( "[code]\n", "\n[/code]" );
}

void MarkupTextEdit::markup( const QString& openWith, const QString& closeWith, const QString& placeholder /* = QString() */ )
{
    QTextCursor cursor = m_edit->textCursor();
    QString text = cursor.selectedText();
    if ( !text.isEmpty() ) {
        cursor.insertText( openWith + text + closeWith );
    } else if ( placeholder.isEmpty() ) {
        cursor.insertText( openWith + closeWith );
        cursor.movePosition( QTextCursor::Left, QTextCursor::MoveAnchor, closeWith.length() );
        m_edit->setTextCursor( cursor );
    } else {
        cursor.insertText( openWith + placeholder + closeWith );
        cursor.movePosition( QTextCursor::Left, QTextCursor::MoveAnchor, closeWith.length() + placeholder.length() );
        cursor.movePosition( QTextCursor::Right, QTextCursor::KeepAnchor, placeholder.length() );
        m_edit->setTextCursor( cursor );
    }
}

void MarkupTextEdit::markupBlock( const QString& openBlockWith, const QString& closeBlockWith, const QString& openWith, const QString& closeWith )
{
    QTextCursor cursor = m_edit->textCursor();
    QString text = cursor.selectedText();
    if ( !text.isEmpty() ) {
        QChar separator( QChar::ParagraphSeparator );
        text.replace( separator, closeWith + separator + openWith );
        cursor.insertText( openBlockWith + openWith + text + closeWith + closeBlockWith );
    } else {
        cursor.insertText( openBlockWith + openWith + closeWith + closeBlockWith );
        cursor.movePosition( QTextCursor::Left, QTextCursor::MoveAnchor, closeBlockWith.length() + closeWith.length() );
        m_edit->setTextCursor( cursor );
    }
}

void MarkupTextEdit::showPreview()
{
    if ( !m_previewWindow ) {
        m_previewWindow = new QWidget( this, Qt::Tool );
        m_previewWindow->setWindowTitle( tr( "Preview" ) );

        QVBoxLayout* layout = new QVBoxLayout( m_previewWindow );
        layout->setMargin( 0 );

        m_preview = new QWebView( m_previewWindow );
        layout->addWidget( m_preview );

        m_preview->setContextMenuPolicy( Qt::NoContextMenu );
        m_preview->page()->setLinkDelegationPolicy( QWebPage::DelegateAllLinks );

        m_previewWindow->resize( m_edit->size() );

        connect( m_preview, SIGNAL( linkClicked( const QUrl& ) ), this, SLOT( linkClicked( const QUrl& ) ) );
    }

    m_previewWindow->show();

    HtmlWriter writer;
    writer.writeBlock( MarkupProcessor::parse( m_edit->inputValue() ), HtmlWriter::CommentBlock );

    m_preview->setHtml( writer.toHtml() );
}

void MarkupTextEdit::linkClicked( const QUrl& url )
{
    QString scheme = url.scheme().toLower();

    if ( scheme == QLatin1String( "id" ) ) {
        int itemId = url.host().toInt();
        findItem( itemId );
    } else {
        Application::openUrl( m_previewWindow, url );
    }
}

void MarkupTextEdit::findItem( int itemId )
{
    int issueId = FindItemDialog::getFindItem( m_previewWindow, itemId );
    if ( issueId != 0 )
        viewManager->openIssueView( issueId, itemId );
}

void MarkupTextEdit::formatChanged( int format )
{
    m_toolBar->setVisible( format == TextWithMarkup );

    if ( format == PlainText && m_previewWindow != NULL )
        m_previewWindow->close();
}

QSize MarkupTextEdit::sizeHint() const
{
    return QSize( 700, 350 );
}
