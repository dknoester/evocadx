/* png_centroid.cpp
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

#include "analysis.h"
#include "evocadx.h"
#include <evocadx/db/png.h>

typedef boost::shared_ptr<png> png_ptr_type;
typedef std::vector<png_ptr_type> image_vector_type;


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
            value_type threshold = get<EVOCADX_PIXEL_THRESHOLD>(ea);
            unsigned int downscalefact = get<EVOCADX_IMAGE_DOWNSCALE_FACTOR>(ea);
            png_ptr_type p(new png(*i, false, threshold, downscalefact)); // not weighted, threshold == 0 (implies calculate the threshold); this turns the image into black & white.
            _images.push_back(p);

            std::string imgdir = get<EVOCADX_DUMP_IMAGES_DIR>(ea);
            if (imgdir.length() > 0) {
              std::string wrkstr = (*i);
              std::string outfn = imgdir;
              if (outfn[outfn.length()-1] != '/') outfn += "/";
              outfn = outfn + basename((char*)(wrkstr.c_str()));
              int pos = outfn.rfind(".png");
              if (pos < outfn.length()) {
                outfn.replace(pos,4,".pgm");
              }

              if (!p->write_pgm(outfn)) {
                std::cerr << "*** ERROR: failed to dump image to " << outfn << std::endl;
                exit(-1);
              }
              //std::cout << outfn << std::endl;
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
            // normalize d by the length of the diagonal:
            d /= sqrt(_images[i]->width()*_images[i]->width() + _images[i]->height()*_images[i]->height());
            w += d;
        }
        
        return 1.0 / (w + 1.0);
    }
    
    image_vector_type _images; //!< Vector of images loaded from disk.
};

// Evolutionary algorithm definition.
typedef mkv::markov_network_evolution
< centroid_fitness
, recombination::asexual
, generational_models::moran_process<selection::proportionate< >, selection::rank>
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
        add_option<EVOCADX_DUMP_IMAGES_DIR>(this);
        add_option<EVOCADX_IMAGES_N>(this);
        add_option<EVOCADX_EXAMINE_N>(this);
        add_option<EVOCADX_FOVEA_SIZE>(this);
        add_option<EVOCADX_RETINA_SIZE>(this);
        add_option<EVOCADX_PIXEL_THRESHOLD>(this);
        add_option<EVOCADX_IMAGE_DOWNSCALE_FACTOR>(this);
    }
    
    virtual void gather_tools() {
        add_tool<evocadx_filenames>(this);
    }
    
    virtual void gather_events(EA& ea) {
        add_event<datafiles::fitness_dat>(ea);
        add_event<evocadx_shuffle_images>(ea);
    };
    
    virtual void before_initialization(EA& ea) {
        put<mkv::MKV_INPUT_N>(8*get<EVOCADX_RETINA_SIZE>(ea) + get<EVOCADX_FOVEA_SIZE>(ea)*get<EVOCADX_FOVEA_SIZE>(ea), ea);
        put<mkv::MKV_OUTPUT_N>(4, ea);
    }
};
LIBEA_CMDLINE_INSTANCE(ea_type, cli);
