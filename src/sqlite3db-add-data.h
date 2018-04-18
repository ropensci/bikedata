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

int rcpp_import_to_trip_table (const char* bikedb, 
        Rcpp::CharacterVector datafiles, std::string city,
        std::string header_file_name, bool data_has_stations, bool quiet);
int rcpp_import_to_file_table (const char * bikedb,
        Rcpp::CharacterVector datafiles, std::string city, int nfiles);

HeaderStruct get_field_positions (const std::string fname,
        const std::string header_file_name, bool data_has_stations,
        const std::string city);
void get_field_quotes (const std::string line, HeaderStruct &headers);
void dump_headers (const HeaderStruct &headers);
