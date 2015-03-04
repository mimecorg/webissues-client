/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2015 WebIssues Team
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

#ifndef SSLDIALOGS_H
#define SSLDIALOGS_H

#include "dialogs/informationdialog.h"
#include "dialogs/commanddialog.h"

#include <QSslError>

#if !defined( QT_NO_OPENSSL )

class PropertyPanel;

class QListWidget;
class QCheckBox;

/**
* Dialog for showing SSL connection certificates.
*/
class SslCertificatesDialog : public InformationDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent The parent widget.
    */
    SslCertificatesDialog( QWidget* parent );

    /**
    * Destructor.
    */
    ~SslCertificatesDialog();

public:
    /**
    * Set the list of certificates to display.
    */
    void setCertificates( const QList<QSslCertificate>& certificates );

private slots:
    void showCertificate( int index );

private:
    QString formatDate( const QDateTime& dateTime );
    QString formatBinary( const QByteArray& binary );

private:
    QList<QSslCertificate> m_certificates;

    QListWidget* m_list;

    PropertyPanel* m_subjectPanel;
    PropertyPanel* m_issuerPanel;
    PropertyPanel* m_detailsPanel;
};

/**
* Dialog for showing SSL connection errors.
*/
class SslErrorsDialog : public CommandDialog
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent The parent widget.
    */
    SslErrorsDialog( QWidget* parent );

    /**
    * Destructor.
    */
    ~SslErrorsDialog();

public:
    /**
    * Set the list of errors to display.
    */
    void setErrors( const QList<QSslError>& errors );

    /**
    * Set the list of associated certificates.
    */
    void setCertificates( const QList<QSslCertificate>& certificates );

    /**
    * Return @c true if certificate was accepted permanently.
    */
    bool acceptPermanently() const;

private slots:
    void showCertificates();

private:
    QListWidget* m_list;

    QPushButton* m_certificatesButton;

    QCheckBox* m_acceptCheckBox;

    QList<QSslCertificate> m_certificates;
};

#endif // !defined( QT_NO_OPENSSL )

#endif
