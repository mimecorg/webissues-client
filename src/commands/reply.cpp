/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2014 WebIssues Team
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

#include "reply.h"

ReplyLine::ReplyLine()
{
}

ReplyLine::~ReplyLine()
{
}

void ReplyLine::addArg( int number )
{
    m_args.append( QVariant( number ) );
}

void ReplyLine::addArg( const QString& string )
{
    m_args.append( QVariant( string ) );
}

Reply::Reply()
{
}

Reply::~Reply()
{
}

void Reply::addLine( const ReplyLine& line )
{
    m_lines.append( line );
}

ReplyRule::ReplyRule( const QString& signature, Multiplicity multiplicity ) :
    m_signature( signature ),
    m_multiplicity( multiplicity )
{
}

ReplyRule::~ReplyRule()
{
}
