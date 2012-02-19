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

#ifndef SQLTREEMODEL_H
#define SQLTREEMODEL_H

#include <QAbstractItemModel>

class SqlTreeModelPrivate;

class QSqlQueryModel;

/**
* Item model which builds a tree from multiple SQL models.
* Items at the root of the tree correspond to items from the first SQL model.
* The next nesting levels of the tree are built from the following SQL models
* by matching the foreign keys of their items with the primary keys of parent
* items. In the first SQL model, the first column must be the primary key.
* In the following SQL models, the first column must be the primary key
* and the second column must be the foreign key.
*/
class SqlTreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent The parent object.
    */
    explicit SqlTreeModel( QObject* parent = NULL );

    /**
    * Destructor.
    */
    ~SqlTreeModel();

public:
    /**
    * Append the SQL model as the next level of the tree.
    * @param model The SQL model to append.
    */
    void appendModel( QSqlQueryModel* model );

    /**
    * Return the SQL model at the given level.
    * @param level The level of the tree (numbered from zero).
    * @return The SQL model.
    */
    QSqlQueryModel* modelAt( int level ) const;

    /**
    * Map tree model columns to SQL model columns.
    * By default all columns are mapped except the primary and foreign keys.
    * @param level The level of the tree (numbered from zero).
    * @param columnMapping A list of column indexes from the SQL model.
    * Use -1 to indicate that a column is calculated.
    */
    void setColumnMapping( int level, const QList<int>& columnMapping );

    /**
    * Rebuild the tree model after updating SQL models.
    */
    void updateData();

    /**
    * Return the level of the given item.
    * @param index The index of the item.
    * @return The level of the item (numbered from zero) or -1 if index is
    * not valid.
    */
    int levelOf( const QModelIndex& index ) const;

    /**
    * Return the row number in the SQL model of the given item.
    * @param index The index of the item.
    * @return The mapped row number or -1 if index is not valid.
    */
    int mappedRow( const QModelIndex& index ) const;

    /**
    * Return the column number in the SQL model of the given item.
    * @param index The index of the item.
    * @return The mapped column number or -1 if index is not valid.
    */
    int mappedColumn( const QModelIndex& index ) const;

    /**
    * Return the primary key of the given item.
    * @param index The index of the item.
    * @return The value of the primary key or -1 if index is not valid.
    */
    int rowId( const QModelIndex& index ) const;

    /**
    * Return the foreign key of the given item.
    * @param index The index of the item.
    * @return The value of the foreign key or -1 if index is not valid.
    */
    int rowParentId( const QModelIndex& index ) const;

    /**
    * Return the data from the SQL model.
    * @param level The level of the item (numbered from zero).
    * @param row The mapped row number of the item.
    * @param column The mapped column number of the item.
    * @param role The role to retrieve.
    * @return The data for the given cell and role.
    */
    QVariant rawData( int level, int row, int column, int role = Qt::DisplayRole ) const;

    /**
    * Return the index of an item with given level and primary key.
    * @param level The level of the item (numbered from zero).
    * @param id The primary key of the item.
    * @param column The column of the cell to return.
    * @return The item index or an invalid index if the item was not found.
    */
    QModelIndex findIndex( int level, int id, int column ) const;

    /**
    * Set the sort order without updating the model.
    * @param column The index of the sort column.
    * @param order The sort order.
    */
    void setSort( int column, Qt::SortOrder order = Qt::AscendingOrder );

    /**
    * Return the index of the current sort column.
    */
    int sortColumn() const;

    /**
    * Return the current sort order.
    */
    Qt::SortOrder sortOrder() const;

public: // overrides
    int columnCount( const QModelIndex& parent = QModelIndex() ) const;
    int rowCount( const QModelIndex& parent = QModelIndex() ) const;

    QModelIndex index( int row, int column, const QModelIndex& parent = QModelIndex() ) const;
    QModelIndex parent( const QModelIndex& index ) const;

    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;

    bool setHeaderData( int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole );
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

    void sort( int column, Qt::SortOrder order = Qt::AscendingOrder );

protected:
    /**
    * Called to update the queries of child SQL models after changing
    * sort order.
    */
    virtual void updateQueries();

private:
    SqlTreeModelPrivate* d;
};

#endif
