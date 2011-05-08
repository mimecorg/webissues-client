/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2011 WebIssues Team
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

#ifndef COMMANDDIALOG_H
#define COMMANDDIALOG_H

#include <QDialog>

class AbstractBatch;
class ElidedLabel;

class QLabel;
class QBoxLayout;
class QProgressBar;
class QDialogButtonBox;
class QLayoutItem;

/**
* Base class for dialogs for executing commands.
*
* Inherited classes should create widgets using mainWidget() as their parent,
* put them in a layout and pass that layout to the setContentLayout() method.
* They should also initialize the prompt text before setting the layout.
*
* To execute commands, create a batch and pass it to the executeBatch() method.
* If the batch is executed successfully, the dialog is closed automatically.
*
* While a command batch is being executed, all widgets are automatically disabled
* and the Cancel button changes into an Abort button allowing the user to abort
* the batch.
*
* The command dialog shows a status message and icon in the bottom. It can also
* display a progress bar with status text.
*/
class CommandDialog : public QDialog
{
    Q_OBJECT
public:
    /**
    * Conctructor.
    * @param parent The parent widget.
    */
    CommandDialog( QWidget* parent );

    /**
    * Destructor.
    */
    ~CommandDialog();

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
    * Add a progress panel beneath the main content.
    * @param total The total number of steps in the progress bar.
    * @param text The initial status text.
    */
    void createProgressPanel( int total, const QString& text );

    /**
    * Set the current progress.
    * @param done The number of completed steps in the progress bar.
    * @param text The status text.
    */
    void setProgress( int done, const QString& text );

    /**
    * Show a message with an information icon.
    * @param text The status message.
    */
    void showInfo( const QString& text );

    /**
    * Show a message with a warning icon.
    * @param text The warning text.
    */
    void showWarning( const QString& text );

    /**
    * Show a warning message based on the error code.
    * @param code The error code.
    */
    void showWarning( int code );

    /**
    * Show a message with an error icon.
    * @param text The warning text.
    */
    void showError( const QString& text );

    /**
    * Show Close button instead of OK and Cancel.
    */
    void showCloseButton();

    /**
    * Validate all input controls in the dialog.
    * @return @c true if all controls have valid input.
    */
    bool validate();

    /**
    * Start executing the batch of commands.
    * @param batch The batch to execute.
    * @param text Optional message text.
    */
    void executeBatch( AbstractBatch* batch, const QString& text = QString() );

    /**
    * Return the dialog's button box.
    */
    QDialogButtonBox* buttonBox() const { return m_buttonBox; }

protected:
    /**
    * Called when the batch is finished successfully.
    * @param batch The batch that has been executed.
    * @return @c true to close the dialog, @c false to cancel closing.
    */
    virtual bool batchSuccessful( AbstractBatch* batch );

    /**
    * Called when the batch failed.
    * @param batch The batch that has been executed.
    * @return @c true to restore original state, @c false to keep disabled.
    */
    virtual bool batchFailed( AbstractBatch* batch );

protected: // overrides
    void reject();

private slots:
    void batchCompleted( bool successful );

private:
    void setWidgetsEnabled( QLayoutItem* item, bool enabled );

private:
    QWidget* m_promptWidget;
    QBoxLayout* m_promptLayout;

    QLabel* m_promptLabel;
    QLabel* m_promptPixmap;

    QBoxLayout* m_mainLayout;

    QLayout* m_contentLayout;
    bool m_fixed;

    QLabel* m_statusPixmap;
    ElidedLabel* m_statusLabel;

    bool m_statusSet;

    QDialogButtonBox* m_buttonBox;

    QProgressBar* m_progressBar;
    QLabel* m_progressLabel;

    QString m_progressText;

    AbstractBatch* m_batch;
};

#endif
