/* dayan.h
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
#ifndef _DAYAN_H_
#define _DAYAN_H_

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/math/constants/constants.hpp>

#include <ea/fitness_function.h>

LIBEA_MD_DECL(EVOCADX_DAYAN_ALPHA, "evocadx.dayan.alpha", double);
LIBEA_MD_DECL(EVOCADX_DAYAN_BETA, "evocadx.dayan.beta", double);
LIBEA_MD_DECL(EVOCADX_DAYAN_RN, "evocadx.dayan.rn", double);
LIBEA_MD_DECL(EVOCADX_DAYAN_PEAK, "evocadx.dayan.peak", double);


struct action {
    enum action_type {L,R,C,N};
};


struct dayan_output {
    template <typename MarkovNetwork>
    dayan_output(MarkovNetwork& net) {
        L = static_cast<double>(algorithm::range_pair2int(net.begin_output(), net.begin_output()+2));
        R = static_cast<double>(algorithm::range_pair2int(net.begin_output()+2, net.begin_output()+4));
        C = static_cast<double>(algorithm::range_pair2int(net.begin_output()+4, net.begin_output()+6));
    }
    int L,R,C;
};


/*! MDP fitness function based on Dayan / Dawes paper.
 L    R
 alpha
 s0  s1
 
 1
 s2  s3
 
 */
struct dayan_fitness : fitness_function<unary_fitness<double>, constantS, stochasticS> {
    typedef boost::numeric::ublas::matrix<double> cue_matrix_type; //!< Type for matrix that will store discretized cue distributions.
    
    //! Constructor.
    dayan_fitness() {
        rL.resize(4,0.0); rR.resize(4,0.0); rC.resize(4,0.0); px1.resize(4,0.0); rN.resize(4,0.0);
        pc.resize(4,32);
        pc = boost::numeric::ublas::scalar_matrix<double>(4,32,0.0);
    }
    
    //! Initialize this fitness function.
    template <typename RNG, typename EA>
    void initialize(RNG& rng, EA& ea) {
        // L is always good in states 1 & 3:
        rL[0] = get<EVOCADX_DAYAN_ALPHA>(ea);
        rL[1] = 0;
        rL[2] = 1;
        rL[3] = 0;
        
        rR[0] = 0;
        rR[1] = get<EVOCADX_DAYAN_ALPHA>(ea);
        rR[2] = 0;
        rR[3] = 1;
        
        rC[2] = -0.5;
        rC[3] = -0.5;
        
        rN[0] = get<EVOCADX_DAYAN_RN>(ea);
        rN[1] = get<EVOCADX_DAYAN_RN>(ea);
        rN[2] = get<EVOCADX_DAYAN_RN>(ea);
        rN[3] = get<EVOCADX_DAYAN_RN>(ea);
        
        px1[2] = get<EVOCADX_DAYAN_BETA>(ea);
        px1[3] = 1.0 - get<EVOCADX_DAYAN_BETA>(ea);
    }
    
    //! Calculate fitness of an individual.
    template <typename Individual, typename RNG, typename EA>
    double operator()(Individual& ind, RNG& rng, EA& ea) {
        typename EA::phenotype_type &N = ealib::phenotype(ind, ea);
        
        if(N.ngates() == 0) {
            return 0.0;
        }
        
        // initial conditions:
        double w=0.0;
        std::vector<int> inputs(pc.size2());
        state = rng(il,iu);
        N.clear();
        
        // for each "cpu cycle":
        for(int i=0; i<100; ++i) {
            for(std::size_t j=0; j<pc.size2(); ++j) {
                inputs[j] = rng.p(pc(state,j));
            }
            
            N.update(inputs.begin());
            dayan_output output(N);

            // if the action is invalid given the current state,
            // or multiple actions are selected, apply the rN reward:
            if(((output.L + output.R + output.C) != 1)
               || (output.C && (state<2))) {
                w += rN[state];
                continue;
            }

            if(output.C) {
                w += rC[state];
                // probabilistic transition to x1:
                if(rng.p(px1[state])) {
                    state = 0;
                } else {
                    state = 1;
                }
            } else {
                if(output.L) {
                    w += rL[state];
                } else {
                    w += rR[state];
                }
                // restart the game:
                state = rng(il,iu);
                N.clear();
            }
        }
        
        return std::max(1.0, w);
    }
    
    int state; // current state; [0..3].
    int il,iu; // initial state lower and upper bounds.
    std::vector<double> rL, rR, rC; // rewards for actions in state i.
    std::vector<double> rN; // reward for no action in state i.
    std::vector<double> px1; // probability to transition to state 1 from state i.
    cue_matrix_type pc;
};

//! MDP problem from Dayan / Daw.
struct dayan_mdp : dayan_fitness {
    //! Initialize this fitness function.
    template <typename RNG, typename EA>
    void initialize(RNG& rng, EA& ea) {
        dayan_fitness::initialize(rng,ea);
        
        // cue probability distributions:
        for(std::size_t i=0; i<pc.size1(); ++i) {
            for(std::size_t j=0; j<pc.size2(); ++j) {
                if((j/8)==i) {
                    pc(i,j) = 1.0;
                }
            }
        }
        
        // initial state:
        il=2; iu=4;
    }
};

//! Returns the probability of x given a normal distribution N(sigma,mu).
double normal_pdf(double x, double mu, double sigma) {
    return (1.0/(sigma*sqrt(2.0*boost::math::constants::pi<double>()))) * exp(-((x-mu)*(x-mu))/(2.0*sigma*sigma));
}

//! Signal detection problem from Dayan/Daw.
struct dayan_signal : dayan_fitness {
    //! Initialize this fitness function.
    template <typename RNG, typename EA>
    void initialize(RNG& rng, EA& ea) {
        dayan_fitness::initialize(rng,ea);

        // cue probability distributions:
        for(std::size_t i=0; i<pc.size2(); ++i) {
            pc(0,i) = normal_pdf(static_cast<double>(i), 0.0+get<EVOCADX_DAYAN_PEAK>(ea), 4.0);
            pc(1,i) = normal_pdf(static_cast<double>(i), 31.0-get<EVOCADX_DAYAN_PEAK>(ea), 4.0);
        }
        
        px1[2] = 0.0;
        px1[3] = 1.0;

        // initial state:
        il=0; iu=2;
    }
};

//! Temporal state uncertainty problem from Dayan/Daw.
struct dayan_temporal : dayan_fitness {
    //! Initialize this fitness function.
    template <typename RNG, typename EA>
    void initialize(RNG& rng, EA& ea) {
        dayan_fitness::initialize(rng,ea);
        
        // cue probability distributions:
        for(std::size_t i=0; i<pc.size2(); ++i) {
            pc(0,i) = normal_pdf(static_cast<double>(i), 0.0+get<EVOCADX_DAYAN_PEAK>(ea), 2.0);
            pc(1,i) = normal_pdf(static_cast<double>(i), 31.0-get<EVOCADX_DAYAN_PEAK>(ea), 2.0);
            pc(2,i) = normal_pdf(static_cast<double>(i), 0.0+get<EVOCADX_DAYAN_PEAK>(ea), 4.0);
            pc(3,i) = normal_pdf(static_cast<double>(i), 31.0-get<EVOCADX_DAYAN_PEAK>(ea), 4.0);
        }
        
        // initial state:
        il=2; iu=4;
    }
};


/*! Define the EA's command-line interface.
 */
template <typename EA>
class dayan_cli : public cmdline_interface<EA> {
public:
    virtual void gather_options() {
        mkv::add_options(this);
        
        add_option<MORAN_REPLACEMENT_RATE_P>(this);
        add_option<POPULATION_SIZE>(this);
        add_option<RUN_UPDATES>(this);
        add_option<RUN_EPOCHS>(this);
        add_option<CHECKPOINT_PREFIX>(this);
        add_option<RNG_SEED>(this);
        add_option<RECORDING_PERIOD>(this);
        
        add_option<EVOCADX_DAYAN_ALPHA>(this);
        add_option<EVOCADX_DAYAN_BETA>(this);
        add_option<EVOCADX_DAYAN_RN>(this);
        add_option<EVOCADX_DAYAN_PEAK>(this);
    }
    
    virtual void gather_tools() {
        add_tool<mkv::dominant_reduced_graph>(this);
        add_tool<mkv::dominant_causal_graph>(this);
        add_tool<mkv::dominant_genetic_graph>(this);
    }
    
    virtual void gather_events(EA& ea) {
        add_event<datafiles::fitness_dat>(ea);
    }
};

#endif
