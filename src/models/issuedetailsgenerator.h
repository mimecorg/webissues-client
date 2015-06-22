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

#ifndef ISSUEDETAILSGENERATOR_H
#define ISSUEDETAILSGENERATOR_H

#include "utils/htmltext.h"

#include <QObject>
#include <QList>

class HtmlWriter;
class IssueEntity;
class ChangeEntity;
class FileEntity;
class ValueEntity;
class DescriptionEntity;
class CommentEntity;

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
    * @param description @c true if the description should be included.
    * @param history Type of issue history to be included.
    */
    void setIssue( int issueId, bool description, History history );

    /**
    * Output the issue details to the writer.
    * @param writer The text document writer to output the details to.
    * @param flags Optional flags affecting extracting of links.
    */
    void write( HtmlWriter* writer, HtmlText::Flags flags = 0 );

    int commentsCount() const { return m_commentsCount; }

    int filesCount() const { return m_filesCount; }

private:
    void writeProperties( HtmlWriter* writer, const IssueEntity& issue );
    void writeAttributes( HtmlWriter* writer, const QList<ValueEntity>& values, HtmlText::Flags flags );
    void writeHistory( HtmlWriter* writer, const IssueEntity& issue, HtmlText::Flags flags );

    QString formatStamp( const ChangeEntity& change );

    HtmlText formatChange( const ChangeEntity& change, HtmlText::Flags flags );
    HtmlText formatFile( const FileEntity& file, HtmlText::Flags flags );

    HtmlText historyLinks( HtmlText::Flags flags );

    HtmlText descriptionLinks( HtmlText::Flags flags );
    HtmlText changeLinks( const ChangeEntity& change, HtmlText::Flags flags );

    HtmlText descriptionEdited( const IssueEntity& issue, const DescriptionEntity& description, HtmlText::Flags flags );
    HtmlText changeEdited( const ChangeEntity& change, HtmlText::Flags flags );

    HtmlText descriptionText( const DescriptionEntity& description, HtmlText::Flags flags );
    HtmlText commentText( const CommentEntity& comment, HtmlText::Flags flags );

private:
    int m_issueId;
    bool m_description;
    History m_history;

    bool m_isOwner;
    bool m_isAdmin;

    int m_commentsCount;
    int m_filesCount;
};

#endif
