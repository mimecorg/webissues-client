/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2012 WebIssues Team
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

#ifndef ISSUEDETAILSGENERATOR_H
#define ISSUEDETAILSGENERATOR_H

#include "utils/textwithlinks.h"

#include <QObject>
#include <QList>

class TextWriter;
class IssueEntity;
class ChangeEntity;
class FileEntity;

/**
* Class providing issue details to the TextWriter.
*
* This class extracts the details and history of an issue and outputs it
* to the TextWriter.
*/
class IssueDetailsGenerator : public QObject
{
    Q_OBJECT
public:
    /**
    * Default constructor.
    */
    IssueDetailsGenerator();

    /**
    * Destructor.
    */
    ~IssueDetailsGenerator();

public:
    enum History
    {
        NoHistory,
        AllHistory,
        OnlyComments,
        OnlyFiles,
        CommentsAndFiles,
    };

public:
    /**
    * Set the issue to provide details for.
    * @param issueId Idenitifier of the issue.
    * @param history Type of issue history to be included.
    */
    void setIssue( int issueId, History history );

    /**
    * Output the issue details to the writer.
    * @param writer The text document writer to output the details to.
    * @param flags Optional flags affecting extracting of links.
    */
    void write( TextWriter* writer, TextWithLinks::Flags flags = 0 );

    int commentsCount() const { return m_commentsCount; }

    int filesCount() const { return m_filesCount; }

private:
    void writeProperties( TextWriter* writer, const IssueEntity& issue );
    void writeAttributes( TextWriter* writer, const IssueEntity& issue, TextWithLinks::Flags flags );
    void writeHistory( TextWriter* writer, const IssueEntity& issue, TextWithLinks::Flags flags );

    QString formatStamp( const ChangeEntity& change );

    TextWithLinks formatChange( const ChangeEntity& change, TextWithLinks::Flags flags );
    TextWithLinks formatFile( const FileEntity& file, TextWithLinks::Flags flags );

    TextWithLinks historyLinks( TextWithLinks::Flags flags );

    bool checkChangeLinks( const ChangeEntity& change );
    TextWithLinks changeLinks( const ChangeEntity& change, TextWithLinks::Flags flags );

private:
    int m_issueId;
    History m_history;

    bool m_isAdmin;

    int m_commentsCount;
    int m_filesCount;
};

#endif
