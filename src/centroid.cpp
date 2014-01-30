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
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>
#include <ea/mkv/markov_evolution_algorithm.h>
#include <ea/mkv/sequence_matrix.h>
#include <ea/mkv/camera.h>
#include <ea/generational_models/moran_process.h>
#include <ea/fitness_function.h>
#include <ea/cmdline_interface.h>
#include <ea/datafiles/fitness.h>
using namespace ealib;

#include "analysis.h"
#include "evocadx.h"
#include "png.h"

/*! Image centroid fitness function for Markov networks.
 */
struct centroid_fitness : fitness_function<unary_fitness<double>, constantS, stochasticS> {
    typedef boost::shared_ptr<png> png_ptr_type;
    typedef std::vector<png_ptr_type> image_vector_type; //!< Type of vector of PNGs.
    
    
    /*! Initialize this fitness function -- load data, etc. */
    template <typename RNG, typename EA>
    void initialize(RNG& rng, EA& ea) {
        using namespace boost::filesystem;
        boost::regex e(get<EVOCADX_FILE_REGEX>(ea));
        recursive_directory_iterator i(path(get<EVOCADX_DATADIR>(ea)));
        recursive_directory_iterator end;
        
        for( ; i!=end; ++i) {
            if(boost::filesystem::is_regular_file(*i)) {
                std::string abspath=absolute(*i).string();
                if(boost::regex_match(abspath,e)) {
                    png_ptr_type p(new png(abspath));
                    _images.push_back(p);
                }
            }
        }
    }
    
	template <typename Individual, typename RNG, typename EA>
	double operator()(Individual& ind, RNG& rng, EA& ea) {
        typedef sequence_matrix<png> matrix_type;
        typedef retina2_iterator<matrix_type> iterator_type;

        // get the phenotype (markov network):
        typename EA::phenotype_type &N = ealib::phenotype(ind, ea);
        int seed=rng.seed();
        
        // accumulated fitness:
        double w=0.0;
        
        // and analyze images...
        for(int i=0; i<get<EVOCADX_EXAMINE_N>(ea); ++i) {
            N.reset(seed);
            N.clear();
            
            matrix_type M(*_images[i]);
            iterator_type ci(M, get<EVOCADX_RETINA_SIZE>(ea), get<EVOCADX_RETINA_SIZE>(ea));
            
            // move camera to ~middle of the image:
            ci.position((M.size1()-get<EVOCADX_RETINA_SIZE>(ea)/2),
                        (M.size2()-get<EVOCADX_RETINA_SIZE>(ea)/2));            
            
            for(int j=0; j<get<EVOCADX_UPDATE_N>(ea); ++j) {
                N.update(ci);
                ci.move(algorithm::bits2ternary(N.begin_output()), algorithm::bits2ternary(N.begin_output()+2));
            }
            
            w += _images[i]->distance_to_centroid(ci._j, ci._i);
        }
        
        return 1.0 / (w + 1.0);
    }
    
    image_vector_type _images; //!< Vector of images loaded from disk.
};

// Evolutionary algorithm definition.
typedef markov_evolution_algorithm
< centroid_fitness
, recombination::asexual
, generational_models::moran_process< >
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
        add_option<EVOCADX_FILE_REGEX>(this);
        add_option<EVOCADX_EXAMINE_N>(this);
        add_option<EVOCADX_UPDATE_N>(this);
        add_option<EVOCADX_RETINA_SIZE>(this);
    }
    
    
    virtual void gather_tools() {
    }
    
    virtual void gather_events(EA& ea) {
        add_event<datafiles::fitness_dat>(ea);
    };
};
LIBEA_CMDLINE_INSTANCE(ea_type, cli);
