/* lidxgen.cpp
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
#include <boost/lexical_cast.hpp>

#include <evocadx/db/lidx.h>
#include <ea/rng.h>


int numerals[10][15] = {
    { 1, 1, 1,
        1, 0, 1,
        1, 0, 1,
        1, 0, 1,
        1, 1, 1,
    }, { 0, 0, 1,
        0, 0, 1,
        0, 0, 1,
        0, 0, 1,
        0, 0, 1,
    }, { 1, 1, 1,
        0, 0, 1,
        0, 1, 0,
        1, 0, 0,
        1, 1, 1,
    }, { 1, 1, 1,
        0, 0, 1,
        1, 1, 1,
        0, 0, 1,
        1, 1, 1,
    }, { 1, 0, 1,
        1, 0, 1,
        1, 1, 1,
        0, 0, 1,
        0, 0, 1,
    }, { 1, 1, 1,
        1, 0, 0,
        1, 1, 1,
        0, 0, 1,
        1, 1, 1,
    }, { 1, 0, 0,
        1, 0, 0,
        1, 1, 1,
        1, 0, 1,
        1, 1, 1,
    }, { 1, 1, 1,
        0, 0, 1,
        0, 0, 1,
        0, 0, 1,
        0, 0, 1,
    }, { 1, 1, 1,
        1, 0, 1,
        1, 1, 1,
        1, 0, 1,
        1, 1, 1,
    }, { 1, 1, 1,
        1, 0, 1,
        1, 1, 1,
        0, 0, 1,
        0, 0, 1,
    }
};


// 1==repititions
// 2==dst file
// 3==field rows
// 4==field cols
// 5==place random, random horizontal==1, random vertical==2, center
int main(int argc, const char * argv[]) {
    std::size_t rows=boost::lexical_cast<std::size_t>(argv[3]);
    std::size_t cols=boost::lexical_cast<std::size_t>(argv[4]);
    
    int fix=boost::lexical_cast<int>(argv[5]);
    
    typedef lidx::lidx_db<int, int> db_type;
    db_type dst;
    dst.dims().push_back(rows);
    dst.dims().push_back(cols);

    ealib::default_rng_type rng(42);
    
    std::size_t r=boost::lexical_cast<std::size_t>(argv[1]);
    for( ; r>0; --r) {
        for(std::size_t i=0; i<10; ++i) {
            std::vector<int> f(rows*cols, 0);

            db_type::record_type rec;
            
            std::size_t sr=rng(rows-5);
            std::size_t sc=rng(cols-3);
            switch(fix) {
                case 0: break;
                case 1: sr = (rows-5)/2; break;
                case 2: sc = (cols-3)/2; break;
                case 3: sr = (rows-5)/2; sc = (cols-3)/2; break;
            }
            
            for(std::size_t y=0; y<5; ++y) {
                for(std::size_t x=0; x<3; ++x) {
                    f[sc+x + (sr+y)*cols] = numerals[i][x+y*3];
                }
            }
            
//            for(std::size_t y=0; y<rows; ++y) {
//                for(std::size_t x=0; x<cols; ++x) {
//                    std::cerr << f[x+cols*y];
//                }
//                std::cerr << std::endl;
//            }
//            std::cerr << std::endl;
            
            rec.label = i;
            rec.data.insert(rec.data.end(), f.begin(), f.end());
            dst.records().push_back(rec);
        }
    }
    
    lidx::write(argv[2], dst);
}
