#pragma once
/***************************************************************************
 *  Project:    bikedata
 *  File:       read-station-files.h
 *  Language:   C++
 *
 *  Author:     Mark Padgham 
 *  E-Mail:     mark.padgham@email.com 
 *
 *  Description:    Routines to read and store data on bike docking stations in
 *                  the stations table of the SQLite3 database.
 *
 *  Compiler Options:   -std=c++11
 ***************************************************************************/

#include <unordered_set>

#include "sqlite3db-utils.h"

int import_to_station_table (sqlite3 * dbcon,
    std::map <std::string, std::string> stationqry);

std::map <std::string, std::string> get_bo_stn_table (sqlite3 * dbcon);
std::map <std::string, std::string> get_dc_stn_table (sqlite3 * dbcon);
std::unordered_set <std::string> get_stn_ids (sqlite3 * dbcon, std::string ci);

int rcpp_import_stn_df (const char * bikedb, Rcpp::DataFrame stn_data,
        std::string city);
