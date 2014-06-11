/* evocadx.cpp
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
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#include "evocadx.h"

//! Returns a vector of filenames matching regex r in a recursive search of directory d.
filename_vector_type find_files(const std::string& d, const std::string& r) {
    using namespace boost::filesystem;
    filename_vector_type filenames;
    boost::regex e(r);
    recursive_directory_iterator i((path(d))); // the extra parens are needed to avoid the most vexing parse
    recursive_directory_iterator end;
    
    for( ; i!=end; ++i) {
        if(boost::filesystem::is_regular_file(*i)) {
            std::string abspath=absolute(*i).string();
            if(boost::regex_match(abspath,e)) {
                filenames.push_back(abspath);
            }
        }
    }
    return filenames;
}
