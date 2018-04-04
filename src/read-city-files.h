/***************************************************************************
 *  Project:    bikedata
 *  File:       read-city-files
 *  Language:   C++
 *
 *  Author:     Mark Padgham 
 *  E-Mail:     mark.padgham@email.com 
 *
 *  Description:    Routines to read single lines of the data files for
 *                  different cities.
 *
 *  Compiler Options:   -std=c++11
 ***************************************************************************/
#pragma once

#include "common.h"
#include "utils.h"
#include "vendor/sqlite3/sqlite3.h"

// NOTE: Return values are only used for LA

unsigned int read_one_line_nyc (sqlite3_stmt * stmt, char * line,
        std::map <std::string, std::string> * stationqry, const char * delim);
unsigned int read_one_line_nyc_standard (sqlite3_stmt * stmt,
        std::string &in_line2,
        std::map <std::string, std::string> * stationqry, const char * delim);
unsigned int read_one_line_nyc_mixed (sqlite3_stmt * stmt,
        std::string &in_line2,
        std::map <std::string, std::string> * stationqry);

unsigned int read_one_line_boston (sqlite3_stmt * stmt, char * line,
        std::map <std::string, std::string> &stn_map, 
        std::unordered_set <std::string> &stn_ids);
unsigned int read_one_line_boston_pre15 (sqlite3_stmt * stmt, char * line,
        std::map <std::string, std::string> &stn_map, 
        std::unordered_set <std::string> &stn_ids);
unsigned int read_one_line_boston_pre18 (sqlite3_stmt * stmt, char * line,
        std::map <std::string, std::string> &stn_map, 
        std::unordered_set <std::string> &stn_ids);
unsigned int read_one_line_boston_post18 (sqlite3_stmt * stmt, char * line,
        std::map <std::string, std::string> &stn_map, 
        std::unordered_set <std::string> &stn_ids);
std::string convert_bo_stn_name (std::string &station_name,
        std::map <std::string, std::string> &stn_map);

unsigned int read_one_line_chicago (sqlite3_stmt * stmt, char * line,
        const char * delim);
unsigned int read_one_line_dc (sqlite3_stmt * stmt, char * line, 
        std::map <std::string, std::string> &stn_map, 
        std::unordered_set <std::string> &stn_ids);
std::string convert_dc_stn_name (std::string &station_name, bool id,
        std::map <std::string, std::string> &stn_map);

unsigned int read_one_line_london (sqlite3_stmt * stmt, char * line);
std::string add_0_to_time (std::string time);

unsigned int read_one_line_nabsa (sqlite3_stmt * stmt, char * line,
        std::map <std::string, std::string> * stationqry,
        std::string city);

unsigned int read_one_line_mn (sqlite3_stmt * stmt, char * line);
