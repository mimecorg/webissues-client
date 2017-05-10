/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2017 WebIssues Team
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

#ifndef MANAGEALERTSDIALOG_H
#define MANAGEALERTSDIALOG_H

#include "dialogs/informationdialog.h"
#include "xmlui/client.h"

class AlertsModel;

class QTreeView;
class QModelIndex;

/**
* Base dialog for managing alerts.
*/
class ManageAlertsDialog : public InformationDialog, public XmlUi::Client
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param folderId Identifier of the folder.
    */
    ManageAlertsDialog( int folderId );

    /**
    * Destructor.
    */
    ~ManageAlertsDialog();

protected:
    void initializeList( AlertsModel* model );

protected slots:
    virtual void addAlert() = 0;
    virtual void addPublicAlert() = 0;

    virtual void updateActions();

private slots:
    void editDelete();
    void editModify();

    void doubleClicked( const QModelIndex& index );
    void listContextMenu( const QPoint& pos );

private:
    QTreeView* m_list;
    AlertsModel* m_model;

    bool m_emailEnabled;
    bool m_canEditPublic;

    int m_selectedAlertId;
    bool m_selectedEditable;
};

/**
* Dialog for managing folder alerts.
*/
class ManageFolderAlertsDialog : public ManageAlertsDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param folderId Identifier of the folder.
    */
    ManageFolderAlertsDialog( int folderId );

    /**
    * Destructor.
    */
    ~ManageFolderAlertsDialog();

protected: // overrides
    void addAlert();
    void addPublicAlert();

    void updateActions();

private:
    int m_folderId;
};

/**
* Dialog for managing global alerts.
*/
class ManageGlobalAlertsDialog : public ManageAlertsDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param typeId Identifier of the issue type.
    */
    ManageGlobalAlertsDialog( int typeId );

    /**
    * Destructor.
    */
    ~ManageGlobalAlertsDialog();

protected: // overrides
    void addAlert();
    void addPublicAlert();

    void updateActions();

private:
    int m_typeId;
};


#endif
