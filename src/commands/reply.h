/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2013 WebIssues Team
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

#ifndef REPLY_H
#define REPLY_H

#include <QString>
#include <QList>
#include <QVariant>

/**
* Structure representing a single line of the server's reply.
*/
class ReplyLine
{
public:
    /**
    * Default constructor.
    * Create an empty line.
    */
    ReplyLine();

    /**
    * Destructor.
    */
    ~ReplyLine();

public:
    /**
    * Set the keyword of the line.
    */
    void setKeyword( const QString& keyword ) { m_keyword = keyword; }

    /**
    * Return the keyword of the line.
    */
    const QString& keyword() const { return m_keyword; }

    /**
    * Set a list of arguments of the line.
    * Arguments can be integers and strings.
    */
    void setArgs( const QVariantList& args ) { m_args = args; }

    /**
    * Return the list of arguments of the line.
    */
    const QVariantList& args() const { return m_args; }

    /**
    * Add an integer argument to the line.
    */
    void addArg( int number );

    /**
    * Add a string argument to the line.
    */
    void addArg( const QString& string );

    /**
    * Return the given argument.
    */
    const QVariant& arg( int index ) const { return m_args.at( index ); }

    /**
    * Read the given integer argument from the line.
    */
    int argInt( int index ) const { return m_args.at( index ).toInt(); }

    /**
    * Read the given string argument from the line.
    */
    QString argString( int index ) const { return m_args.at( index ).toString(); }

private:
    QString m_keyword;
    QVariantList m_args;
};

/**
* Structure representing the server's reply to a command.
*
* A reply consists of one or more lines. Each line consists of a keyword and zero
* or more arguments. The format of reply lines depends on the command and is described
* in the WebIssues protocol documentation.
*/
class Reply
{
public:
    /**
    * Default constructor.
    * Create an empty reply.
    */
    Reply();

    /**
    * Destructor.
    */
    ~Reply();

public:
    /**
    * Set a list of lines in the reply.
    */
    void setLines( const QList<ReplyLine>& lines ) { m_lines = lines; }

    /**
    * Return a list of lines from the reply.
    */
    const QList<ReplyLine>& lines() const { return m_lines; }

    /**
    * Add a line to the reply.
    */
    void addLine( const ReplyLine& line );

    /**
    * Return the given line.
    */
    const ReplyLine& at( int index ) const { return m_lines.at( index ); }

    /**
    * Return the number of lines.
    */
    int count() const { return m_lines.count(); }

private:
    QList<ReplyLine> m_lines;
};

/**
* A rule for validating the syntax of a reply line.
*
* The rule verifies the signature of a line and the number of occurences
* of the given line in the server's reply.
*
* The signature checks the keyword and the number and types of arguments.
* It is a string consisting of the keyword, followed by a space and a lowercase
* 'i' for each integer argument and a lowercase 's' for each string argument.
* If the line has no arguments the signature is only the keyword.
*/
class ReplyRule
{
public:
    /**
    * The line multiplicity.
    */
    enum Multiplicity
    {
        /** The line must occur exactly once. */
        One,
        /** The line can occur zero or more times. */
        ZeroOrMore,
        /** The line can occur zero or one time. */
        ZeroOrOne,
    };

public:
    /**
    * Constructor.
    * @param signature The signature of the line.
    * @param multiplicity The multiplicity type of the rule.
    */
    ReplyRule( const QString& signature = QString(), Multiplicity multiplicity = One );

    /**
    * Desctructor.
    */
    ~ReplyRule();

public:
    /**
    * Set the line signature for this rule.
    */
    void setSignature( const QString& signature ) { m_signature = signature; }

    /**
    * Return the line signature for this rule.
    */
    const QString& signature() const { return m_signature; }

    /**
    * Set the multiplicity of this rule.
    */
    void setMultiplicity( Multiplicity multiplicity ) { m_multiplicity = multiplicity; }

    /**
    * Return the multiplicity of this rule.
    */
    Multiplicity multiplicity() const { return m_multiplicity; }

private:
    QString m_signature;
    Multiplicity m_multiplicity;
};

#endif
