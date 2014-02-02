/* test_png.cpp
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
#ifndef BOOST_TEST_DYN_LINK
#define BOOST_TEST_DYN_LINK
#endif
#define BOOST_TEST_MAIN
#include "test.h"
#include "png.h"

BOOST_AUTO_TEST_CASE(test_png1) {
    png image = png("/mnt/research/evocadx/testdata/test1.png", true, 1000);
    BOOST_CHECK_EQUAL(image.width(), 256u);
    BOOST_CHECK_EQUAL(image.height(), 80u);
    BOOST_CHECK_EQUAL(image.size(), 20480u);
    BOOST_CHECK_EQUAL(image[0], 0);
    BOOST_CHECK_EQUAL(image[image.width()*image.height() - 1], 65280);
    BOOST_CHECK_CLOSE(image.distance_to_centroid(1480,1100), 1752.684, 0.01);
}

BOOST_AUTO_TEST_CASE(test_png2) {
    png image = png("/mnt/research/evocadx/testdata/test2.png", true, 1000);
    BOOST_CHECK_EQUAL(image.width(), 1914u);
    BOOST_CHECK_EQUAL(image.height(), 2294u);
    BOOST_CHECK_EQUAL(image.size(), 4390716u);
    BOOST_CHECK_EQUAL(image[0], 0);
    BOOST_CHECK_EQUAL(image[image.width()*image.height() - 1], 0);
    BOOST_CHECK_CLOSE(image.distance_to_centroid(1480,1100), 1844.017, 0.01);
}
