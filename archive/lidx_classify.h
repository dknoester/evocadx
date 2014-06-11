/* mkv_retina.h
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
#ifndef _MKV_RETINA_H_
#define _MKV_RETINA_H_

#include <ea/fitness_function.h>
#include <ea/analysis/information.h>
#include <ea/algorithm.h>

#include <mkv/matrix.h>
#include <mkv/camera.h>
using namespace ealib;

#include "aid.h"

// retina characteristics:
LIBEA_MD_DECL(AID_RETINA_WIDTH, "aid.retina.width", double);
LIBEA_MD_DECL(AID_RETINA_HEIGHT, "aid.retina.height", double);
LIBEA_MD_DECL(AID_RETINA_SIZE, "aid.retina.size", double);
LIBEA_MD_DECL(AID_RETINA_RINGS, "aid.retina.rings", double);

//! MKV camera-based fitness function.
struct mkv_retina : fitness_function<unary_fitness<double>, constantS, stochasticS> {
    
    //! Classify the range [f,l) of images from db using the individual.
    template <typename Individual, typename DB, typename RNG, typename EA>
    void classify(std::size_t f, std::size_t l, DB& db, Individual& ind, result_matrix& M, RNG& rng, EA& ea, aid_fitness_callback* cb=0) {
        typedef mkv::sequence_matrix<typename DB::record_type::vector_type> image_matrix_type;
        typedef mkv::retina2_iterator<image_matrix_type> camera_iterator_type;
        
        int seed=rng.seed();
        mkv::markov_network& net = ealib::phenotype(ind,rng,ea);
        M.resize(l-f, 2);
        
        std::size_t updates=get<mkv::MKV_UPDATE_N>(ea);
        
        for(std::size_t i=f; i<l; ++i) {
            typename DB::record_type& r = db[i];
            
            image_matrix_type I(r.data, db.dim(0), db.dim(1));
            
            camera_iterator_type ci(I, get<AID_RETINA_SIZE>(ea), get<AID_RETINA_RINGS>(ea));
            ci.position((db.dim(0)-get<AID_RETINA_SIZE>(ea))/2, (db.dim(1)-get<AID_RETINA_SIZE>(ea))/2);
            net.reset(seed);
            
            for(std::size_t j=0; j<updates; ++j) {
                mkv::update(net, 1, ci);
                ci.move(algorithm::bits2ternary(net.begin_output()), algorithm::bits2ternary(net.begin_output()+2));
            }
            
            // The +1's below are to account for building confusion matrices from
            // M (avoiding negative-valued indices).
            M(i-f, 0) = r.label+1; // predicted
            
            int d=0;
            std::vector<int> decisions;
            algorithm::range_pair2indices(net.begin_output()+4, net.end_output(), std::back_inserter(decisions));
            // for this go-round, allow exact solutions only:
            if((decisions.size() == 1) && (decisions[0] == r.label)) {
                d = r.label+1;
            }
            M(i-f, 1) = d; // actual
        }
    }
    
    //! Convenience method, run the network through all the training data.
    template <typename Individual, typename EA>
    void train(Individual& ind, result_matrix& M, EA& ea, aid_fitness_callback* cb=0) {
        classify(0, data::instance()->training.records().size(), data::instance()->training, ind, M, ea.rng(), ea, cb);
    }
    
    //! Convenience method, run the network through all the testing data.
    template <typename Individual, typename EA>
    void test(Individual& ind, result_matrix& M, EA& ea, aid_fitness_callback* cb=0) {
        classify(0, data::instance()->testing.records().size(), data::instance()->testing, ind, M, ea.rng(), ea, cb);
    }
    
    //! Calculate fitness from a result matrix.
    template <typename EA>
    double fitness(result_matrix& M, EA& ea) {
        //        return 1.0 / (1.0 + analysis::information_distance(M));
        //        return exp(analysis::mutual_information(M));
        return 100.0/(1.0 + analysis::sum_squared_error(result_column(M,0), result_column(M,1), get<AID_NCLASSES>(ea)+1));
    }
    
    //! Calculate the accuracy of a result matrix.
    double accuracy(result_matrix& M) {
        assert(M.size1() > 0);
        double a=0.0;
        for(std::size_t i=0; i<M.size1(); ++i) {
            if(M(i,0) == M(i,1)) {
                a += 1.0;
            }
        }
        return a/static_cast<double>(M.size1());
    }
    
    //! Calculate fitness of ind.
	template <typename Individual, typename RNG, typename EA>
	double operator()(Individual& ind, RNG& rng, EA& ea) {
        result_matrix M;
        classify(get<AID_FIRST>(ea), get<AID_LAST>(ea), data::instance()->training, ind, M, rng, ea);
        return fitness(M,ea);
    }
};


//! MKV camera-based fitness function.
struct inverted_mkv_retina : fitness_function<unary_fitness<double>, constantS, stochasticS> {
    
    //! Decode the output from the inverted MKV network.
    template <typename ForwardIterator, typename OutputIterator>
    std::pair<bool,int> decode_output(ForwardIterator f, ForwardIterator l, OutputIterator oi) {
        assert((std::distance(f,l)%2) == 0);
        std::pair<bool,int> r(false,-1);
        for(int i=0; f!=l; ++f, ++i, ++oi) {
            int excite=(*f & 0x01);
            int inhibit=(*++f & 0x01);
            if(excite & ~inhibit) {
                *oi = *oi & i;
            }
        }
    }
    
    //! Classify the range [f,l) of images from db using the individual.
    template <typename Individual, typename DB, typename RNG, typename EA>
    void classify(std::size_t f, std::size_t l, DB& db, Individual& ind, std::vector<int>& M, RNG& rng, EA& ea, aid_fitness_callback* cb=0) {
        typedef mkv::sequence_matrix<typename DB::record_type::vector_type> image_matrix_type;
        typedef mkv::retina2_iterator<image_matrix_type> camera_iterator_type;
        
        int seed=rng.seed();
        mkv::markov_network& net = ealib::phenotype(ind,rng,ea);
        M.clear();
        
        std::size_t i=f;
        std::size_t updates=get<mkv::MKV_UPDATE_N>(ea);
        
        while((updates > 0) && (i<l)) {
            typename DB::record_type& r = db[i];
            image_matrix_type I(r.data, db.dim(0), db.dim(1));
            camera_iterator_type ci(I, get<AID_RETINA_SIZE>(ea), get<AID_RETINA_RINGS>(ea));
            ci.position((db.dim(0)-get<AID_RETINA_SIZE>(ea))/2, (db.dim(1)-get<AID_RETINA_SIZE>(ea))/2);
            net.reset(seed);
            std::vector<int> decisions(get<AID_NCLASSES>(ea),1);
            
            for( ; updates>0; --updates) {
                mkv::update(net, 1, ci);
                ci.move(algorithm::bits2ternary(net.begin_output()), algorithm::bits2ternary(net.begin_output()+2));
                
                // if output == 1, then turn off this guess, and latch it.
                // go to the next image when only one guess remains.
                int num_decisions=0;
                int last_decision=-1;
                for(std::size_t j=0; j<get<AID_NCLASSES>(ea); ++j) {
                    // inverting the signal here; they must assert to turn off a label,
                    // and there's no going back:
                    decisions[j] = decisions[j] && !((net.output(j*2) & 0x01) && !(net.output(j*2+1) & 0x01));
                    if(decisions[j]) {
                        ++num_decisions;
                        last_decision = j;
                    }
                }
                
                if(num_decisions <= 1) {
                    if(r.label == last_decision) {
                        M.push_back(i); // got this one right.
                    }
                    ++i; // next image
                    break;
                }
                // else continue evaluating this image.
            }
        }
    }
    
    //! Convenience method, run the network through all the training data.
    template <typename Individual, typename EA>
    void train(Individual& ind, result_matrix& M, EA& ea, aid_fitness_callback* cb=0) {
//        classify(0, data::instance()->training.records().size(), data::instance()->training, ind, M, ea.rng(), ea, cb);
    }
    
    //! Convenience method, run the network through all the testing data.
    template <typename Individual, typename EA>
    void test(Individual& ind, result_matrix& M, EA& ea, aid_fitness_callback* cb=0) {
//        classify(0, data::instance()->testing.records().size(), data::instance()->testing, ind, M, ea.rng(), ea, cb);
    }
    
    //! Calculate fitness from a result matrix.
    template <typename EA>
    double fitness(result_matrix& M, EA& ea) {
        //        return 1.0 / (1.0 + analysis::information_distance(M));
        //        return exp(analysis::mutual_information(M));
        return 1.0;//00.0/(1.0 + analysis::sum_squared_error(result_column(M,0), result_column(M,1), get<AID_NCLASSES>(ea)+1));
    }
    
    //! Calculate the accuracy of a result matrix.
    double accuracy(result_matrix& M) {
        assert(M.size1() > 0);
        double a=0.0;
        for(std::size_t i=0; i<M.size1(); ++i) {
            if(M(i,0) == M(i,1)) {
                a += 1.0;
            }
        }
        return a/static_cast<double>(M.size1());
    }
    
    //! Calculate fitness of ind.
	template <typename Individual, typename RNG, typename EA>
	double operator()(Individual& ind, RNG& rng, EA& ea) {
        std::vector<int> M;
        classify(get<AID_FIRST>(ea), get<AID_LAST>(ea), data::instance()->training, ind, M, rng, ea);
        return M.size();
    }
};

/*! Analyze per-image retina paths.
 */
template <typename EA>
struct aid_retina : public ealib::analysis::unary_function<EA>, aid_fitness_callback {
    static const char* name() { return "aid_retina"; }
    
    aid_retina() : _df("aid_retina.dat"), _db("aid_db.dat") { }
    
    virtual void initialize(EA& ea) {
        _df.add_field("update").add_field("subpopulation")
        .add_field("image")
        .add_field("class")
        .add_field("row")
        .add_field("col")
        .add_field("depth")
        .add_field("repr");
        for(int i=0; i<(get<AID_RETINA_HEIGHT>(ea)*get<AID_RETINA_WIDTH>(ea)); ++i) {
            _df.add_field("p" + boost::lexical_cast<std::string>(i));
        }
    }
    
    // data record
    virtual void data(ivector_type& d) {
        _db.write_all(d.begin(), d.end()).endl();
    }
    
    // image #, image class, retina i, retina j, output
    virtual void retina(int i, int c, int ri, int rj, int rk, ivector_type& input, ivector_type& output) {
        _df.write(_update)
        .write(_sp)
        .write(i).write(c)
        .write(ri).write(rj).write(rk)
        .write(algorithm::range_pair2int(output.begin()+6, output.end()))
        .write_all(input.begin(), input.end())
        .endl();
    }
    
    //! Run the analysis.
    virtual void operator()(EA& ea) {
        using namespace ealib;
        using namespace ealib::analysis;
        using namespace boost::accumulators;
        
        _sp=0;
        _update = ea.current_update();
        for(typename EA::iterator i=ea.begin(); _sp<1; ++i, ++_sp) {
            typename EA::subindividual_type& ind=analysis::find_dominant(*i);
            
            result_matrix M, Mi;
            i->fitness_function().train(ind, M, *i, this);
        }
    }
    
    datafile _df, _db;
    int _update, _sp;
};


#endif
