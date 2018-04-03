#pragma once
/***************************************************************************
 *  Project:    bikedata
 *  File:       splite3db-admin.cpp
 *  Language:   C++
 *
 *  Author:     Mark Padgham 
 *  E-Mail:     mark.padgham@email.com 
 *
 *  Description:    Routines to construct sqlite3 database and associated
 *  indexes. Routines to store and add data are in 'sqlite3db-add-data.h'
 *
 *  Compiler Options:   -std=c++11
 ***************************************************************************/

#include <stdio.h>

// [[Rcpp::depends(BH)]]
#include <Rcpp.h>
#include "common.h"
#include "utils.h"
#include "sqlite3db-add-data.h"
#include "vendor/sqlite3/sqlite3.h"

int rcpp_create_sqlite3_db (const char * bikedb);
int rcpp_create_db_indexes (const char* bikedb, Rcpp::CharacterVector tables,
        Rcpp::CharacterVector cols, bool reindex);
int rcpp_create_city_index (const char* bikedb, bool reindex);
