/* png.h
 *
 * This file is part of EvoCADx.
 *
 * Copyright 2014 Emily L. Dolson, David B. Knoester.
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
 */
#ifndef _PNG_H
#define _PNG_H

#include <vector>
#include <string>

/*! This class loads a PNG and stores the pixel values and metadata in object form.
 */
class png {
public:
    typedef std::vector<unsigned char> pixel_vector_type; //<! Type for storing pixel data.
    typedef std::pair<float, float> float_pair; //<! Type for storing centroid.

    //! Constructor.
    png(const std::string& filename);
    //! Returns the width of this image, in pixels.
    unsigned long get_width() const;
    //! Returns the height of this image, in pixels.
    unsigned long get_height() const;
    //! Returns the 16b value of the n'th pixel.
    uint16_t operator[](std::size_t n) const;
    //! Returns centroid of light pixels
    png::float_pair get_centroid(bool weighted = true, int threshold = 1000);
    //! Returns distance of specified x,y coordinates to centroid
    float distance_to_centroid(int x, int y, bool weighted = true, int threshold = 1000);
    
private:
    pixel_vector_type _pixels; //!< Pixel data.
    unsigned long _width; //!< Width of image.
    unsigned long _height; //<! Height of image.
};

#endif
