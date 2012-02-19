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

#include "errorhelper.h"

QString ErrorHelper::errorMessage( ErrorCode code ) const
{
    switch ( code ) {
        case LoginRequired:
            return tr( "Your session has expired; please reconnect" );
        case AccessDenied:
            return tr( "You have no permission to perform this operation" );
        case IncorrectLogin:
            return tr( "Invalid login or password" );
        case UnknownProject:
            return tr( "Project does not exist" );
        case UnknownFolder:
            return tr( "Folder does not exist" );
        case UnknownIssue:
            return tr( "Issue does not exist" );
        case UnknownFile:
            return tr( "Attachment does not exist" );
        case UnknownUser:
            return tr( "User does not exist" );
        case UnknownType:
            return tr( "Type does not exist" );
        case UnknownAttribute:
            return tr( "Attribute does not exist" );
        case UnknownEvent:
            return tr( "Event does not exist" );
        case ProjectAlreadyExists:
            return tr( "A project with this name already exists" );
        case FolderAlreadyExists:
            return tr( "A folder with this name already exists" );
        case UserAlreadyExists:
            return tr( "A user with this login or name already exists" );
        case TypeAlreadyExists:
            return tr( "A type with this name already exists" );
        case AttributeAlreadyExists:
            return tr( "An attribute with this name already exists" );
        case CannotDeleteProject:
            return tr( "Project cannot be deleted" );
        case CannotDeleteFolder:
            return tr( "Folder cannot be deleted" );
        case CannotDeleteType:
            return tr( "Type cannot be deleted" );
        case InvalidString:
            return tr( "Text contains invalid characters" );
        case InvalidAccessLevel:
            return tr( "Access level is invalid" );
        case InvalidValue:
            return tr( "Value is invalid" );
        case InvalidDefinition:
            return tr( "Definition is invalid" );
        case InvalidPreference:
            return tr( "Invalid preference value" );
        case InvalidSetting:
            return tr( "Invalid setting value" );
        case EmptyValue:
            return tr( "Required value is missing" );
        case StringTooShort:
            return tr( "Text is too short" );
        case StringTooLong:
            return tr( "Text is too long" );
        case NumberTooLittle:
            return tr( "Number is too small" );
        case NumberTooGreat:
            return tr( "Number is too big" );
        case TooManyDecimals:
            return tr( "Number has too many decimal digits" );
        case TooManyDigits:
            return tr( "Number has too many digits" );
        case InvalidFormat:
            return tr( "Value has incorrect format" );
        case InvalidDate:
            return tr( "Date is not correct" );
        case InvalidTime:
            return tr( "Time is not correct" );
        case InvalidEmail:
            return tr( "Email address is invalid" );
        case NoMatchingItem:
            return tr( "No matching item is selected" );
        case DuplicateItems:
            return tr( "Duplicate items are entered" );
        case InvalidLimits:
            return tr( "Minimum value is greater than maximum value" );
        case IncompatibleType:
            return tr( "Incompatible attribute type" );
        case UnknownView:
            return tr( "View does not exist" );
        case UnknownColumn:
            return tr( "Column does not exist" );
        case ViewAlreadyExists:
            return tr( "A view with this name already exists" );
        case MissingColumn:
            return tr( "A required column is missing" );
        case MissingAttribute:
            return tr( "An attribute is missing" );
        case NoItems:
            return tr( "No items are specified" );
        case PasswordNotMatching:
            return tr( "Passwords do not match; please retype them" );
        case UnknownAlert:
            return tr( "Alert does not exist" );
        case AlertAlreadyExists:
            return tr( "Alert already exists" );
        case InvalidAlertEmail:
            return tr( "Invalid alert email setting" );
        case UnknownComment:
            return tr( "Comment does not exist" );
        case CannotDeleteAttribute:
            return tr( "Attribute cannot be deleted" );
        case MustChangePassword:
            return tr( "You must change your password" );
        case CannotReusePassword:
            return tr( "Cannot reuse password; choose different password" );
        case ItemNotFound:
            return tr( "The specified item was not found" );
        case CommaNotAllowed:
            return tr( "Value cannot contain a comma" );
        case TransactionDeadlock:
            return tr( "Concurrency error; please try again" );
        case ConstraintConflict:
            return tr( "One of the dependent objects no longer exists" );
        default:
            return QString();
    }
}

QString ErrorHelper::statusMessage( ErrorCode code ) const
{
    QString message = errorMessage( code );

    if ( message.isEmpty() )
        return tr( "Some of the values you entered are incorrect." );

    return tr( "Incorrect value: %1." ).arg( message );
}
