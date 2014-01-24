/* centroid.cpp
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
#include <ea/evolutionary_algorithm.h>
#include <ea/generational_models/moran_process.h>
#include <ea/fitness_function.h>
#include <ea/cmdline_interface.h>
#include <ea/datafiles/fitness.h>
#include <ea/mkv/markov_network.h>
using namespace ealib;

#include "analysis.h"
#include "evocadx.h"


/*! Image centroid fitness function for Markov networks.
 */
struct centroid_fitness : fitness_function<unary_fitness<double>, constantS, stochasticS> {
    
    /*! Initialize this fitness function -- load data, etc. */
    template <typename RNG, typename EA>
    void initialize(RNG& rng, EA& ea) {
    }
    
	template <typename Individual, typename RNG, typename EA>
	double operator()(Individual& ind, RNG& rng, EA& ea) {
        // get the phenotype (markov network):
        typename EA::phenotype_type &N = ealib::phenotype(ind, ea);
        
        // probably want to reset the RNG for the markov network:
        N.reset(rng.seed());
        
        // now, set the values of the bits in the input vector:
        double f=0.0;
        
        // analyze images...
        
        // and return some measure of fitness:
        return f;
    }
};

// Evolutionary algorithm definition.
typedef evolutionary_algorithm
< individual<mkv::representation_type, centroid_fitness, markov_network< >, indirectS, mkv::default_traits>
, mkv::ancestor_generator
, mkv::mutation_type
, recombination::asexual
, generational_models::moran_process< >
, dont_stop
, mkv::configuration
> ea_type;

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
        
        add_option<EVOCADX_DATADIR>(this);
    }
    
    
    virtual void gather_tools() {
    }
    
    virtual void gather_events(EA& ea) {
        add_event<datafiles::fitness_dat>(ea);
    };
};
LIBEA_CMDLINE_INSTANCE(ea_type, cli);
