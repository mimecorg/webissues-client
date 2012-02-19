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

#include "command.h"
#include "formdatamessage.h"

#include <QNetworkAccessManager>

Command::Command() :
    m_attachmentInput( NULL ),
    m_binaryResponseOutput( NULL ),
    m_acceptNullReply( false ),
    m_reportNullReply( false )
{
}

Command::~Command()
{
}

void Command::addArg( int number )
{
    m_args.append( QVariant( number ) );
}

void Command::addArg( const QString& string )
{
    m_args.append( QVariant( string ) );
}

void Command::addRule( const QString& signature, ReplyRule::Multiplicity multiplicity )
{
    m_rules.append( ReplyRule( signature, multiplicity ) );
}
