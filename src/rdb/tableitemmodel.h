/****************************************************************************
* Simple template-based relational database
* Copyright (C) 2006-2011 Michał Męciński
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

#ifndef RDB_TABLEITEMMODEL_H
#define RDB_TABLEITEMMODEL_H

#include "rdb.h"
#include "abstracttablemodel.h"

#include <QAbstractItemModel>
#include <QList>

namespace RDB
{

class AbstractRowFilter;

/**
* Item model populated with items from %RDB tables.
*
* This model implements QAbstractItemModel and connects one or more
* instances of AbstractTableModel to provide a list or tree of rows.
*
* The top level items can be either all rows from a table or rows matching
* the given foreign key value. They can be additionally filtered using
* an AbstractRowFilter.
*
* Child items are rows from another table whose foreign key match
* the primary key of their parent item. Child tables can be nested
* to any level.
*
* The list of column used by the model can be set using the setColumns()
* method. This allows to map column indexes to logical identifiers passed
* to the table models.
*
* Use the updateData() method to update the model and its views when data
* is chaged. The model automatically updates itself when table models emit
* update signal.
*/
class TableItemModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum Roles
    {
        RowIdRole = Qt::UserRole,
        LevelRole,
        ColumnRole
    };

public:
    /**
    * Constructor.
    * @param parent The parent object.
    */
    TableItemModel( QObject* parent );

    /**
    * Destructor.
    */
    ~TableItemModel();

public:
    /**
    * Set the list of columns for this model.
    */
    void setColumns( const QList<int>& columns );

    /**
    * Return the list of columns of this model.
    */
    const QList<int>& columns() const { return m_columns; }

    /**
    * Set the filter for top level rows.
    * @param filter The row filter or @c NULL to disable filtering.
    */
    void setRowFilter( AbstractRowFilter* filter );

    /**
    * Set the top level table model.
    * This method creates top level items using all rows of a table.
    * @param model The model for the top level table.
    * @param index The primary index of the table.
    */
    void setRootTableModel( AbstractTableModel* model, const RDB::UniqueConstIndex<void>& index );

    /**
    * Set the top level table model.
    * This method creates top level items using table rows matching a given value
    * of a foreign key.
    * @param model The model for the top level table.
    * @param index Index identifying rows in the table.
    * @param parentIndex Foreign index for matching with the given value.
    * @param parentId The value of the foreign key to match.
    */
    void setRootTableModel( AbstractTableModel* model, const RDB::UniqueConstIndex<void>& index,
        const RDB::ForeignConstIndex<void>& parentIndex, int parentId );

    /**
    * @overload
    */
    void setRootTableModel( AbstractTableModel* model, const RDB::ForeignConstIndex<void>& index,
        const RDB::ForeignConstIndex<void>& parentIndex, int parentId );

    /**
    * Return the top level table model.
    */
    AbstractTableModel* rootTableModel() const;

    /**
    * Add a nested child table model.
    * This method adds a new level of child items using rows from the given table
    * whose foreign key match the primary key of their parent items.
    * @param model The model for the child table to add.
    * @param index Index identifying rows in the table.
    * @param parentIndex Foreign index for matching with parent table.
    */
    void addChildTableModel( AbstractTableModel* model, const RDB::UniqueConstIndex<void>& index,
        const RDB::ForeignConstIndex<void>& parentIndex );

    /**
    * @overload
    */
    void addChildTableModel( AbstractTableModel* model, const RDB::ForeignConstIndex<void>& index,
        const RDB::ForeignConstIndex<void>& parentIndex );

    /**
    * Return the total number of top level rows.
    * The number of rows includes rows that were filtered out.
    */
    int totalCount() const { return m_totalCount; }

    /**
    * Return the column by which the data is sorted.
    */
    int sortColumn() const { return m_sortColumn; }

    /**
    * Return the sort order of the data.
    */
    Qt::SortOrder sortOrder() const { return m_sortOrder; }

    /**
    * Find the index of the specified cell.
    * @param level The nest level of the cell.
    * @param id The identifier of the row.
    * @param column The identifier of the column.
    */
    QModelIndex findCell( int level, int id, int column );

public: // overrides
    int columnCount( const QModelIndex& parent = QModelIndex() ) const;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

    int rowCount( const QModelIndex& parent = QModelIndex() ) const;
    QModelIndex index( int row, int column, const QModelIndex& parent = QModelIndex() ) const;
    QModelIndex parent( const QModelIndex& index ) const;

    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;

    void sort( int column, Qt::SortOrder order = Qt::AscendingOrder );

public slots:
    /**
    * Update the model and views connected to it.
    */
    void updateData();

private:
    struct Item
    {
        int m_id;
        int m_childGroup;
    };

    struct ItemGroup
    {
        int m_level;
        QList<Item> m_items;
    };

    struct CellIndex
    {
        int m_level;
        int m_id;
        int m_column;
    };

    class CompareItems
    {
    public:
        CompareItems( AbstractTableModel* model, int column, Qt::SortOrder order );

        bool operator()( const Item& item1, const Item& item2 );

    private:
        AbstractTableModel* m_model;
        int m_column;
        Qt::SortOrder m_order;
    };

private:
    void attachTableModel( AbstractTableModel* model, int dim );

    void buildAllIndexes();
    void buildRootIndex();
    void buildChildIndexes( int group );

    int addGroup( int level );
    void addItemToGroup( int group, int id );

    void sortAllGroups();
    void sortGroup( int group );

    CellIndex cellAt( const QModelIndex& index );
    QModelIndex findCell( const CellIndex& cell );

private:
    QList<int> m_columns;

    int m_sortColumn;
    Qt::SortOrder m_sortOrder;

    AbstractRowFilter* m_filter;

    QList<AbstractTableModel*> m_models;

    RDB::UniqueConstIndex<void> m_uniqueIndex;
    RDB::ForeignConstIndex<void> m_foreignIndex;
    int m_parentId;

    QList<RDB::ForeignConstIndex<void> > m_childIndexes;

    QList<int> m_keyDims;

    QList<ItemGroup> m_itemGroups;

    int m_totalCount;
};

}

#endif
