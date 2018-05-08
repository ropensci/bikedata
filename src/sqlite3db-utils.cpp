/***************************************************************************
 *  Project:    bikedata
 *  File:       splite3db-utils.cpp
 *  Language:   C++
 *
 *  Author:     Mark Padgham 
 *  E-Mail:     mark.padgham@email.com 
 *
 *  Description:    Utility functions for interaction with sqlite3 database.
 *
 *  Compiler Options:   -std=c++11
 ***************************************************************************/

#include "sqlite3db-utils.h"

//' get_max_trip_id
//'
//' @param dbcon Active connection to sqlite3 database
//'
//' @return Maximal database primary ID of trips table
//'
//' @noRd
int db_utils::get_max_trip_id (sqlite3 * dbcon)
{
    sqlite3_stmt * stmt;
    char qry_id [BUFFER_SIZE] = "\0";
    int rc = sprintf(qry_id, "SELECT MAX(id) FROM trips");
    rc = sqlite3_prepare_v2(dbcon, qry_id, BUFFER_SIZE, &stmt, nullptr);
    rc = sqlite3_step (stmt);
    int max_trip_id = sqlite3_column_int (stmt, 0);
    sqlite3_finalize (stmt);
    (void) rc; // supress unused variable warning;

    return max_trip_id;
}

//' get_max_stn_id
//'
//' @param dbcon Active connection to sqlite3 database
//'
//' @return Maximal database primary ID of station table
//'
//' @noRd
int db_utils::get_max_stn_id (sqlite3 * dbcon)
{
    sqlite3_stmt * stmt;
    char qry_id [BUFFER_SIZE] = "\0";
    sprintf(qry_id, "SELECT MAX(id) FROM stations");
    int rc = sqlite3_prepare_v2(dbcon, qry_id, BUFFER_SIZE, &stmt, nullptr);
    rc = sqlite3_step (stmt);
    int max_stn_id = sqlite3_column_int (stmt, 0);
    sqlite3_finalize (stmt);
    (void) rc; // supress unused variable warning;

    return max_stn_id;
}

//' get_stn_table_size
//'
//' @param dbcon Active connection to sqlite3 database
//'
//' @return Number of stations in table
//'
//' @noRd
int db_utils::get_stn_table_size (sqlite3 * dbcon)
{
    sqlite3_stmt * stmt;
    char qry_id [BUFFER_SIZE] = "\0";
    sprintf(qry_id, "SELECT COUNT(*) FROM stations");
    int rc = sqlite3_prepare_v2(dbcon, qry_id, BUFFER_SIZE, &stmt, nullptr);
    rc = sqlite3_step (stmt);
    int num_stns = sqlite3_column_int (stmt, 0);
    sqlite3_finalize (stmt);
    (void) rc; // supress unused variable warning;

    return num_stns;
}
