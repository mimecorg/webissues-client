/****************************************************************************
* Simple XML-based UI builder for Qt4
* Copyright (C) 2007-2011 Michał Męciński
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*   1. Redistributions of source code must retain the above copyright notice,
*      this list of conditions and the following disclaimer.
*   2. Redistributions in binary form must reproduce the above copyright
*      notice, this list of conditions and the following disclaimer in the
*      documentation and/or other materials provided with the distribution.
*   3. Neither the name of the copyright holder nor the names of the
*      contributors may be used to endorse or promote products derived from
*      this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
****************************************************************************/

#include "macstyle.h"

#if !defined( XMLUI_NO_STYLE_MAC )

#include "gradientwidget.h"
#include "toolstrip.h"

#include <QStyleOption>
#include <QPainter>
#include <QLibrary>
#include <QApplication>
#include <QMainWindow>
#include <QAbstractButton>
#include <QScrollArea>
#include <QStatusBar>
#include <QMenu>
#include <QMenuBar>
#include <QLayout>
#include <QDebug>

#if !defined( XMLUI_NO_STYLE_PLUGIN )
#include <QStylePlugin>
#endif

using namespace XmlUi;

MacStyle::MacStyle() : MacBaseStyle()
{
}

MacStyle::~MacStyle()
{
}

static QColor blendColors( const QColor& src, const QColor& dest, double alpha )
{
    double red = alpha * src.red() + ( 1.0 - alpha ) * dest.red();
    double green = alpha * src.green() + ( 1.0 - alpha ) * dest.green();
    double blue = alpha * src.blue() + ( 1.0 - alpha ) * dest.blue();
    return QColor( (int)( red + 0.5 ), (int)( green + 0.5 ), (int)( blue + 0.5 ) );
}

static void drawHighlightFrameMac( QPainter* painter, const QWidget* widget, const QStyleOptionComplex* option, const QRect& rect, const QColor& begin, const QColor& middle, const QColor& end, const QColor& border, const QColor& inner, bool roundLeft, bool roundRight )
{
    const QSize size( 50, 34 );
    int margin = 6;
    
    QStyleOptionButton opt;
    opt.initFrom( widget );
    opt.direction = option->direction;
    opt.fontMetrics = option->fontMetrics;
    opt.palette = option->palette;
    opt.rect = QRect( QPoint(), size );
    opt.state = option->state | QStyle::State_Sunken;
    
    // grab original rendering
    QPixmap pixmap( size );
    pixmap.fill( QColor( Qt::transparent ) );
    {
        QPainter p( &pixmap );
        widget->style()->drawControl( QStyle::CE_PushButton, &opt, &p, 0 );
    }
    
    // create box pixmap
    {
        QPixmap pix = QPixmap( 34, 34 );
        pix.fill( QColor( Qt::transparent ) );
        {
            QPainter p( &pix );
            
            p.drawPixmap( QPoint(), pixmap.copy( QRect( QPoint(), QSize( 17, 34 ) ) ) );
            p.drawPixmap( QPoint( 16, 0 ), pixmap.copy( QRect( QPoint( pixmap.width() -16, 0 ), QSize( 17, 34 ) ) ) );
        }
        
        pixmap = pix;
    }
    
    QRect r = QRect( QPoint(), rect.size() ).adjusted( -margin, -margin, margin, margin );
    
    // paint final pixmap
    {
        const QSize s16( rect.width() >= 26 ? 16 : rect.width() /2, rect.height() >= 26 ? 16 : rect.height() /2 );
        const QPixmap topLeft = pixmap.copy( QRect( QPoint(), s16 ) );
        const QPixmap topRight = pixmap.copy( QRect( QPoint( pixmap.width() -( s16.width() +2 ), 0 ), s16 ) );
        const QPixmap bottomLeft = pixmap.copy( QRect( QPoint( 0, pixmap.height() -( s16.height() +2 ) ), s16 ) );
        const QPixmap bottomRight = pixmap.copy( QRect( QPoint( pixmap.width() -( s16.width() +2 ), pixmap.height() -( s16.height() +2 ) ), s16 ) );
        const QPixmap left = pixmap.copy( QRect( QPoint( 0, pixmap.height() /2 ), QSize( s16.width(), 1 ) ) );
        const QPixmap top = pixmap.copy( QRect( QPoint( pixmap.width() /2, 0 ), QSize( 1, s16.height() ) ) );
        const QPixmap right = pixmap.copy( QRect( QPoint( pixmap.width() /2, pixmap.height() /2 ), QSize( s16.width(), 1 ) ) );
        const QPixmap bottom = pixmap.copy( QRect( QPoint( pixmap.width() /2, pixmap.height() /2 ), QSize( 1, s16.height() ) ) );
        const QPixmap center = pixmap.copy( QRect( QPoint( pixmap.width() /2, pixmap.height() /2 ), QSize( 2, 2 ) ) );
        
        QPixmap pix = QPixmap( r.size() );
        pix.fill( QColor( Qt::transparent ) );
        
        {
            QPainter p( &pix );
            
            p.drawPixmap( r.topLeft() +QPoint( margin, margin ), topLeft );
            p.drawPixmap( r.topRight() +QPoint( -s16.width() +margin, 0 +margin ), topRight );
            p.drawPixmap( r.bottomLeft() +QPoint( 0 +margin, -s16.height() +margin ), bottomLeft );
            p.drawPixmap( r.bottomRight() +QPoint( -s16.width() +margin, -s16.height() +margin ), bottomRight );
            
            p.drawPixmap( QRect( QPoint( 0, s16.height() ), QSize( s16.width(), r.height() -( s16.height() *2 ) ) ), left );
            p.drawPixmap( QRect( QPoint( s16.width(), 0 ), QSize( r.width() -( s16.width() *2 ), s16.height() ) ), top );
            p.drawPixmap( QRect( QPoint( r.width() -s16.width(), s16.height() ), QSize( s16.width(), r.height() -( s16.height() *2 ) ) ), right );
            p.drawPixmap( QRect( QPoint( s16.width(), r.height() -s16.height() ), QSize( r.width() -( s16.width() *2 ), s16.height() ) ), bottom );
            
            p.drawPixmap( r.adjusted( s16.width() +margin, s16.height() +margin, -s16.width() +margin, -s16.height() +margin ), center );
        }
        
        pixmap = pix;
    }
    
    // draw scaled content
    painter->setRenderHint( QPainter::SmoothPixmapTransform );
    painter->drawPixmap( rect.topLeft() -QPoint( margin, margin ), pixmap );
}

static void drawHighlightFrame( QPainter* painter, const QWidget* widget, const QStyleOptionComplex* option, const QRect& rect, const QColor& begin, const QColor& middle, const QColor& end, const QColor& border, const QColor& inner, bool roundLeft, bool roundRight )
{
    if ( rect.size() != QSize( 26, 26 ) ) {
        drawHighlightFrameMac( painter, widget, option, rect, begin, middle, end, border, inner, roundLeft, roundRight );
        return;
    }
    
    painter->save();

    QRect frameRect = rect.adjusted( 0, 0, -1, -1 );

    QLinearGradient gradient( frameRect.topLeft(), frameRect.bottomLeft() );
    gradient.setColorAt( 0.0, begin );
    gradient.setColorAt( 0.5, middle );
    gradient.setColorAt( 1.0, end );

    QRegion region = rect;
    if ( roundLeft ) {
        region -= QRect( rect.topLeft(), QSize( 1, 1 ) );
        region -= QRect( rect.bottomLeft(), QSize( 1, 1 ) );
    }
    if ( roundRight ) {
        region -= QRect( rect.topRight(), QSize( 1, 1 ) );
        region -= QRect( rect.bottomRight(), QSize( 1, 1 ) );
    }
    painter->setClipRegion( region );

    painter->setPen( border );
    painter->setBrush( gradient );
    painter->drawRect( frameRect );

    painter->setPen( inner );
    painter->setBrush( QBrush() );
    painter->drawRect( frameRect.adjusted( 1, 1, -1, -1 ) );
    if ( roundLeft ) {
        painter->drawPoint( rect.left() + 2, rect.top() + 2 );
        painter->drawPoint( rect.left() + 2, rect.bottom() - 2 );
    }
    if ( roundRight ) {
        painter->drawPoint( rect.right() - 2, rect.top() + 2 );
        painter->drawPoint( rect.right() - 2, rect.bottom() - 2 );
    }

    painter->setPen( border );
    if ( roundLeft ) {
        painter->drawPoint( rect.left() + 1, rect.top() + 1 );
        painter->drawPoint( rect.left() + 1, rect.bottom() - 1 );
    }
    if ( roundRight ) {
        painter->drawPoint( rect.right() - 1, rect.top() + 1 );
        painter->drawPoint( rect.right() - 1, rect.bottom() - 1 );
    }

    painter->restore();
}

void MacStyle::polish( QPalette& palette )
{
    MacBaseStyle::polish( palette );

    QColor button = palette.color( QPalette::Button );
    QColor base = palette.color( QPalette::Base );
    QColor text = palette.color( QPalette::Text );
    QColor dark = palette.color( QPalette::Dark );
    QColor light = palette.color( QPalette::Light );
    QColor shadow = palette.color( QPalette::Shadow );
    QColor highlight = palette.color( QPalette::Highlight );

    highlight = QColor::fromHsv( highlight.hue(), 204, 255 ).toRgb();

    m_colorBackgroundBegin = button;
    m_colorBackgroundEnd = blendColors( button, base, 0.205 );
    m_colorMenuBorder = blendColors( text, dark, 0.2 );
    m_colorMenuBackground = blendColors( button, base, 0.143 );
    m_colorBarBegin = blendColors( button, base, 0.2 );
    m_colorBarEnd = blendColors( button, dark, 0.8 );
    m_colorSeparator = blendColors( dark, base, 0.5 );
    m_colorItemBorder = highlight;
    
    m_colorItemBackgroundBegin = blendColors( highlight, base, 0.2 );
    m_colorItemBackgroundMiddle = blendColors( highlight, base, 0.4 );
    m_colorItemBackgroundEnd = blendColors( highlight, light, 0.1 );

    m_colorItemCheckedBegin = blendColors( highlight, base, 0.2 );
    m_colorItemCheckedMiddle = blendColors( highlight, light, 0.1 );
    m_colorItemCheckedEnd = blendColors( highlight, base, 0.4 );
    
    m_colorItemSunkenBegin = blendColors( highlight, base, 0.3 );
    m_colorItemSunkenMiddle = blendColors( highlight, base, 0.5 );
    m_colorItemSunkenEnd = blendColors( highlight, light, 0.2 );
    
    m_colorToolStripLabel = blendColors( highlight, shadow, 0.3 );
}

void MacStyle::polish( QWidget* widget )
{
    if ( qobject_cast<QMainWindow*>( widget ) )
        widget->setAttribute( Qt::WA_StyledBackground );

    if ( qobject_cast<GradientWidget*>( widget ) )
        widget->setAttribute( Qt::WA_StyledBackground );

    /*if ( qobject_cast<ToolStrip*>( widget ) ) {
        QPalette palette = widget->palette();
        palette.setColor( QPalette::Text, m_colorToolStripLabel );
        widget->setPalette( palette );
    }*/

    MacBaseStyle::polish( widget );
}

void MacStyle::unpolish( QWidget* widget )
{
    if ( qobject_cast<QMainWindow*>( widget ) )
        widget->setAttribute( Qt::WA_StyledBackground, false );

    if ( qobject_cast<GradientWidget*>( widget ) )
        widget->setAttribute( Qt::WA_StyledBackground, false );

    /*if ( qobject_cast<ToolStrip*>( widget ) )
        widget->setPalette( QApplication::palette( widget ) );*/

    MacBaseStyle::unpolish( widget );
}

void MacStyle::drawComplexControl( ComplexControl control, const QStyleOptionComplex* option,
    QPainter* painter, const QWidget* widget ) const
{
    switch ( control ) {
        case CC_ToolButton:
            if ( option->state & State_AutoRaise ) {
                if ( const QStyleOptionToolButton* optionToolButton = qstyleoption_cast<const QStyleOptionToolButton*>( option ) ) {
                    QRect buttonRect = subControlRect( control, option, SC_ToolButton, widget );
                    QStyle::State buttonState = option->state & ~State_Sunken;
                    if ( option->state & State_Sunken ) {
                        if ( optionToolButton->activeSubControls & SC_ToolButton )
                            buttonState |= State_Sunken;
                        else if ( optionToolButton->activeSubControls & SC_ToolButtonMenu )
                            buttonState |= State_MouseOver;
                    }
                    bool selected = buttonState & State_MouseOver && option->state & State_Enabled;
                    bool checked = buttonState & State_On;
                    bool sunken = buttonState & State_Sunken;
                    if ( selected || checked || sunken ) {
                        bool roundRight = !( optionToolButton->subControls & SC_ToolButtonMenu );
                        if ( sunken || selected && checked )
                            drawHighlightFrame( painter, widget, option, buttonRect, m_colorItemSunkenBegin, m_colorItemSunkenMiddle, m_colorItemSunkenEnd, m_colorItemBorder, m_colorItemSunkenBegin, true, roundRight );
                        else if ( checked )
                            drawHighlightFrame( painter, widget, option, buttonRect, m_colorItemCheckedBegin, m_colorItemCheckedMiddle, m_colorItemCheckedEnd, m_colorItemBorder, m_colorItemCheckedEnd, true, roundRight );
                        else
                            drawHighlightFrame( painter, widget, option, buttonRect, m_colorItemBackgroundBegin, m_colorItemBackgroundMiddle, m_colorItemBackgroundEnd, m_colorItemBorder, m_colorItemBackgroundEnd, true, roundRight );
                    }
                    QStyleOptionToolButton optionLabel = *optionToolButton;
                    optionLabel.state = buttonState;
                    int fw = pixelMetric( PM_DefaultFrameWidth, option, widget );
                    optionLabel.rect = buttonRect.adjusted( fw, fw, -fw, -fw );
                    drawControl( CE_ToolButtonLabel, &optionLabel, painter, widget );
                    if ( optionToolButton->subControls & SC_ToolButtonMenu ) {
                        QRect menuRect = subControlRect( control, option, SC_ToolButtonMenu, widget );
                        menuRect.adjust( -1, 0, 0, 0 );
                        if ( sunken || optionToolButton->state & State_Sunken && optionToolButton->activeSubControls & SC_ToolButtonMenu )
                            drawHighlightFrame( painter, widget, option, menuRect, m_colorItemSunkenBegin, m_colorItemSunkenMiddle, m_colorItemSunkenEnd, m_colorItemBorder, m_colorItemSunkenBegin, false, true );
                        else if ( selected )
                            drawHighlightFrame( painter, widget, option, menuRect, m_colorItemBackgroundBegin, m_colorItemBackgroundMiddle, m_colorItemBackgroundEnd, m_colorItemBorder, m_colorItemBackgroundEnd, false, true );
                        QStyleOptionToolButton optionArrow = *optionToolButton;
                        optionArrow.rect = menuRect.adjusted( 2, 3, -1, -3 );
                        drawPrimitive( PE_IndicatorArrowDown, &optionArrow, painter, widget );
                    } else if ( optionToolButton->features & QStyleOptionToolButton::HasMenu ) {
                        int size = pixelMetric( PM_MenuButtonIndicator, option, widget );
                        QRect rect = optionToolButton->rect;
                        QStyleOptionToolButton optionArrow = *optionToolButton;
                        optionArrow.rect = QRect( rect.right() + 4 - size, rect.height() - size + 4, size - 5, size - 5 );
                        drawPrimitive( PE_IndicatorArrowDown, &optionArrow, painter, widget );
                    }
                    return;
            }
            break;
        }

        default:
            break;
    }

    // TODO: QMacStyle
    MacBaseStyle::drawComplexControl( control, option, painter, widget );
}

#if !defined( XMLUI_NO_STYLE_PLUGIN )

namespace XmlUi
{

class MacStylePlugin : public QStylePlugin
{
public: // overrides
    QStringList keys() const;
    QStyle* create( const QString& key );
};

QStringList MacStylePlugin::keys() const
{
    return QStringList() << "XmlUi::MacStyle";
}

QStyle* MacStylePlugin::create( const QString& key )
{
    if ( key.toLower() == QLatin1String( "xmlui::macstyle" ) )
        return new MacStyle();
    return NULL;
}

#if !defined( XMLUI_EXPORT_STYLE_PLUGIN )

QObject* qt_plugin_instance_xmlui_macstyle()
{
    static QPointer<QObject> instance;
    if ( !instance )
        instance = new MacStylePlugin();
    return instance;
}

Q_IMPORT_PLUGIN( xmlui_macstyle )

#else

Q_EXPORT_PLUGIN2( xmlui_macstyle, MacStylePlugin )

#endif // !defined( XMLUI_EXPORT_STYLE_PLUGIN )

}

#endif // !defined( XMLUI_NO_STYLE_PLUGIN )

#endif // !defined( XMLUI_NO_STYLE_MAC )
