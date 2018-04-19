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

unsigned int read_one_line_generic (sqlite3_stmt * stmt, char * line,
        std::map <std::string, std::string> * stationqry,
        const std::string city, const HeaderStruct &headers,
        std::map <std::string, std::string> &stn_map);
unsigned int read_one_line_london (sqlite3_stmt * stmt, char * line);
unsigned int read_one_line_nabsa (sqlite3_stmt * stmt, char * line,
        std::map <std::string, std::string> * stationqry,
        std::string city);

std::string convert_usertype (std::string ut);
std::string convert_gender (std::string g);

std::string convert_bo_stn_name (std::string &station_name,
        std::map <std::string, std::string> &stn_map);
std::string convert_dc_stn_name (std::string &station_name, bool id,
        std::map <std::string, std::string> &stn_map);
