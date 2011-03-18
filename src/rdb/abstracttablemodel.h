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

#ifndef RDB_ABSTRACTTABLEMODEL_H
#define RDB_ABSTRACTTABLEMODEL_H

#include <QObject>

class QPixmap;

namespace RDB
{

/**
* Abstract model providing information about rows of a table.
*
* One or more table models can be added to a TableItemModel
* in order to populate a list or tree view.
*
* Inherited classes must implement at least the columnName()
* an text() methods.
*
* @see TableItemModel
*/
class AbstractTableModel : public QObject
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent The parent object.
    */
    AbstractTableModel( QObject* parent );

    /**
    * Destructor.
    */
    ~AbstractTableModel();

public:
    /**
    * Return the header text of the given column.
    */
    virtual QString columnName( int column ) const = 0;

    /**
    * Return the text of a given column of a row.
    * This method must be implemented in the inherited class.
    */
    virtual QString text( int id, int column ) const = 0;

    /**
    * Return the icon of a given column of a row.
    * The default implementation returns an empty pixmap.
    */
    virtual QPixmap icon( int id, int column ) const;

    /**
    * Return the tool tip of a given row.
    * The default implementation returns an empty string.
    */
    virtual QString toolTip( int id, int column ) const;

    /**
    * Compare two rows using given column.
    * The default implementation compares the texts using QString::localeAwareCompare().
    */
    virtual int compare( int id1, int id2, int column ) const;

signals:
    /**
    * Emit this signal when the model needs to be updated.
    * This signal is automatically connected to TableItemModel::updateData().
    */
    void dataChanged();
};

}

#endif
