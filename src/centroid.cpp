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
#include <algorithm>
#include <iterator>
#include <vector>
#include <ea/mkv/markov_evolution_algorithm.h>
#include <ea/mkv/sequence_matrix.h>
#include <ea/mkv/camera.h>
#include <ea/generational_models/moran_process.h>
#include <ea/selection/elitism.h>
#include <ea/fitness_function.h>
#include <ea/cmdline_interface.h>
#include <ea/datafiles/fitness.h>
using namespace ealib;

#include "analysis.h"
#include "evocadx.h"
#include "png.h"

typedef boost::shared_ptr<png> png_ptr_type;
typedef std::vector<png_ptr_type> image_vector_type;
typedef std::vector<std::string> filename_vector_type;

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

//! Analysis function that prints the list of image files that will be loaded.
template <typename EA>
struct evocadx_filenames : public ealib::analysis::unary_function<EA> {
    static const char* name() { return "evocadx_filenames"; }
    virtual ~evocadx_filenames() { }
    virtual void operator()(EA& ea) {
        std::cout << "Matching filenames:" << std::endl << "\t";
        filename_vector_type filenames = find_files(get<EVOCADX_DATADIR>(ea), get<EVOCADX_FILE_REGEX>(ea));
        std::copy(filenames.begin(), filenames.end(), std::ostream_iterator<std::string>(std::cout, "\n\t"));
    }
};


/*! Image centroid fitness function for Markov networks.
 */
struct centroid_fitness : fitness_function<unary_fitness<double>, constantS, stochasticS> {
    
    //! Initializes this fitness function by loading the image files.
    template <typename RNG, typename EA>
    void initialize(RNG& rng, EA& ea) {
        filename_vector_type filenames = find_files(get<EVOCADX_DATADIR>(ea), get<EVOCADX_FILE_REGEX>(ea));
        std::random_shuffle(filenames.begin(), filenames.end(), ea.rng());
        int count=0;
        for(filename_vector_type::iterator i=filenames.begin(); i!=filenames.end() && (count < get<EVOCADX_IMAGES_N>(ea)); ++i, ++count) {
            png_ptr_type p(new png(*i, false, 1000)); // not weighted, threshold == 1000; this turns the image into black & white.
            _images.push_back(p);
        }
    }
    
	template <typename Individual, typename RNG, typename EA>
	double operator()(Individual& ind, RNG& rng, EA& ea) {
        typedef sequence_matrix<png> matrix_type;
        typedef retina2_iterator<matrix_type> iterator_type;

        // get the phenotype (markov network):
        typename EA::phenotype_type &N = ealib::phenotype(ind, ea);
        int seed=rng.seed();
        
        // empty network guard:
        if(N.ngates() == 0) {
            return 0.0;
        }
        
        double w=0.0; // accumulated fitness
        
        // and analyze images...
        for(int i=0; i<get<EVOCADX_EXAMINE_N>(ea); ++i) {
            N.reset(seed);
            N.clear();
            
            matrix_type M(*_images[i]);
            iterator_type ci(M, get<EVOCADX_FOVEA_SIZE>(ea), get<EVOCADX_RETINA_SIZE>(ea));
            
            // move camera to ~middle of the image:
            ci.position(M.size1()/2, M.size2()/2);
            
            int updates = std::max(_images[i]->width(), _images[i]->height());
            
            for(int j=0; j<updates; ++j) {
                N.update(ci);
                ci.move(algorithm::bits2ternary(N.begin_output()), algorithm::bits2ternary(N.begin_output()+2));
            }
            
            double d = _images[i]->distance_to_centroid(ci._j, ci._i);
            w += d;
        }
        
        return 1.0 / (w + 1.0);
    }
    
    image_vector_type _images; //!< Vector of images loaded from disk.
};

// Evolutionary algorithm definition.
typedef markov_evolution_algorithm
< centroid_fitness
, recombination::asexual
, generational_models::moran_process<selection::proportionate< >, selection::elitism<selection::random> >
> ea_type;

//! Randomly shuffles the list of images at the end of every update.
template <typename EA>
struct evocadx_shuffle_images : end_of_update_event<EA> {
    evocadx_shuffle_images(EA& ea) : end_of_update_event<EA>(ea) { }
    virtual ~evocadx_shuffle_images() { }
    
    virtual void operator()(EA& ea) {
        std::random_shuffle(ea.fitness_function()._images.begin(),
                            ea.fitness_function()._images.end(),
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
        add_option<ELITISM_N>(this);

        add_option<EVOCADX_DATADIR>(this);
        add_option<EVOCADX_FILE_REGEX>(this);
        add_option<EVOCADX_IMAGES_N>(this);
        add_option<EVOCADX_EXAMINE_N>(this);
        add_option<EVOCADX_FOVEA_SIZE>(this);
        add_option<EVOCADX_RETINA_SIZE>(this);
    }
    
    virtual void gather_tools() {
        add_tool<evocadx_filenames>(this);
    }
    
    virtual void gather_events(EA& ea) {
        add_event<datafiles::fitness_dat>(ea);
        add_event<evocadx_shuffle_images>(ea);
    };
    
    virtual void before_initialization(EA& ea) {
        using namespace ealib::mkv;
        put<MKV_INPUT_N>(8*get<EVOCADX_RETINA_SIZE>(ea) + get<EVOCADX_FOVEA_SIZE>(ea)*get<EVOCADX_FOVEA_SIZE>(ea), ea);
        
        ea.config().disable(PROBABILISTIC);
        ea.config().disable(ADAPTIVE);
    }
};
LIBEA_CMDLINE_INSTANCE(ea_type, cli);
