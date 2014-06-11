/* analysis.h
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
#ifndef _ANALYSIS_H_
#define _ANALYSIS_H_

#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <deque>
#include <map>
#include <set>
#include <vector>

#include <ea/analysis.h>
#include <mkv/graph.h>
using namespace ealib;

#include "aid.h"

/*! Analyze fitness and accuracy for the dominants from each subpopulation.
 */
template <typename EA>
struct aid_examine : public ealib::analysis::unary_function<EA> {
    typedef typename EA::individual_type::fitness_function_type fitness_function_type;
    typedef std::vector<int> rb_type;
    
    static const char* name() { return "aid_examine"; }
    
    //! Run the analysis.
    virtual void operator()(EA& ea) {
        using namespace ealib;
        using namespace ealib::analysis;
        using namespace boost::accumulators;
        
        datafile df("aid_examine.dat");
        
        df.add_field("update").add_field("subpopulation")
        .add_field("dominant_fitness")
        .add_field("training_fitness")
        .add_field("training_accuracy")
        .add_field("testing_fitness")
        .add_field("testing_accuracy");
        
        accumulator_set<double, stats<tag::mean, tag::max> > trfit, tefit;
        accumulator_set<double, stats<tag::mean, tag::max> > tracc, teacc;
        
        std::size_t sp=0;
        for(typename EA::iterator i=ea.begin(); i!=ea.end(); ++i, ++sp) {
            typename EA::subindividual_type& ind=analysis::find_dominant(*i);
            
            df.write(ea.current_update()).write(sp).write(static_cast<double>(ealib::fitness(ind,*i)));
            
            result_matrix M;
            i->fitness_function().train(ind, M, *i);
            df.write(i->fitness_function().fitness(M,*i));
            df.write(i->fitness_function().accuracy(M));
            
            datafile dftr("aid_examine_training_" + boost::lexical_cast<std::string>(sp) + ".dat");
            dftr.add_field("image").add_field("label").add_field("decision");
            for(std::size_t j=0; j<M.size1(); ++j) {
                dftr.write(j).write(M(j,0)).write(M(j,1)).endl();
            }
            
            result_matrix R;
            i->fitness_function().test(ind, R, *i);
            df.write(i->fitness_function().fitness(R,*i));
            df.write(i->fitness_function().accuracy(R)).endl();

            datafile dfte("aid_examine_testing_" + boost::lexical_cast<std::string>(sp) + ".dat");
            dfte.add_field("image").add_field("label").add_field("decision");
            for(std::size_t j=0; j<R.size1(); ++j) {
                dfte.write(j).write(R(j,0)).write(R(j,1)).endl();
            }
        }
    }
};


/*! Datafile that records mean & max fitness and accuracy, over the entire
 training and testing datasets, for the dominant individual from each subpopulation.
 */
template <typename EA>
struct aid_progress : record_statistics_event<EA> {
    aid_progress(EA& ea)
    : record_statistics_event<EA>(ea)
    , _train_fitness("aid_train_fitness.dat")
    , _train_accuracy("aid_train_accuracy.dat")
    , _test_fitness("aid_test_fitness.dat")
    , _test_accuracy("aid_test_accuracy.dat") {
        
        _train_fitness.add_field("update").add_field("max_fitness").add_field("mean_fitness");
        _train_accuracy.add_field("update").add_field("max_accuracy").add_field("mean_accuracy");
        _test_fitness.add_field("update").add_field("max_fitness").add_field("mean_fitness");
        _test_accuracy.add_field("update").add_field("max_accuracy").add_field("mean_accuracy");
    }
    
    virtual ~aid_progress() {
    }
    
    virtual void operator()(EA& ea) {
        using namespace boost::accumulators;
        
        accumulator_set<double, stats<tag::mean, tag::max> > trfit, tefit;
        accumulator_set<double, stats<tag::mean, tag::max> > tracc, teacc;
        
        for(typename EA::iterator i=ea.begin(); i!=ea.end(); ++i) {
            typename EA::subindividual_type& ind=analysis::find_dominant(*i);
            
            result_matrix M;
            i->fitness_function().train(ind, M, *i);
            trfit(i->fitness_function().fitness(M));
            tracc(i->fitness_function().accuracy(M));
            
            result_matrix R;
            i->fitness_function().test(ind, R, *i);
            tefit(i->fitness_function().fitness(R));
            teacc(i->fitness_function().accuracy(R));
        }
        
        _train_fitness.write(ea.current_update()).write(max(trfit)).write(mean(trfit)).endl();
        _train_accuracy.write(ea.current_update()).write(max(tracc)).write(mean(tracc)).endl();
        _test_fitness.write(ea.current_update()).write(max(tefit)).write(mean(tefit)).endl();
        _test_accuracy.write(ea.current_update()).write(max(teacc)).write(mean(teacc)).endl();
    }
    
    datafile _train_fitness, _train_accuracy, _test_fitness, _test_accuracy;
};


/*! Analyze per-network update entropies.
 */
template <typename EA>
struct aid_entropy : public ealib::analysis::unary_function<EA>, aid_fitness_callback {
    static const char* name() { return "aid_entropy"; }

    virtual void initialize(EA& ea) {
        D.resize(data::instance()->training.dim(0),
                 result_matrix(data::instance()->training.records().size(),5));
    }

    
    // image #, image row, image class, row data, v1, v2 in, v2 out
    virtual void row(int i, int j, int c, ivector_type& row, result_matrix& v1_input, ivector_type& v2_input, ivector_type& v2_output) {
        D[j](i,0) = c; // world (w)
        D[j](i,1) = algorithm::range2bits<long>(row.begin(), row.end());
        D[j](i,2) = algorithm::range2bits<long>(v1_input.data().begin(), v1_input.data().end());
        D[j](i,3) = algorithm::range2bits<long>(v2_input.begin(), v2_input.end()); // input to v2 from v1 (v1)
        D[j](i,4) = algorithm::range2bits<long>(v2_output.begin(), v2_output.end()); // output from v2 (v2)
    }
    
    //! Run the analysis.
    virtual void operator()(EA& ea) {
        using namespace ealib;
        using namespace ealib::analysis;
        using namespace boost::accumulators;
        
        datafile df("aid_entropy.dat");
        
        df.add_field("update").add_field("subpopulation")
        .add_field("row")
        .add_field("Iwv2")
        .add_field("Iwv1")
        .add_field("Iv1v2")
        .add_field("Iwv1v2");
        
        std::size_t sp=0;
        for(typename EA::iterator i=ea.begin(); i!=ea.end(); ++i, ++sp) {
            typename EA::subindividual_type& ind=analysis::find_dominant(*i);
            
            result_matrix M, Mi;
            i->fitness_function().train(ind, M, *i, this);
            
            for(std::size_t j=0; j<D.size(); ++j) {
                df.write(ea.current_update())
                .write(sp)
                .write(j)
                .write(analysis::mutual_information(result_column(D[j],0), result_column(D[j],4)))
                .write(analysis::mutual_information(result_column(D[j],0), result_column(D[j],3)))
                .write(analysis::mutual_information(result_column(D[j],3), result_column(D[j],4)))
                .write(analysis::multivariate_information(result_column(D[j],0), result_column(D[j],3), result_column(D[j],4)))
                .endl();
            }
        }
    }
    
    std::vector<result_matrix> D;
};

#endif
