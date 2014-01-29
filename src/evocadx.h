/* analysis.h
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
#ifndef _EVOCADX_H_
#define _EVOCADX_H_

#include <ea/meta_data.h>

LIBEA_MD_DECL(EVOCADX_DATADIR, "evocadx.data_directory", std::string);
LIBEA_MD_DECL(EVOCADX_FILE_REGEX, "evocadx.file_regex", std::string);
LIBEA_MD_DECL(EVOCADX_EXAMINE_N, "evocadx.examine_n", int);
LIBEA_MD_DECL(EVOCADX_UPDATE_N, "evocadx.update_n", int);
LIBEA_MD_DECL(EVOCADX_RETINA_SIZE, "evocadx.retina_size", std::size_t);

#endif
