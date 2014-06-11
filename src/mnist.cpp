/* mnist.cpp
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

#include <arpa/inet.h>
#include <boost/scoped_array.hpp>
#include <fstream>
#include <set>
#include <stdexcept>
#include <evocadx/db/mnist.h>

/*! Load the image database.
 */
void mnist::initialize(const std::string& lname, const std::string& iname) {
    using namespace std;
    
    // Read in the labels:
    //
    ifstream ifs(lname.c_str(), ios::binary);
    if(!ifs.good()) {
        throw std::runtime_error("could not open: " + lname + " for reading");
    }
    
    // check that the magic number is right:
    unsigned int magic=0;
    ifs.read((char*)&magic, sizeof(magic));
    magic = ntohl(magic); // convert from file to host byte order
    assert(magic == 2049);
    
    // check that the file has more than 0 records:
    unsigned int lrecords=0;
    ifs.read((char*)&lrecords, sizeof(lrecords));
    lrecords = ntohl(lrecords);
    assert(lrecords > 0);
    
    // read in all the labels in one fell swoop:
    boost::scoped_array<unsigned char> labels(new unsigned char[lrecords]);
    ifs.read(reinterpret_cast<char*>(labels.get()), lrecords);
    ifs.close();
    
    
    // Read in the images:
    //
    ifs.open(iname.c_str(), ios::binary);
    if(!ifs.good()) {
        throw std::runtime_error("could not open: " + iname + " for reading");
    }
    
    // check that the magic number is right:
    magic=0;
    ifs.read((char*)&magic, sizeof(magic));
    magic = ntohl(magic); // convert from file to host byte order
    assert(magic == 2051);
    
    // check that the file has more than 0 records:
    unsigned int irecords=0;
    ifs.read((char*)&irecords, sizeof(irecords));
    irecords = ntohl(irecords);
    assert(irecords > 0);
    
    // sanity; make sure that our labels & images have the same number of records
    assert(irecords == lrecords);
    
    // read in the size of the images:
    unsigned int rows, cols;
    ifs.read((char*)&rows, sizeof(rows));
    rows = ntohl(rows);
    ifs.read((char*)&cols, sizeof(cols));
    cols = ntohl(cols);
    
    // read in the images, match them up with their labels, and figure out 
    // how many labels we have (we need this to determine the number of outputs):
    std::set<char> lset;
    boost::scoped_array<unsigned char> img(new unsigned char[rows*cols]);
    //    boost::scoped_array<unsigned char> bbimg(new unsigned char[rows*cols]);
    
    for(unsigned int i=0; i<irecords; ++i) {
        lset.insert(labels[i]); // incrementally build up our label set...
        ifs.read(reinterpret_cast<char*>(img.get()), rows*cols); // read the image
        if(!ifs.good()) {
            throw std::runtime_error("could not read from: " + lname);
        }
        
        // now, build the labeled_image struct:
        _idb.push_back(labeled_image(labels[i], img.get(), rows*cols));
    }
    assert(_idb.size() == irecords);
}
