/* lidx_classify.cpp
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
#include <libgen.h>
#include <boost/shared_ptr.hpp>
#include <algorithm>
#include <iterator>
#include <vector>
#include <mkv/markov_network_evolution.h>
#include <ea/data_structures/sequence_matrix.h>
#include <ea/iterators/camera.h>
#include <ea/generational_models/moran_process.h>
#include <ea/selection/rank.h>
#include <ea/fitness_function.h>
#include <ea/cmdline_interface.h>
#include <ea/datafiles/fitness.h>
using namespace ealib;

#include "evocadx.h"
#include <evocadx/db/lidx.h>


/*! Singleton container for LIDX data.
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
    
    data() : _initialized(false) {
    }
    
    //! Load data.
    void initialize(const std::string& train, const std::string& test) {
        if(!_initialized) {
            lidx::read(train, training);
            lidx::read(test, testing);
            _initialized = true;
        }
    }
    
    db_type training, testing;
    bool _initialized;
};
boost::shared_ptr<data> data::_inst; // define the instance pointer above


/*! Fitness function for classifying LIDX data via a MKV-controlled camera.
 */
struct lidx_classify : fitness_function<unary_fitness<double>, constantS, stochasticS> {
    //! Calculate fitness of ind.
	template <typename Individual, typename RNG, typename EA>
	double operator()(Individual& ind, RNG& rng, EA& ea) {
        typedef sequence_matrix<data::db_type::record_type::vector_type> matrix_type;
        typedef retina2_iterator<matrix_type> iterator_type;
        
        // lazy load of the mnist data (so we don't have to wait unless we
        // absolutely have to).
        data::instance()->initialize(get<EVOCADX_TRAIN_FILE>(ea), get<EVOCADX_TEST_FILE>(ea));
        
        // get a markov network:
        typename EA::phenotype_type &N = ealib::phenotype(ind, ea);
        int seed = rng.seed(); // save the seed
        
        // don't let empty networks play:
        if(N.ngates() == 0) {
            return 0.0;
        }
        
        double w=0.0; // accumulated fitness

        // analyze the lidx records:
        for(int i=0; i<get<EVOCADX_EXAMINE_N>(ea); ++i) {
            N.reset(seed);
            N.clear();
            
            // build a matrix facade for the lix record we're looking at:
            data::db_type::record_type& R=data::instance()->training[i];
            matrix_type M(R.data,
                          data::instance()->training.dim(0),
                          data::instance()->training.dim(1));
            
            // now build a retina iterator over this matrix:
            iterator_type ci(M, get<EVOCADX_FOVEA_SIZE>(ea), get<EVOCADX_RETINA_SIZE>(ea));
            ci.position(M.size1()/2, M.size2()/2);

            int updates = get<mkv::MKV_UPDATE_N>(ea);
            for(int j=0; j<updates; ++j) {
                N.update(ci);
                ci.move(algorithm::bits2ternary(N.begin_output()), algorithm::bits2ternary(N.begin_output()+2));
            }
            
            std::vector<int> D;
            algorithm::range_pair2indices(N.begin_output()+4, N.end_output(), std::back_inserter(D));

            if((D.size() == 1) && (D[0] == R.label)) {
                w += 1.0;
            }
        }

        return w;
    }
};


// Evolutionary algorithm definition.
typedef mkv::markov_network_evolution
< lidx_classify
, recombination::asexual
, generational_models::moran_process<selection::proportionate< >, selection::rank>
> ea_type;

//! Randomly shuffles the list of images at the end of every update.
template <typename EA>
struct shuffle_data : end_of_update_event<EA> {
    shuffle_data(EA& ea) : end_of_update_event<EA>(ea) { }
    virtual ~shuffle_data() { }
    
    virtual void operator()(EA& ea) {
        std::random_shuffle(data::instance()->training.records().begin(),
                            data::instance()->training.records().end(),
                            ea.rng());
    }
};

/*! Define the EA's command-line interface.
 */
template <typename EA>
class cli : public cmdline_interface<EA> {
public:
    virtual void gather_options() {
        mkv::add_options(this);
        add_option<POPULATION_SIZE>(this);
        add_option<MORAN_REPLACEMENT_RATE_P>(this);
        add_option<RUN_UPDATES>(this);
        add_option<RUN_EPOCHS>(this);
        add_option<CHECKPOINT_PREFIX>(this);
        add_option<RNG_SEED>(this);
        add_option<RECORDING_PERIOD>(this);
        
        add_option<EVOCADX_TRAIN_FILE>(this);
        add_option<EVOCADX_TEST_FILE>(this);
        add_option<EVOCADX_EXAMINE_N>(this);
        add_option<EVOCADX_LABELS_N>(this);
        add_option<EVOCADX_FOVEA_SIZE>(this);
        add_option<EVOCADX_RETINA_SIZE>(this);
    }
    
    virtual void gather_tools() {
    }
    
    virtual void gather_events(EA& ea) {
        add_event<datafiles::fitness_dat>(ea);
        add_event<shuffle_data>(ea);
    };
    
    virtual void before_initialization(EA& ea) {
        put<mkv::MKV_INPUT_N>(8*get<EVOCADX_RETINA_SIZE>(ea)
                              + get<EVOCADX_FOVEA_SIZE>(ea)*get<EVOCADX_FOVEA_SIZE>(ea)
                              , ea);
        put<mkv::MKV_OUTPUT_N>(4 + 2*get<EVOCADX_LABELS_N>(ea), ea);
    }
};
LIBEA_CMDLINE_INSTANCE(ea_type, cli);
