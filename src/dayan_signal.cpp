/* dayan_signal.cpp
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
#include <mkv/markov_network_evolution.h>
#include <ea/generational_models/moran_process.h>
#include <ea/selection/rank.h>
#include <ea/cmdline_interface.h>
#include <ea/datafiles/fitness.h>
using namespace ealib;

#include "dayan.h"

typedef mkv::markov_network_evolution
< dayan_signal
, recombination::asexual
, generational_models::moran_process<selection::proportionate< >, selection::rank>
> ea_type;

LIBEA_CMDLINE_INSTANCE(ea_type, dayan_cli);
