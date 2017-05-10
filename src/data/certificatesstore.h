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

#ifndef CERTIFICATESSTORE_H
#define CERTIFICATESSTORE_H

#include <QObject>
#include <QSslCertificate>

#if !defined( QT_NO_OPENSSL )

/**
* Class for storing a list of SSL certificates.
*/
class CertificatesStore : public QObject
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param path The path of the file containing certificates.
    * @param parent The parent object.
    */
    CertificatesStore( const QString& path, QObject* parent );

    /**
    * Destructor.
    */
    ~CertificatesStore();

public:
    /**
    * Return @c true if the store contains the given certificate.
    */
    bool containsCertificate( const QSslCertificate& certificate );

    /**
    * Add the certificate to the store.
    */
    void addCertificate( const QSslCertificate& certificate );

    /**
    * Load bookmarks from the file.
    */
    void load();

    /**
    * Save bookmarks to the file.
    */
    void save();

private:
    QString m_path;

    QList<QSslCertificate> m_certificates;
};

#endif // !defined( QT_NO_OPENSSL )

#endif
