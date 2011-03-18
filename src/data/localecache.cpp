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

#include "localecache.h"

#include "commands/command.h"

LocaleCache::LocaleCache( QObject* parent ) : QObject( parent ),
    m_populated( false )
{
}

LocaleCache::~LocaleCache()
{
}

QList<LocaleFormat> LocaleCache::formats( const QString& type ) const
{
    QList<LocaleFormat> result;

    for ( int i = 0; i < m_formats.count(); i++ ) {
        if ( m_formats.at( i ).type() == type )
            result.append( m_formats.at( i ) );
    }

    return result;
}

Command* LocaleCache::updateLocale()
{
    Command* command = new Command();

    command->setKeyword( "GET LOCALE" );

    command->setAcceptNullReply( true );
    command->addRule( "L ss", ReplyRule::ZeroOrMore );
    command->addRule( "F sss", ReplyRule::ZeroOrMore );
    command->addRule( "Z si", ReplyRule::ZeroOrMore );

    connect( command, SIGNAL( commandReply( const Reply& ) ), this, SLOT( updateLocaleReply( const Reply& ) ) );

    return command;
}

void LocaleCache::updateLocaleReply( const Reply& reply )
{
    m_languages.clear();
    m_formats.clear();
    m_timeZones.clear();

    for ( int i = 0; i < reply.lines().count(); i++ ) {
        ReplyLine line = reply.lines().at( i );
        if ( line.keyword() == QLatin1String( "L" ) ) {
            LocaleLanguage language( line.argString( 0 ), line.argString( 1 ) );
            m_languages.append( language );
        } else if ( line.keyword() == QLatin1String( "F" ) ) {
            LocaleFormat format( line.argString( 0 ), line.argString( 1 ), line.argString( 2 ) );
            m_formats.append( format );
        } else { // "Z"
            LocaleTimeZone timeZone( line.argString( 0 ), line.argInt( 1 ) );
            m_timeZones.append( timeZone );
        }
    }

    m_populated = true;
}
