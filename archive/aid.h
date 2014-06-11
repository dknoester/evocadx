/* aid.h
 *
 * This file is part of AID.
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
#ifndef _AID_H_
#define _AID_H_

#include <boost/shared_ptr.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>

#include <ea/metadata.h>
#include <lidx.h>

// These are the indices of the [first,last) records that a given subpopulation
// will examine.
LIBEA_MD_DECL(AID_FIRST, "aid.first", std::size_t);
LIBEA_MD_DECL(AID_LAST, "aid.last", std::size_t);

// Filenames for train and test data:
LIBEA_MD_DECL(AID_TRAIN, "aid.train_filename", std::string);
LIBEA_MD_DECL(AID_TEST, "aid.test_filename", std::string);

// RNG seed; used only for shuffling images upon initialization:
LIBEA_MD_DECL(AID_RNG_SEED, "aid.rng_seed", unsigned int);

// Number of classes present in the data:
LIBEA_MD_DECL(AID_NCLASSES, "aid.nclasses", unsigned int);

// typedefs for matrices used in fitness functions:
typedef boost::numeric::ublas::matrix<int> result_matrix;
typedef boost::numeric::ublas::matrix_row<result_matrix> result_row;
typedef boost::numeric::ublas::matrix_column<result_matrix> result_column;
typedef std::vector<int> ivector_type;

//! Struct used as a base class to provide tracing functionality during fitness evaluation.
struct aid_fitness_callback {
    // image #, image row, image class, row data, v1 input, v2 input, and v2 output
    virtual void row(int i, int j, int c, ivector_type& row, result_matrix& v1_input, ivector_type& v2_input, ivector_type& v2_output) { }
    
    // image #, image class, retina i, retina j, output
    virtual void retina(int i, int c, int ri, int rj, int rk, ivector_type& input, ivector_type& output) { }

    virtual void retina(int i, int c, int ri, int rj, ivector_type& input, ivector_type& output) { }

    // data record
    virtual void data(ivector_type& d) { }
};


/*! Singleton container for train and test data.
 */
struct data {
    typedef lidx::lidx_db<int, int> db_type;
    static boost::shared_ptr<data> _inst;
    
    static data* instance() {
        if(!_inst.get()) {
            _inst.reset(new data());
        }
        return _inst.get();
    }
    
    data() {
    }
    
    //! Load data.
    void initialize(const std::string& train, const std::string& test) {
        lidx::read(train, training);
        lidx::read(test, testing);
    }
    
    db_type training, testing;
};

#endif
