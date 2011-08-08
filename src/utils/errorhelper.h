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

#ifndef ERRORHELPER_H
#define ERRORHELPER_H

#include <QObject>

/**
* Class providing localized error messages.
*/
class ErrorHelper : public QObject
{
    Q_OBJECT
public:
    /**
    * Error codes for the WebIssues protocol.
    * @note Keep in sync with System_Api_Error in the server.
    */
    enum ErrorCode
    {
        LoginRequired = 300,
        AccessDenied = 301,
        IncorrectLogin = 302,
        UnknownProject = 303,
        UnknownFolder = 304,
        UnknownIssue = 305,
        UnknownFile = 306,
        UnknownUser = 307,
        UnknownType = 308,
        UnknownAttribute = 309,
        UnknownEvent = 310,
        ProjectAlreadyExists = 311,
        FolderAlreadyExists = 312,
        UserAlreadyExists = 313,
        TypeAlreadyExists = 314,
        AttributeAlreadyExists = 315,
        CannotDeleteProject = 316,
        CannotDeleteFolder = 317,
        CannotDeleteType = 318,
        InvalidString = 319,
        InvalidAccessLevel = 320,
        InvalidValue = 321,
        InvalidDefinition = 322,
        InvalidPreference = 323,
        InvalidSetting = 324,
        EmptyValue = 325,
        StringTooShort = 326,
        StringTooLong = 327,
        NumberTooLittle = 328,
        NumberTooGreat = 329,
        TooManyDecimals = 330,
        TooManyDigits = 331,
        InvalidFormat = 332,
        InvalidDate = 333,
        InvalidTime = 334,
        InvalidEmail = 335,
        NoMatchingItem = 336,
        DuplicateItems = 337,
        InvalidLimits = 338,
        IncompatibleType = 339,
        UnknownView = 340,
        UnknownColumn = 341,
        ViewAlreadyExists = 342,
        MissingColumn = 343,
        MissingAttribute = 344,
        NoItems = 345,
        PasswordNotMatching = 346,
        UnknownAlert = 347,
        AlertAlreadyExists = 348,
        InvalidAlertEmail = 349,
        UnknownComment = 350,
        CannotDeleteAttribute = 351,
        MustChangePassword = 352,
        CannotReusePassword = 353,
        ItemNotFound = 354,
        CommaNotAllowed = 355,
        TransactionDeadlock = 356,
        ConstraintConflict = 357,
    };

public:
    /**
    * Return the error message for given error code.
    */
    QString errorMessage( ErrorCode code ) const;

    /**
    * Return the error message formatted for status.
    */
    QString statusMessage( ErrorCode code ) const;
};

#endif
