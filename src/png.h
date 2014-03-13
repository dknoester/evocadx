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
#include <stdint.h>
#include <gd.h>

/*! This class loads a PNG and stores the pixel values and metadata in object form.
 */
class png {
public:
    typedef uint16_t value_type; //!< Value type for pixel data.
    typedef std::vector<double> histogram_vector_type; //<! Type for storing histograms.
    typedef std::vector<uint16_t> pixel_vector_type; //<! Type for storing pixel data.
    typedef std::pair<double, double> centroid_type; //<! Type for storing centroid.
    
    //! Constructor.
    png(const std::string& filename, bool weighted=true, value_type threshold=1000);

    //! Returns the width of this image, in pixels.
    unsigned long width() const;
    
    //! Returns the height of this image, in pixels.
    unsigned long height() const;

    //! Returns the size of this image, in pixels.
    unsigned long size() const;

    //! Returns the number of rows (when treating this image as a matrix).
    unsigned long size1() const;
    
    //! Returns the number of columns (when treating this image as a matrix).
    unsigned long size2() const;
    
    //! Returns the number of bytes per pixel.
    unsigned long get_bpp() const;
    
    //! Returns the image centoid
    centroid_type& get_centroid();

    //! Returns the image centoid
    bool write_pgm(const std::string& outfilename);

    //! Returns the 16b value of the n'th pixel.
    value_type& operator[](std::size_t n);

    //! Returns the 16b value of the n'th pixel (const-qualified).
    const value_type& operator[](std::size_t n) const;

    //! Returns the distance of specified (x,y) coordinate to the centroid of this image.
    double distance_to_centroid(std::size_t x, std::size_t y);
    
private:
    pixel_vector_type _pixels; //!< Pixel data.
    unsigned int _bpp; //!< Bytes per pixel.
    unsigned long _width; //!< Width of image in pixels.
    unsigned long _height; //<! Height of image in pixels.
    value_type _threshold; //!< Value below which pixels are set to 0.
    centroid_type _centroid; //!< Centroid of the image.
};

#endif
