/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2014 WebIssues Team
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

#include "separatorcombobox.h"

#include <QItemDelegate>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QPainter>

class SeparatorItemDelegate : public QItemDelegate
{
public:
    SeparatorItemDelegate( QObject* parent ) : QItemDelegate( parent )
    {
    }

    ~SeparatorItemDelegate()
    {
    }

public: // overrides
    void paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        QString type = index.data( Qt::AccessibleDescriptionRole ).toString();

        if ( type == QLatin1String( "separator" ) ) {
            QItemDelegate::paint( painter, option, index );
            int y = ( option.rect.top() + option.rect.bottom() ) / 2;
            painter->setPen(  option.palette.color( QPalette::Active, QPalette::Dark ) );
            painter->drawLine( option.rect.left(), y, option.rect.right(), y );
        } else if ( type == QLatin1String( "parent" ) ) {
            QStyleOptionViewItem parentOption = option;
            parentOption.state |= QStyle::State_Enabled;
            QItemDelegate::paint( painter, parentOption, index );
        } else if ( type == QLatin1String( "child" ) ) {
            QStyleOptionViewItem childOption = option;
            int indent = option.fontMetrics.width( QString( 4, QChar( ' ' ) ) );
            childOption.rect.adjust( indent, 0, 0, 0 );
            childOption.textElideMode = Qt::ElideNone;
            QItemDelegate::paint( painter, childOption, index );
        } else {
            QItemDelegate::paint( painter, option, index );
        }
    }

    QSize sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        QString type = index.data( Qt::AccessibleDescriptionRole ).toString();
        if ( type == QLatin1String( "separator" ) )
            return QSize( 5, 5 );
        return QItemDelegate::sizeHint( option, index );
    }
};

SeparatorComboBox::SeparatorComboBox( QWidget* parent ) : QComboBox( parent )
{
    setItemDelegate( new SeparatorItemDelegate( this ) );
}

SeparatorComboBox::~SeparatorComboBox()
{
}

void SeparatorComboBox::addSeparator()
{
    insertSeparator( count() );
}

void SeparatorComboBox::addParentItem( const QString& text )
{
    QStandardItem* item = new QStandardItem( text );
    item->setFlags( item->flags() & ~( Qt::ItemIsEnabled | Qt::ItemIsSelectable ) );
    item->setData( "parent", Qt::AccessibleDescriptionRole );

    QFont font = item->font();
    font.setBold( true );
    item->setFont( font );

    QStandardItemModel* itemModel = (QStandardItemModel*)model();
    itemModel->appendRow( item );
}

void SeparatorComboBox::addChildItem( const QString& text, const QVariant& data )
{
    QStandardItem* item = new QStandardItem( text + QString( 4, QChar( ' ' ) ) );
    item->setData( data, Qt::UserRole );
    item->setData( "child", Qt::AccessibleDescriptionRole );

    QStandardItemModel* itemModel = (QStandardItemModel*)model();
    itemModel->appendRow( item );
}
