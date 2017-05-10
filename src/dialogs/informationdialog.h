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

#ifndef INFORMATIONDIALOG_H
#define INFORMATIONDIALOG_H

#include <QDialog>

class QLabel;
class QBoxLayout;
class QDialogButtonBox;

/**
* Base class for dialogs for displaying information.
*
* Inherited classes should create widgets using mainWidget() as their parent,
* put them in a layout and pass that layout to the setContentLayout() method.
* They should also initialize the prompt text before setting the layout.
*/
class InformationDialog : public QDialog
{
    Q_OBJECT
public:
    /**
    * Conctructor.
    * @param parent The parent widget.
    * @param flags Optional window flags.
    */
    InformationDialog( QWidget* parent, Qt::WindowFlags flags = 0 );

    /**
    * Destructor.
    */
    ~InformationDialog();

public:
    /**
    * Return the key name for the remembered size of the dialog.
    */
    QString dialogSizeKey() const;

protected:
    /**
    * Set the prompt text.
    * The prompt text is displayed in the top part of the dialog. HTML tags
    * can be used.
    */
    void setPrompt( const QString& text );

    /**
    * Set the prompt pixmap.
    */
    void setPromptPixmap( const QPixmap& pixmap );

    /**
    * Initialize the dialog content.
    * @param layout The layout to insert into the dialog.
    * @param fixed If @c true, the dialog cannot be resized vertically.
    */
    void setContentLayout( QLayout* layout, bool fixed );

    /**
    * Set the key name for the remembered size of the dialog.
    */
    void setDialogSizeKey( const QString& key );

    /**
    * Return the dialog's button box.
    */
    QDialogButtonBox* buttonBox() const { return m_buttonBox; }

private:
    QWidget* m_promptWidget;
    QBoxLayout* m_promptLayout;

    QLabel* m_promptLabel;
    QLabel* m_promptPixmap;

    QBoxLayout* m_mainLayout;

    QDialogButtonBox* m_buttonBox;

    QString m_sizeKey;
};

#endif
