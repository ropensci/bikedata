#pragma once
/***************************************************************************
 *  Project:    bikedata
 *  File:       splite3db-add-data.h
 *  Language:   C++
 *
 *  Author:     Mark Padgham 
 *  E-Mail:     mark.padgham@email.com 
 *
 *  Description:    Routines to store and add data to sqlite3 database.
 *                  Routines to construct sqlite3 database and associated
 *                  indexes are in 'sqlite3db-add-data.cpp'.
 *
 *  Compiler Options:   -std=c++11
 ***************************************************************************/

#include "common.h"
#include "utils.h"
#include "vendor/sqlite3/sqlite3.h"
#include "sqlite3db-utils.h"
#include "read-station-files.h"
#include "read-city-files.h"

#include <sstream>
#include <fstream>
#include <iostream>

// [[Rcpp::depends(BH)]]
#include <Rcpp.h>

struct HeaderStruct {
    std::vector <std::string> city, field_names;
    std::vector <bool> quoted, do_stations;
    std::vector <int> position;
};

int rcpp_import_to_trip_table (const char* bikedb, 
        Rcpp::CharacterVector datafiles, std::string city,
        std::string header_file, bool quiet);
int rcpp_import_to_file_table (const char * bikedb,
        Rcpp::CharacterVector datafiles, std::string city, int nfiles);

HeaderStruct get_all_file_headers (const std::string header_file);
HeaderStruct get_file_headers (const std::string fname, const std::string city,
        const HeaderStruct &headers_all);
