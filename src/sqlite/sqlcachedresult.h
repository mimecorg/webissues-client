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
*
* This file is based on the QtSql module of the Qt Toolkit
* Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
**************************************************************************/

#ifndef SQLCACHEDRESULT_H
#define SQLCACHEDRESULT_H

#include <QtSql/qsqlresult.h>

class QVariant;
template <typename T> class QVector;

class SqlCachedResultPrivate;

class SqlCachedResult: public QSqlResult
{
public:
    virtual ~SqlCachedResult();

    typedef QVector<QVariant> ValueCache;

protected:
    SqlCachedResult(const QSqlDriver * db);

    void init(int colCount);
    void cleanup();
    void clearValues();

    virtual bool gotoNext(ValueCache &values, int index) = 0;

    QVariant data(int i);
    bool isNull(int i);
    bool fetch(int i);
    bool fetchNext();
    bool fetchPrevious();
    bool fetchFirst();
    bool fetchLast();

    int colCount() const;
    ValueCache &cache();

    void virtual_hook(int id, void *data);
private:
    bool cacheNext();
    SqlCachedResultPrivate *d;
};

#endif
