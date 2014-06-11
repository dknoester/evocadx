/* mnist2lidx.cpp
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
#include <iostream>
#include <fstream>
#include <string>
#include <arpa/inet.h>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#include <evocadx/db/lidx.h>
#include <evocadx/db/mnist.h>

using namespace std;


// 1==label file
// 2==image file
// 3==dst file
int main(int argc, const char * argv[]) {
    mnist src;
    src.initialize(argv[1], argv[2]);
    
    typedef lidx::lidx_db<int, int> db_type;
    db_type dst;
    dst.dims().push_back(28);
    dst.dims().push_back(28);
    
    for(std::size_t i=0; i<src.size(); ++i) {
        mnist::labeled_image& img=src[i];
        db_type::record_type rec;
        rec.label = static_cast<int>(img.label);
        rec.data.insert(rec.data.end(), img.img.begin(), img.img.end());
        dst.records().push_back(rec);
    }
    
    lidx::write(argv[3], dst);
}
