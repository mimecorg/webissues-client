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

#include "issuedetailsgenerator.h"

#include "data/datamanager.h"
#include "data/entities.h"
#include "utils/datetimehelper.h"
#include "utils/viewsettingshelper.h"
#include "utils/markupprocessor.h"
#include "utils/htmlwriter.h"
#include "utils/formatter.h"

#include <QtAlgorithms>

IssueDetailsGenerator::IssueDetailsGenerator() :
    m_issueId( 0 ),
    m_history( NoHistory ),
    m_isOwner( false ),
    m_isAdmin( false ),
    m_commentsCount( 0 ),
    m_filesCount( 0 )
{
}

IssueDetailsGenerator::~IssueDetailsGenerator()
{
}

void IssueDetailsGenerator::setIssue( int issueId, History history )
{
    m_issueId = issueId;
    m_history = history;

    m_isOwner = IssueEntity::isOwner( issueId );
    m_isAdmin = IssueEntity::isAdmin( issueId );
}

void IssueDetailsGenerator::write( HtmlWriter* writer, HtmlText::Flags flags /*= 0*/ )
{
    IssueEntity issue = IssueEntity::find( m_issueId );

    if ( issue.isValid() ) {
        writer->writeBlock( issue.name(), HtmlWriter::Header2Block );

        writer->createLayout();

        QList<ValueEntity> values;
        if ( dataManager->setting( "hide_empty_values" ) == "1" )
            values = issue.nonEmptyValues();
        else
            values = issue.values();

        writer->beginCell( HtmlWriter::TopPane, values.isEmpty() ? 2 : 1 );
        writeProperties( writer, issue );

        if ( !values.isEmpty() ) {
            writer->beginCell( HtmlWriter::TopPane );
            writeAttributes( writer, values, flags );
        }

        if ( m_history != NoHistory ) {
            DescriptionEntity description = issue.description();

            if ( description.isValid() ) {
                writer->appendLayoutRow();
                writer->beginCell( HtmlWriter::BottomPane, 2 );

                writer->writeBlock( descriptionLinks( description, flags ), HtmlWriter::FloatBlock );
                writer->writeBlock( tr( "Description" ), HtmlWriter::Header3Block );
                writer->writeBlock( descriptionText( description, flags ), HtmlWriter::CommentBlock );
            }

            writer->appendLayoutRow();
            writer->beginCell( HtmlWriter::BottomPane, 2 );

            if ( !flags.testFlag( HtmlText::NoInternalLinks ) )
                writer->writeBlock( historyLinks( flags ), HtmlWriter::FloatBlock );

            writer->writeBlock( tr( "Issue History" ), HtmlWriter::Header3Block );

            writeHistory( writer, issue, flags );
        }

        writer->endLayout();
    }
}

void IssueDetailsGenerator::writeProperties( HtmlWriter* writer, const IssueEntity& issue )
{
    QStringList headers;
    QList<HtmlText> items;

    headers.append( tr( "ID:" ) );
    items.append( QString( "#%1" ).arg( issue.id() ) );

    FolderEntity folder = issue.folder();

    headers.append( tr( "Type:" ) );
    items.append( folder.type().name() );
    headers.append( tr( "Location:" ) );
    items.append( folder.project().name() + QString::fromUtf8( " — " ) + folder.name() );

    Formatter formatter;

    headers.append( tr( "Created:" ) );
    items.append( QString::fromUtf8( "%1 — %2" ).arg( formatter.formatDateTime( issue.createdDate(), true ), issue.createdUser() ) );
    headers.append( tr( "Last Modified:" ) );
    items.append( QString::fromUtf8( "%1 — %2" ).arg( formatter.formatDateTime( issue.modifiedDate(), true ), issue.modifiedUser() ) );

    writer->writeInfoList( headers, items, false );
}

void IssueDetailsGenerator::writeAttributes( HtmlWriter* writer, const QList<ValueEntity>& values, HtmlText::Flags flags )
{
    QStringList headers;
    QList<HtmlText> items;

    Formatter formatter;

    for ( int i = 0; i < values.count(); i++ ) {
        const ValueEntity& value = values.at( i );
        headers.append( value.name() + tr( ":" ) );
        QString formattedValue = formatter.convertAttributeValue( value.definition(), value.value(), true );
        items.append( HtmlText::parse( formattedValue, flags ) );
    }

    writer->writeInfoList( headers, items, true );
}

void IssueDetailsGenerator::writeHistory( HtmlWriter* writer, const IssueEntity& issue, HtmlText::Flags flags )
{
    Qt::SortOrder order = Qt::AscendingOrder;

    if ( dataManager->preferenceOrSetting( "history_order" ) == "desc" )
        order = Qt::DescendingOrder;

    QList<ChangeEntity> changes;
    if ( m_history == AllHistory )
        changes = issue.changes( order );
    else if ( m_history == OnlyComments )
        changes = issue.comments( order );
    else if ( m_history == OnlyFiles )
        changes = issue.files( order );
    else if ( m_history == CommentsAndFiles )
        changes = issue.commentsAndFiles( order );

    QList<HtmlText> list;

    int lastUserId = 0;
    QDateTime lastDate;

    for ( int i = 0; i < changes.count(); i++ ) {
        const ChangeEntity& change = changes.at( i );

        if ( change.type() == ValueChanged && change.attributeId() == 0 )
            continue;

        if ( change.type() <= ValueChanged && list.count() > 0 ) {
            if ( change.createdUserId() == lastUserId && lastDate.secsTo( change.createdDate() ) < 180 ) {
                list.append( formatChange( change, flags ) );
                continue;
            }
        }

        if ( list.count() > 0 ) {
            writer->writeBulletList( list );
            writer->endHistoryItem();
            list.clear();
        }

        switch ( change.type() ) {
            case IssueCreated:
            case IssueRenamed:
            case ValueChanged:
                lastUserId = change.createdUserId();
                lastDate = change.createdDate();
                writer->beginHistoryItem();
                writer->writeBlock( formatStamp( change ), HtmlWriter::Header4Block );
                list.append( formatChange( change, flags ) );
                break;

            case CommentAdded:
                writer->beginHistoryItem();
                writer->writeBlock( changeLinks( change, flags ), HtmlWriter::HistoryInfoBlock );
                writer->writeBlock( formatStamp( change ), HtmlWriter::Header4Block );
                if ( change.comment().format() == TextWithMarkup )
                    writer->writeBlock( MarkupProcessor::parse( change.comment().text(), flags ), HtmlWriter::CommentBlock );
                else
                    writer->writeBlock( HtmlText::parse( change.comment().text(), flags ), HtmlWriter::CommentBlock );
                writer->endHistoryItem();
                m_commentsCount++;
                break;

            case FileAdded:
                writer->beginHistoryItem();
                writer->writeBlock( changeLinks( change, flags ), HtmlWriter::HistoryInfoBlock );
                writer->writeBlock( formatStamp( change ), HtmlWriter::Header4Block );
                writer->writeBlock( formatFile( change.file(), flags ), HtmlWriter::AttachmentBlock );
                writer->endHistoryItem();
                m_filesCount++;
                break;

            case IssueMoved:
                writer->beginHistoryItem();
                writer->writeBlock( formatStamp( change ), HtmlWriter::Header4Block );
                writer->writeBulletList( QList<HtmlText>() << formatChange( change, flags ) );
                writer->endHistoryItem();
                break;
        }
    }

    if ( list.count() > 0 ) {
        writer->writeBulletList( list );
        writer->endHistoryItem();
    }

    if ( changes.count() == 0 ) {
        if ( m_history == OnlyComments )
            writer->writeBlock( tr( "There are no comments." ), HtmlWriter::NoItemsBlock );
        else if ( m_history == OnlyFiles )
            writer->writeBlock( tr( "There are no attachments." ), HtmlWriter::NoItemsBlock );
        else if ( m_history == CommentsAndFiles )
            writer->writeBlock( tr( "There are no comments or attachments." ), HtmlWriter::NoItemsBlock );
    }
}

QString IssueDetailsGenerator::formatStamp( const ChangeEntity& change )
{
    Formatter formatter;
    return QString::fromUtf8( "%1 — %2" ).arg( formatter.formatDateTime( change.createdDate(), true ), change.createdUser() );
}

HtmlText IssueDetailsGenerator::formatChange( const ChangeEntity& change, HtmlText::Flags flags )
{
    HtmlText result( flags );

    switch ( change.type() ) {
        case IssueCreated:
            result.appendText( tr( "Name:" ) );
            result.appendText( " \"" );
            result.appendParsed( change.newValue() );
            result.appendText( "\"" );
            break;

        case IssueRenamed:
            result.appendText( tr( "Name:" ) );
            result.appendText( " \"" );
            result.appendParsed( change.oldValue() );
            result.appendText( QString::fromUtf8( "\" → \"" ) );
            result.appendParsed( change.newValue() );
            result.appendText( "\"" );
            break;

        case ValueChanged: {
            result.appendText( change.name() );
            result.appendText( tr( ":" ) );
            result.appendText( " " );

            Formatter formatter;
            DefinitionInfo info = change.definition();
            QString oldValue = formatter.convertAttributeValue( info, change.oldValue(), false );
            QString newValue = formatter.convertAttributeValue( info, change.newValue(), false );

            if ( oldValue.isEmpty() ) {
                result.appendText( tr( "empty" ) );
            } else {
                result.appendText( "\"" );
                result.appendParsed( oldValue );
                result.appendText( "\"" );
            }

            result.appendText( QString::fromUtf8( " → " ) );

            if ( newValue.isEmpty() ) {
                result.appendText( tr( "empty" ) );
            } else {
                result.appendText( "\"" );
                result.appendParsed( newValue );
                result.appendText( "\"" );
            }
            break;
        }

        case IssueMoved:
            result.appendText( tr( "Issue moved from" ) );
            result.appendText( " " );

            if ( change.fromFolder().isEmpty() ) {
                result.appendText( tr( "Unknown Folder" ) );
            } else {
                result.appendText( "\"" );
                result.appendText( change.fromFolder() );
                result.appendText( "\"" );
            }

            result.appendText( " " );
            result.appendText( tr( "to" ) );
            result.appendText( " " );

            if ( change.toFolder().isEmpty() ) {
                result.appendText( tr( "Unknown Folder" ) );
            } else {
                result.appendText( "\"" );
                result.appendText( change.toFolder() );
                result.appendText( "\"" );
            }
            break;

        default:
            break;
    }

    return result;
}

HtmlText IssueDetailsGenerator::formatFile( const FileEntity& file, HtmlText::Flags flags )
{
    HtmlText result( flags );

    result.appendLink( file.name(), QString( "attachment://%1" ).arg( file.id() ) );
    result.appendText( " (" );

    Formatter formatter;
    result.appendText( formatter.formatSize( file.size() ) );

    result.appendText( ")" );

    if ( !file.description().isEmpty() ) {
        result.appendText( QString::fromUtf8( " — " ) );
        result.appendParsed( file.description() );
    }

    return result;
}

HtmlText IssueDetailsGenerator::historyLinks( HtmlText::Flags flags )
{
    HtmlText result( flags );

    for ( int i = AllHistory; i <= CommentsAndFiles; i++ ) {
        if ( i != AllHistory )
            result.appendText( " | " );

        QString text;
        switch ( i ) {
            case AllHistory:
                text = tr( "All History" );
                break;
            case OnlyComments:
                text = tr( "Only Comments" );
                break;
            case OnlyFiles:
                text = tr( "Only Attachments" );
                break;
            case CommentsAndFiles:
                text = tr( "Comments & Attachments" );
                break;
        }

        if ( m_history == i )
            result.appendText( text );
        else
            result.appendLink( text, QString( "command://filter/%1" ).arg( i ) );
    }

    return result;
}

HtmlText IssueDetailsGenerator::descriptionLinks( const DescriptionEntity& description, HtmlText::Flags flags )
{
    HtmlText result( flags );

    Formatter formatter;
    result.appendText( tr( "Last Edited:" ) );
    result.appendText( QString::fromUtf8( " %1 — %2" ).arg( formatter.formatDateTime( description.modifiedDate(), true ), description.modifiedUser() ) );

    if ( !flags.testFlag( HtmlText::NoInternalLinks ) ) {
        result.appendText( " | " );
        result.appendImageAndTextLink( "comment-reply", tr( "Reply" ), "command://reply-description/" );
        if ( m_isOwner || m_isAdmin ) {
            result.appendText( " | " );
            result.appendImageAndTextLink( "edit-modify", tr( "Edit" ), "command://edit-description/" );
            result.appendText( " | " );
            result.appendImageAndTextLink( "edit-delete", tr( "Delete" ), "command://delete-description/" );
        }
    }

    return result;
}

HtmlText IssueDetailsGenerator::changeLinks( const ChangeEntity& change, HtmlText::Flags flags )
{
    HtmlText result( flags );

    if ( change.stampId() != change.id() ) {
        Formatter formatter;
        result.appendText( tr( "Last Edited:" ) );
        result.appendText( QString::fromUtf8( " %1 — %2" ).arg( formatter.formatDateTime( change.modifiedDate(), true ), change.modifiedUser() ) );
        result.appendText( " | " );
    }

    result.createAnchor( QString( "item%1" ).arg( change.id() ) );
    if ( flags.testFlag( HtmlText::NoInternalLinks ) ) {
        if ( change.type() == CommentAdded )
            result.appendText( tr( "Comment #%1" ).arg( change.id() ) );
        else
            result.appendText( tr( "Attachment #%1" ).arg( change.id() ) );
    } else {
        if ( change.type() == CommentAdded )
            result.appendImageAndText( "comment", tr( "Comment #%1" ).arg( change.id() ) );
        else
            result.appendImageAndText( "file-attach", tr( "Attachment #%1" ).arg( change.id() ) );
    }
    result.endAnchor();

    if ( !flags.testFlag( HtmlText::NoInternalLinks ) ) {
        if ( change.type() == CommentAdded ) {
            result.appendText( " | " );
            result.appendImageAndTextLink( "comment-reply", tr( "Reply" ), QString( "command://reply-comment/%1" ).arg( change.id() ) );
        }

        if ( m_isAdmin || change.createdUserId() == dataManager->currentUserId() ) {
            result.appendText( " | " );
            if ( change.type() == CommentAdded )
                result.appendImageAndTextLink( "edit-modify", tr( "Edit" ), QString( "command://edit-comment/%1" ).arg( change.id() ) );
            else
                result.appendImageAndTextLink( "edit-modify", tr( "Edit" ), QString( "command://edit-file/%1" ).arg( change.id() ) );

            result.appendText( " | " );
            if ( change.type() == CommentAdded )
                result.appendImageAndTextLink( "edit-delete", tr( "Delete" ), QString( "command://delete-comment/%1" ).arg( change.id() ) );
            else
                result.appendImageAndTextLink( "edit-delete", tr( "Delete" ), QString( "command://delete-file/%1" ).arg( change.id() ) );
        }
    }

    return result;
}

HtmlText IssueDetailsGenerator::descriptionText( const DescriptionEntity& description, HtmlText::Flags flags )
{
    if ( description.format() == TextWithMarkup )
        return MarkupProcessor::parse( description.text(), flags );
    else
        return HtmlText::parse( description.text(), flags );
}

HtmlText IssueDetailsGenerator::commentText( const CommentEntity& comment, HtmlText::Flags flags )
{
    if ( comment.format() == TextWithMarkup )
        return MarkupProcessor::parse( comment.text(), flags );
    else
        return HtmlText::parse( comment.text(), flags );
}
