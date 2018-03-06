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

#ifndef ICONLOADER_H
#define ICONLOADER_H

#include <QPixmap>
#include <QIcon>

/**
* Class for loading icons and pixmaps.
*
* Icons and pixmaps are cached for faster access.
*/
class IconLoader
{
public:
    /**
    * Load the pixmap with the given name and size.
    * @param name Name of the image file.
    * @param size Size of the pixmap.
    */
    static QPixmap pixmap( const QString& name, int size = 16 );

    /**
    * Load the pixmap with an overlay image.
    * @param name Name of the image file.
    * @param overlay Name of the overlay image file.
    * @param size Size of the pixmap.
    */
    static QPixmap overlayedPixmap( const QString& name, const QString& overlay, int size = 16 );

    /**
    * Load the pixmap with two overlay images.
    * @param name Name of the image file.
    * @param overlay1 Name of the first overlay image file.
    * @param overlay2 Name of the second overlay image file.
    * @param size Size of the pixmap.
    */
    static QPixmap overlayedPixmap( const QString& name, const QString& overlay1, const QString& overlay2, int size = 16 );

    /**
    * Create an icon with the given name.
    * The icon contains all available image sizes.
    * @param name Name of the image file.
    */
    static QIcon icon( const QString& name );

    /**
    * Create an icon with an overlay image.
    * @param name Name of the image file.
    * @param overlay Name of the overlay image file.
    */
    static QIcon overlayedIcon( const QString& name, const QString& overlay );
};

#endif
