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

#include "issuedetailsgenerator.h"

#include "data/datamanager.h"
#include "data/datarows.h"
#include "utils/datetimehelper.h"
#include "utils/tablemodelshelper.h"
#include "utils/viewsettingshelper.h"
#include "utils/textwriter.h"
#include "utils/formatter.h"

#include <QtAlgorithms>

IssueDetailsGenerator::IssueDetailsGenerator() :
    m_issueId( 0 ),
    m_history( NoHistory )
{
}

IssueDetailsGenerator::~IssueDetailsGenerator()
{
}

void IssueDetailsGenerator::setIssue( int issueId, History history )
{
    m_issueId = issueId;
    m_history = history;

    m_items.clear();

    if ( history != NoHistory ) {
        RDB::ForeignConstIterator<ChangeRow> it( dataManager->changes()->parentIndex(), issueId );
        while ( it.next() ) {
            const ChangeRow* change = it.get();
            if ( history == AllHistory || history == OnlyComments && change->changeType() == CommentAdded
                 || history == OnlyFiles && change->changeType() == FileAdded )
                m_items.append( change->changeId() );
        }

        qSort( m_items );
    }
}

void IssueDetailsGenerator::write( TextWriter* writer, TextWithLinks::Flags flags /*= 0*/ )
{
    const IssueRow* issue = dataManager->issues()->find( m_issueId );
    if ( issue ) {
        writer->writeBlock( issue->name(), TextWriter::Header1Block );

        writer->createLayout( 1, 4 );

        writer->mergeLayoutCells( 0, 0, 1, 2 );
        writer->gotoLayoutCell( 0, 0, TextWriter::NormalCell );
        writer->writeBlock( tr( "Properties" ), TextWriter::Header2Block );
        writeProperties( writer, issue );

        writer->mergeLayoutCells( 0, 2, 1, 2 );
        writer->gotoLayoutCell( 0, 2, TextWriter::NormalCell );
        writer->writeBlock( tr( "Attributes" ), TextWriter::Header2Block );
        writeAttributes( writer, issue, flags );

        if ( m_history != NoHistory ) {
            writer->appendLayoutRows( 1 );

            writer->gotoLayoutCell( 1, 0, TextWriter::NormalCell );
            writer->writeBlock( tr( "Issue History" ), TextWriter::Header2Block );

            if ( !flags.testFlag( TextWithLinks::NoInternalLinks ) ) {
                writer->mergeLayoutCells( 1, 1, 1, 3 );
                writer->gotoLayoutCell( 1, 1, TextWriter::LinksCell );
                writer->writeBlock( historyLinks( flags ), TextWriter::LinksBlock );
            }

            writeHistory( writer, flags );
        }

        writer->endLayout();
    }
}

void IssueDetailsGenerator::writeProperties( TextWriter* writer, const IssueRow* issue )
{
    const FolderRow* folder = dataManager->folders()->find( issue->folderId() );
    const TypeRow* type = folder ? dataManager->types()->find( folder->typeId() ) : NULL;
    const ProjectRow* project = folder ? dataManager->projects()->find( folder->projectId() ) : NULL;

    QStringList headers;
    QList<TextWithLinks> items;

    headers.append( tr( "ID:" ) );
    items.append( TableModelsHelper::formatId( issue->issueId() ) );

    if ( type ) {
        headers.append( tr( "Type:" ) );
        items.append( type->name() );
    }
    if ( project ) {
        headers.append( tr( "Project:" ) );
        items.append( project->name() );
    }
    if ( folder ) {
        headers.append( tr( "Folder:" ) );
        items.append( folder->name() );
    }

    Formatter formatter;

    headers.append( tr( "Created date:" ) );
    items.append( formatter.formatDateTime( issue->createdDate(), true ) );
    headers.append( tr( "Created by:" ) );
    items.append( TableModelsHelper::userName( issue->createdUser() ) );
    headers.append( tr( "Modified date:" ) );
    items.append( formatter.formatDateTime( issue->modifiedDate(), true ) );
    headers.append( tr( "Modified by:" ) );
    items.append( TableModelsHelper::userName( issue->modifiedUser() ) );

    writer->writeInfoList( headers, items );
}

void IssueDetailsGenerator::writeAttributes( TextWriter* writer, const IssueRow* issue, TextWithLinks::Flags flags )
{
    const FolderRow* folder = dataManager->folders()->find( issue->folderId() );
    int typeId = folder ? folder->typeId() : 0;

    QList<int> attributes = ViewSettingsHelper::attributeOrder( typeId, dataManager->viewSetting( typeId, "attribute_order" ) );

    QStringList headers;
    QList<TextWithLinks> items;

    Formatter formatter;

    for ( int i = 0; i < attributes.count(); i++ ) {
        int attributeId = attributes.at( i );
        const AttributeRow* attribute = dataManager->attributes()->find( attributeId );

        headers.append( ( attribute ? attribute->name() : QString() ) + tr( ":" ) );

        const ValueRow* row = dataManager->values()->find( attributeId, issue->issueId() );
        QString value = formatter.convertAttributeValue( attribute ? attribute->definition() : QString(), row ? row->value() : QString(), true );

        items.append( TextWithLinks::parse( value, flags ) );
    }

    writer->writeInfoList( headers, items );
}

void IssueDetailsGenerator::writeHistory( TextWriter* writer, TextWithLinks::Flags flags )
{
    QList<TextWithLinks> changes;

    int changeUser = 0;
    QDateTime changeDate;

    for ( int i = 0; i < m_items.count(); i++ ) {
        int id = m_items[ i ];

        const ChangeRow* change = dataManager->changes()->find( id );
        if ( !change )
            continue;

        if ( change->changeType() <= ValueChanged && changes.count() > 0 ) {
            if ( change->modifiedUser() == changeUser && changeDate.secsTo( change->modifiedDate() ) < 180 ) {
                changes.append( formatChange( change, flags ) );
                continue;
            }
        }

        if ( changes.count() > 0 ) {
            writer->writeBulletList( changes );
            changes.clear();
        }

        int row;

        switch ( change->changeType() ) {
            case IssueCreated:
            case IssueRenamed:
            case ValueChanged:
                row = writer->appendLayoutRows( 1 );
                writer->mergeLayoutCells( row, 0, 1, 4 );
                writer->gotoLayoutCell( row, 0, TextWriter::NormalCell );
                changeUser = change->createdUser();
                changeDate = change->createdDate();
                if ( change->changeType() == IssueCreated )
                    writer->writeBlock( tr( "Issue Created" ), TextWriter::Header3Block );
                else
                    writer->writeBlock( tr( "Issue Modified" ), TextWriter::Header3Block );
                writer->writeBlock( formatStamp( changeUser, changeDate ), TextWriter::SmallBlock );
                changes.append( formatChange( change, flags ) );
                break;

            case CommentAdded:
                if ( const CommentRow* comment = dataManager->comments()->find( id ) ) {
                    row = writer->appendLayoutRows( 2 );
                    writer->mergeLayoutCells( row, 0, 1, 3 );
                    writer->gotoLayoutCell( row, 0, TextWriter::NormalCell );
                    writer->writeBlock( tr( "Comment %1" ).arg( TableModelsHelper::formatId( id ) ), TextWriter::Header3Block, QString( "id%1" ).arg( id ) );
                    writer->writeBlock( formatStamp( change ), TextWriter::SmallBlock );
                    if ( !flags.testFlag( TextWithLinks::NoInternalLinks ) && checkChangeLinks( change ) ) {
                        writer->gotoLayoutCell( row, 3, TextWriter::LinksCell );
                        writer->writeBlock( changeLinks( change, flags ), TextWriter::LinksBlock );
                    }
                    writer->mergeLayoutCells( row + 1, 0, 1, 4 );
                    writer->gotoLayoutCell( row + 1, 0, TextWriter::CommentCell );
                    writer->writeBlock( TextWithLinks::parse( comment->text(), flags ), TextWriter::NormalBlock );
                }
                break;

            case FileAdded:
                if ( const FileRow* file = dataManager->files()->find( id ) ) {
                    row = writer->appendLayoutRows( 2 );
                    writer->mergeLayoutCells( row, 0, 1, 3 );
                    writer->gotoLayoutCell( row, 0, TextWriter::NormalCell );
                    writer->writeBlock( tr( "Attachment %1" ).arg( TableModelsHelper::formatId( id ) ), TextWriter::Header3Block, QString( "id%1" ).arg( id ) );
                    writer->writeBlock( formatStamp( change ), TextWriter::SmallBlock );
                    if ( !flags.testFlag( TextWithLinks::NoInternalLinks ) && checkChangeLinks( change ) ) {
                        writer->gotoLayoutCell( row, 3, TextWriter::LinksCell );
                        writer->writeBlock( changeLinks( change, flags ), TextWriter::LinksBlock );
                    }
                    writer->mergeLayoutCells( row + 1, 0, 1, 4 );
                    writer->gotoLayoutCell( row + 1, 0, TextWriter::FileCell );
                    writer->writeBlock( formatFile( file, flags ), TextWriter::NormalBlock );
                }
                break;

            case IssueMoved:
                row = writer->appendLayoutRows( 1 );
                writer->mergeLayoutCells( row, 0, 1, 4 );
                writer->gotoLayoutCell( row, 0, TextWriter::NormalCell );
                writer->writeBlock( tr( "Issue Moved" ), TextWriter::Header3Block );
                writer->writeBlock( formatStamp( change->modifiedUser(), change->modifiedDate() ), TextWriter::SmallBlock );
                writer->writeBulletList( QList<TextWithLinks>() << formatChange( change, flags ) );
                break;
        }
    }

    if ( changes.count() > 0 )
        writer->writeBulletList( changes );
}

QString IssueDetailsGenerator::formatStamp( int userId, const QDateTime& date )
{
    Formatter formatter;
    return QString::fromUtf8( "%1 — %2" ).arg( formatter.formatDateTime( date, true ), TableModelsHelper::userName( userId ) );
}

QString IssueDetailsGenerator::formatStamp( const ChangeRow* change )
{
    QString stamp = formatStamp( change->createdUser(), change->createdDate() );
    if ( change->stamp() != change->changeId() ) {
        Formatter formatter;
        stamp += QLatin1String( " (" );
        stamp += tr( "last edited:" );
        stamp += QString::fromUtf8( " %1 — %2)" ).arg( formatter.formatDateTime( change->modifiedDate(), true ), TableModelsHelper::userName( change->modifiedUser() ) );
    }
    return stamp;
}

TextWithLinks IssueDetailsGenerator::formatChange( const ChangeRow* change, TextWithLinks::Flags flags )
{
    TextWithLinks result( flags );

    switch ( change->changeType() ) {
        case IssueCreated:
            result.appendText( tr( "Name:" ) );
            result.appendText( " \"" );
            result.appendParsed( change->newValue() );
            result.appendText( "\"" );
            break;

        case IssueRenamed:
            result.appendText( tr( "Name:" ) );
            result.appendText( " \"" );
            result.appendParsed( change->oldValue() );
            result.appendText( QString::fromUtf8( "\" → \"" ) );
            result.appendParsed( change->newValue() );
            result.appendText( "\"" );
            break;

        case ValueChanged: {
            const AttributeRow* attribute = dataManager->attributes()->find( change->attributeId() );
            result.appendText( attribute ? attribute->name() : tr( "Unknown Attribute" ) );

            Formatter formatter;
            QString oldValue = formatter.convertAttributeValue( attribute->definition(), change->oldValue(), false );
            QString newValue = formatter.convertAttributeValue( attribute->definition(), change->newValue(), false );

            result.appendText( tr( ":" ) );
            result.appendText( " " );

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

        case IssueMoved: {
            result.appendText( tr( "Folder" ) );
            result.appendText( ": " );

            const FolderRow* fromFolder = dataManager->folders()->find( change->fromFolder() );
            if ( !fromFolder ) {
                result.appendText( tr( "Unknown Folder" ) );
            } else {
                result.appendText( "\"" );
                result.appendText( fromFolder->name() );
                result.appendText( "\"" );
            }

            result.appendText( QString::fromUtf8( " → " ) );

            const FolderRow* toFolder = dataManager->folders()->find( change->toFolder() );
            if ( !toFolder ) {
                result.appendText( tr( "Unknown Folder" ) );
            } else {
                result.appendText( "\"" );
                result.appendText( toFolder->name() );
                result.appendText( "\"" );
            }
            break;
        }

        default:
            break;
    }

    return result;
}

TextWithLinks IssueDetailsGenerator::formatFile( const FileRow* file, TextWithLinks::Flags flags )
{
    TextWithLinks result( flags );

    result.appendLink( file->name(), QString( "attachment://%1" ).arg( file->fileId() ) );
    result.appendText( " (" );
    result.appendText( TableModelsHelper::formatSize( file->size() ) );
    result.appendText( ")" );

    if ( !file->description().isEmpty() ) {
        result.appendText( QString::fromUtf8( " — " ) );
        result.appendParsed( file->description() );
    }

    return result;
}

TextWithLinks IssueDetailsGenerator::historyLinks( TextWithLinks::Flags flags )
{
    TextWithLinks result( flags );

    for ( int i = AllHistory; i <= OnlyFiles; i++ ) {
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
        }

        if ( m_history == i )
            result.appendText( text );
        else
            result.appendLink( text, QString( "command://filter/%1" ).arg( i ) );
    }

    return result;
}

bool IssueDetailsGenerator::checkChangeLinks( const ChangeRow* change )
{
    if ( change->createdUser() == dataManager->currentUserId() )
        return true;

    if ( TableModelsHelper::isIssueAdmin( change->issueId() ) )
        return true;

    return false;
}

TextWithLinks IssueDetailsGenerator::changeLinks( const ChangeRow* change, TextWithLinks::Flags flags )
{
    TextWithLinks result( flags );

    if ( change->changeType() == CommentAdded )
        result.appendLink( tr( "Edit" ), QString( "command://edit-comment/%1" ).arg( change->changeId() ) );
    else
        result.appendLink( tr( "Edit" ), QString( "command://edit-file/%1" ).arg( change->changeId() ) );

    result.appendText( " | " );

    if ( change->changeType() == CommentAdded )
        result.appendLink( tr( "Delete" ), QString( "command://delete-comment/%1" ).arg( change->changeId() ) );
    else
        result.appendLink( tr( "Delete" ), QString( "command://delete-file/%1" ).arg( change->changeId() ) );

    return result;
}
