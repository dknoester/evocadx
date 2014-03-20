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
#include "../src/png.h"

#define PNG1 "../testdata/test1.png"
#define PNG2 "../testdata/test2.png"
// #define PNG1 "/mnt/research/evocadx/testdata/test1.png"
// #define PNG2 "/mnt/research/evocadx/testdata/test2.png"

BOOST_AUTO_TEST_CASE(test_png1_size) {
    png image = png(PNG1, true, 0, 0);
    // image.write_pgm("test1.pgm");
    BOOST_CHECK_EQUAL(image.width(), 2669u);
    BOOST_CHECK_EQUAL(image.height(), 5114u);
    BOOST_CHECK_EQUAL(image.size(), 13649266u);
}

BOOST_AUTO_TEST_CASE(test_png2_size) {
    png image = png(PNG2, true, 0, 0);
    // image.write_pgm("test2.pgm");  
    BOOST_CHECK_EQUAL(image.width(), 2444u);
    BOOST_CHECK_EQUAL(image.height(), 4574u);
    BOOST_CHECK_EQUAL(image.size(), 11178856u);
}

BOOST_AUTO_TEST_CASE(test_png1_pixels_weighted) {
    png image = png(PNG1, true, 0, 0);
    BOOST_CHECK_EQUAL(image[0], 11436u);
    BOOST_CHECK_EQUAL(image[image.width()*image.height() - 1], 22301u);
}

BOOST_AUTO_TEST_CASE(test_png2_pixels_weighted) {
    png image = png(PNG2, true, 0, 0);
    BOOST_CHECK_EQUAL(image[0], 50589u);
    BOOST_CHECK_EQUAL(image[image.width()*image.height() - 1], 2962u);
}

BOOST_AUTO_TEST_CASE(test_png1_pixels_unweighted) {
    png image = png(PNG1, false, 0, 0);
    BOOST_CHECK_EQUAL(image[0], 0u);
    BOOST_CHECK_EQUAL(image[image.width()*image.height() - 1], 0u);
}

BOOST_AUTO_TEST_CASE(test_png2_pixels_unweighted) {
    png image = png(PNG2, false, 0, 0);
    BOOST_CHECK_EQUAL(image[0], std::numeric_limits<png::value_type>::max());
    BOOST_CHECK_EQUAL(image[image.width()*image.height() - 1], 0u);
}

BOOST_AUTO_TEST_CASE(test_png1_centroid_thresh) {
    png image = png(PNG1, false, 0, 0);
    png::centroid_type cent = image.get_centroid();
    BOOST_CHECK_CLOSE(cent.first, 1894.13, 0.01);
    BOOST_CHECK_CLOSE(cent.second, 2431.93, 0.01);
    BOOST_CHECK_CLOSE(image.distance_to_centroid(1480,1100), 1394.826578, 0.01);
}

BOOST_AUTO_TEST_CASE(test_png2_centroid_thresh) {
    png image = png(PNG2, false, 0, 0);
    png::centroid_type cent = image.get_centroid();
    BOOST_CHECK_CLOSE(cent.first, 698.737, 0.01);
    BOOST_CHECK_CLOSE(cent.second, 2268.53, 0.01);
    BOOST_CHECK_CLOSE(image.distance_to_centroid(1200,1600), 835.581807, 0.01);
}

BOOST_AUTO_TEST_CASE(test_png1_centroid_downscale) {
    png image = png(PNG1, false, 0, 4);
    BOOST_CHECK_EQUAL(image.width(), 667u);
    BOOST_CHECK_EQUAL(image.height(), 1278u);
    png::centroid_type cent = image.get_centroid();
    BOOST_CHECK_CLOSE(cent.first, 473.025, 0.01);
    BOOST_CHECK_CLOSE(cent.second, 607.872, 0.01);
    BOOST_CHECK_CLOSE(image.distance_to_centroid(201,305), 407.098328, 0.01);
}

BOOST_AUTO_TEST_CASE(test_png2_centroid_downscale) {
    png image = png(PNG2, false, 0, 4);
    BOOST_CHECK_EQUAL(image.width(), 611u);
    BOOST_CHECK_EQUAL(image.height(), 1143u);
    png::centroid_type cent = image.get_centroid();
    BOOST_CHECK_CLOSE(cent.first, 174.253, 0.01);
    BOOST_CHECK_CLOSE(cent.second, 566.591, 0.01);
    BOOST_CHECK_CLOSE(image.distance_to_centroid(400,808), 330.514770, 0.01);
}

