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

#ifndef RDB_ABSTRACTROWFILTER_H
#define RDB_ABSTRACTROWFILTER_H

#include <QObject>

namespace RDB
{

/**
* Abstract filter for filtering table rows.
*
* The inherited class must implement the filterRow() method.
*
* @see TableItemModel
*/
class AbstractRowFilter : public QObject
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent The parent object.
    */
    AbstractRowFilter( QObject* parent );

    /**
    * Destructor.
    */
    ~AbstractRowFilter();

public:
    /**
    * Filter the given row.
    * @param id Identifier of the row to filter.
    * @return @c true if the row should be included, @c false if it should be hidden.
    */
    virtual bool filterRow( int id ) = 0;

signals:
    /**
    * Emit this signal when the model needs to be updated.
    * This signal is automatically connected to TableItemModel::updateData().
    */
    void conditionsChanged();
};

}

#endif
