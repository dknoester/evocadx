    /* lidx.h
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
#ifndef _LIDX_H_
#define _LIDX_H_

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/regex.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stdint.h>


namespace lidx {

    //! Single record in a lidx database.
    template <typename Label, typename Data>
    struct lidx_record {
        typedef Label label_type;
        typedef Data data_type;
        typedef std::vector<data_type> vector_type;
        
        label_type label; //!< Label for this record.
        vector_type data; //!< Data.

        template<class Archive>
        void serialize(Archive & ar, const unsigned int version) {
            ar & BOOST_SERIALIZATION_NVP(label)
            & BOOST_SERIALIZATION_NVP(data);
        }
    };

    //! Selector for binary IO.
    struct binaryS { };
    
    //! Selector for XML IO.
    struct xmlS { };
    
    /*! Labeled IDX database.
     */
    template <typename Label, typename Data, typename Format=xmlS>
    class lidx_db {
    public:
        typedef lidx_record<Label,Data> record_type; //!< Type of record used in this lidx_file.
        typedef std::vector<record_type> record_list_type; //!< Type of the underlying database of records.
        typedef std::vector<uint16_t> dim_list_type; //!< Type for a list of dimension sizes.
        typedef Format format_type; //!< Tag for the format of this database.

        //! Constructor.
        lidx_db() {
        }
        
        //! Get the dimension vector.
        dim_list_type& dims() { return _dims; }
        
        //! Get the size of dimension n.
        std::size_t dim(std::size_t n) { return _dims[n]; }
        
        //! Get the record list.
        record_list_type& records() { return _records; }
        
        //! Get a record.
        record_type& operator[](const std::size_t i) { return _records[i]; }
        
    protected:
        dim_list_type _dims; //!< Number and size of dimensions present in records.
        record_list_type _records; //!< Number and size of records in this database.

        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version) {
            ar & BOOST_SERIALIZATION_NVP(_dims)
            & BOOST_SERIALIZATION_NVP(_records);
        }
    };
    
    
    namespace detail {
        namespace bio = boost::iostreams;

        //! Write a potentially gzipped binary archive.
        template <typename DB>
        void write(const std::string& fname,
                   bio::filtering_stream<bio::output>& out,
                   std::ofstream& ofs,
                   DB& db,
                   const lidx::binaryS) {
            ofs.open(fname.c_str(), std::ios::out|std::ios::binary|std::ios::trunc);
            out.push(ofs);
            
            boost::archive::binary_oarchive oa(out);
            oa << BOOST_SERIALIZATION_NVP(db);
        }
        
        template <typename DB>
        void read(const std::string& fname,
                  bio::filtering_stream<bio::input>& in,
                  std::ifstream& ifs,
                  DB& db,
                  const lidx::binaryS) {
            ifs.open(fname.c_str(), std::ios::binary);
            in.push(ifs);
            
            boost::archive::binary_iarchive ia(in);
            ia >> BOOST_SERIALIZATION_NVP(db);
        }
        
        //! Write a potentially gzipped xml archive.
        template <typename DB>
        void write(const std::string& fname,
                   bio::filtering_stream<bio::output>& out,
                   std::ofstream& ofs,
                   DB& db,
                   const lidx::xmlS) {
            ofs.open(fname.c_str(), std::ios::out|std::ios::trunc);
            out.push(ofs);
            
            boost::archive::xml_oarchive oa(out);
            oa << BOOST_SERIALIZATION_NVP(db);
        }
        
        template <typename DB>
        void read(const std::string& fname,
                  bio::filtering_stream<bio::input>& in,
                  std::ifstream& ifs,
                  DB& db,
                  const lidx::xmlS) {
            ifs.open(fname.c_str());
            in.push(ifs);
            
            boost::archive::xml_iarchive ia(in);
            ia >> BOOST_SERIALIZATION_NVP(db);
        }
        
    } // detail
    
    template <typename DB>
    void read(const std::string& fname, DB& db) {
        static const boost::regex e(".*\\.gz$");
        namespace bio = boost::iostreams;
        std::ifstream ifs;
        bio::filtering_stream<bio::input> in;
        
        if(boost::regex_match(fname, e)) {
            in.push(boost::iostreams::gzip_decompressor());
        }
        
        detail::read(fname, in, ifs, db, typename DB::format_type());
    }
            
    template <typename DB>
    void write(const std::string& fname, DB& db) {
        static const boost::regex e(".*\\.gz$");
        namespace bio = boost::iostreams;
        std::ofstream ofs;
        bio::filtering_stream<bio::output> out;

        if(boost::regex_match(fname, e)) {
            out.push(boost::iostreams::gzip_compressor());
        }
        
        detail::write(fname, out, ofs, db, typename DB::format_type());
    }

} // lidx

#endif
