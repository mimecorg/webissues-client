/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2013 WebIssues Team
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

#ifndef FINDITEMDIALOG_H
#define FINDITEMDIALOG_H

#include "dialogs/commanddialog.h"

class FindItemBatch;
class NumericLineEdit;

/**
* Dialog for executing the <tt>FIND ITEM</tt> command.
*/
class FindItemDialog : public CommandDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent The parent widget.
    */
    FindItemDialog( QWidget* parent );

    /**
    * Destructor.
    */
    ~FindItemDialog();

public:
    /**
    * Execute the command automatically.
    * @param itemId The identifier of the item to find.
    */
    void findItem( int itemId );

    /**
    * Return the identifier of the searched item.
    */
    int itemId() const { return m_itemId; }

    /**
    * Return the identifier of the issue that was found.
    */
    int issueId() const { return m_issueId; }

public: // overrides
    void accept();

protected: // overrides
    bool batchSuccessful( AbstractBatch* batch );

private:
    NumericLineEdit* m_idEdit;

    int m_itemId;
    int m_issueId;
};

#endif
