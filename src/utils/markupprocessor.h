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

#ifndef MARKUPPROCESSOR_H 
#define MARKUPPROCESSOR_H 

#include "utils/htmltext.h"

#include <QRegExp>

/**
* Convert text with markup to HTML formatting.
*/
class MarkupProcessor
{
private:
    MarkupProcessor( const QString& text, HtmlText::Flags flags );
    ~MarkupProcessor();

public:
    static HtmlText parse( const QString& text, HtmlText::Flags flags = 0 );

private:
    enum Tokens
    {
        T_END,
        T_TEXT,
        T_START_CODE,
        T_START_LIST,
        T_START_QUOTE,
        T_START_RTL,
        T_END_CODE,
        T_END_LIST,
        T_END_QUOTE,
        T_END_RTL,
        T_LINK,
        T_BACKTICK,
        T_NEWLINE
    };

private:
    void next();

    void parse();
    void parseBlock();
    void parseText();
    void parseCode();
    void parseList();
    void parseQuote();
    void parseRtl();

    int itemLevel();

private:
    QString m_text;
    HtmlText::Flags m_flags;

    QRegExp m_regExp;

    int m_index;
    bool m_matched;
    
    int m_token;
    QString m_value;
    QString m_extra;
    QString m_rawValue;

    QString m_result;
};

#endif
