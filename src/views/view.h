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

#ifndef VIEW_H
#define VIEW_H

#include "data/datamanager.h"
#include "xmlui/client.h"

#include <QWidget>

class UpdateEvent;
class AbstractBatch;

/**
* A view which can be embedded in a window.
*
* This is a base class for creating views which consist of a widget
* of any type and a set of actions which can be plugged into the
* container window's menus and toolbars.
*
* Inherited classes should create the main widget, create actions
* and load the XMLGUI configuration file. The initialUpdate() method
* should be overriden to perform initialization after embedding
* the view in the container window.
*
* A view may have an identifier which is used to distinguish units
* of data the view is associated with. The identifier is set before
* initialUpdate() and is not changed later.
*
* The view class provides a few properties and signals to communicate
* with the container. It provides a caption, size hint and setting
* status messages.
*
* The view automatically registers itself to receive update events
* from the data manager. It also handles executing update batches.
*
* The ViewManager can be used to create and activate views.
*/
class View : public QObject, public XmlUi::Client
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent The parent object.
    */
    View( QObject* parent );

    /**
    * Destructor.
    */
    ~View();

public:
    /**
    * Return the main widget of the view.
    */
    QWidget* mainWidget() const { return m_mainWidget; }

    /**
    * Return the default size of the view.
    */
    QSize viewerSizeHint() const { return m_sizeHint; }

    /**
    * Return the key name for the remembered size of the view.
    */
    QString viewerSizeKey() const;

    /**
    * Return the caption of the view.
    */
    const QString& caption() const { return m_caption; }

    /**
    * Return the identifier of the data unit.
    */
    int id() const { return m_id; }

    /**
    * Set the identifier of the data unit.
    */
    void setId( int id ) { m_id = id; }

    /**
    * Return the state of the view.
    */
    bool isEnabled() const { return m_enabled; }

    /**
    * Return the access mode of the view.
    */
    Access access() const { return m_access; }

    /**
    * Return the status pixmap.
    */
    QPixmap statusPixmap() const { return m_statusPixmap; }
    /**
    * Return the status text.
    */
    QString statusText() const { return m_statusText; }

    /**
    * Return the summary pixmap.
    */
    QPixmap summaryPixmap() const { return m_summaryPixmap; }
    /**
    * Return the summary text.
    */
    QString summaryText() const { return m_summaryText; }

public:
    /**
    * Update after embedding in the container.
    * Override this method to perform initialization of the view.
    */
    virtual void initialUpdate();

    /**
    * Switch the view to enabled state.
    */
    virtual void enableView();

    /**
    * Switch the view to disabled state.
    */
    virtual void disableView();

    /**
    * Switch between normal and administrator access.
    * Override this method to show or hide administrative actions.
    */
    virtual void updateAccess( Access access );

    /**
    * Ask the view if it can be closed.
    * Override this method to ask the user about unsaved changes.
    * The default implementation returns @c true.
    * @return @c true if the view can be closed, @c false otherwise.
    */
    virtual bool queryClose();

signals:
    /**
    * Emitted when the caption was changed.
    * @param text The new caption.
    */
    void captionChanged( const QString& text );

    /**
    * Emitted when the view was enabled or disabled.
    * @param enabled The state of the view.
    */
    void enabledChanged( bool enabled );

    /**
    * Emitted when the status was changed.
    */
    void statusChanged( const QPixmap& pixmap, const QString& text, int icon = 0 );

    /**
    * Emitted when the summary was changed.
    */
    void summaryChanged( const QPixmap& pixmap, const QString& text );

protected:
    /**
    * Handle the UpdateEvent from the data manager.
    * @param e The update event.
    */
    virtual void updateEvent( UpdateEvent* e );

    /**
    * Handle an error when updating the view.
    */
    virtual void updateFailed();

protected:
    /**
    * Set the main widget of the view.
    */
    void setMainWidget( QWidget* widget );

    /**
    * Set the default size of the view.
    */
    void setViewerSizeHint( const QSize& size );

    /**
    * Set the key name for the remembered size of the view.
    */
    void setViewerSizeKey( const QString& key );

    /**
    * Set the dynamic access mode of the view
    */
    void setAccess( Access access, bool initial = false );

    /**
    * Start executing the update batch.
    * @param batch The batch to execute.
    */
    void executeUpdate( AbstractBatch* batch );

    /**
    * Return @c true if an update batch is being executed.
    */
    bool isUpdating() const { return m_updating; }

    /**
    * Display an information status message.
    * @param text The status message.
    */
    void showInfo( const QString& text );

    /**
    * Display a warning status message.
    * @param text The status message.
    */
    void showWarning( const QString& text );

    /**
    * Display an error status message.
    * @param text The status message.
    */
    void showError( const QString& text );

    /**
    * Display a busy status message.
    * @param text The status message.
    */
    void showBusy( const QString& text );

    /**
    * Display a summary message.
    * @param pixmap The summary pixmap.
    * @param text The summary message.
    */
    void showSummary( const QPixmap& pixmap, const QString& text );

protected: // overrides
    void customEvent( QEvent* e );

protected slots:
    /**
    * Set the caption of the view.
    */
    void setCaption( const QString& text );

private slots:
    void updateCompleted( bool successful );

private:
    QWidget* m_mainWidget;

    QSize m_sizeHint;
    QString m_sizeKey;

    QString m_caption;

    int m_id;

    Access m_access;
    bool m_enabled;

    bool m_updating;

    QPixmap m_statusPixmap;
    QString m_statusText;

    QPixmap m_summaryPixmap;
    QString m_summaryText;
};

#endif
