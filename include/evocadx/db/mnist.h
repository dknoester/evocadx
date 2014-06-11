/* mnist.h
 * 
 * This file is part of EvoCADx.
 * 
 * Copyright 2014 David B. Knoester.
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
#ifndef _MNIST_H_
#define _MNIST_H_

#include <algorithm>
#include <vector>

class mnist {
public:
    //! Struct that contains information about a single image.
    struct labeled_image {
        typedef std::vector<int> image_type; //!< "image" type
        
        //! Constructor.
        labeled_image(unsigned char l, unsigned char* f, std::size_t n) : label(l) {
            img.insert(img.end(), f, f+n);
        }
        
        unsigned char label; //!< label for this image
        image_type img; //!< image
    };

    typedef std::vector<labeled_image> imagedb_type; //!< Type for a list of labeled images.

    //! Constructor.
    mnist() {
    }
    
    //! Initialize the database.
    void initialize(const std::string& lname, const std::string& iname);

    labeled_image& operator[](std::size_t i) { return _idb[i]; }
    
    //! Returns the image database.
    imagedb_type& db() { return _idb; }
    
    std::size_t size() const { return _idb.size(); }
    
protected:
    imagedb_type _idb; //!< image database    
};

#endif
