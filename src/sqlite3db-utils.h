#pragma once
/***************************************************************************
 *  Project:    bikedata
 *  File:       splite3db-utils.h
 *  Language:   C++
 *
 *  Author:     Mark Padgham 
 *  E-Mail:     mark.padgham@email.com 
 *
 *  Description:    Utility functions for interaction with sqlite3 database.
 *
 *  Compiler Options:   -std=c++11
 ***************************************************************************/

#include "common.h"
#include "utils.h"
#include "vendor/sqlite3/sqlite3.h"

#define BUFFER_SIZE 512

int get_max_trip_id (sqlite3 * dbcon);
int get_max_stn_id (sqlite3 * dbcon);
int get_stn_table_size (sqlite3 * dbcon);
