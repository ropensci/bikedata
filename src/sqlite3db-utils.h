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

#define BUFFER_SIZE 512

int get_max_stn_id (sqlite3 * dbcon);
int get_stn_table_size (sqlite3 * dbcon);

//' get_max_trip_id
//'
//' @param dbcon Active connection to sqlite3 database
//'
//' @return Maximal database primary ID of trips table
//'
//' @noRd
int get_max_trip_id (sqlite3 * dbcon)
{
    sqlite3_stmt * stmt;
    char qry_id [BUFFER_SIZE];
    int rc = sprintf(qry_id, "SELECT MAX(id) FROM trips");
    rc = sqlite3_prepare_v2(dbcon, qry_id, BUFFER_SIZE, &stmt, NULL);
    rc = sqlite3_step (stmt);
    int max_trip_id = sqlite3_column_int (stmt, 0);
    sqlite3_reset (stmt);
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
int get_max_stn_id (sqlite3 * dbcon)
{
    sqlite3_stmt * stmt;
    char qry_id [BUFFER_SIZE];
    sprintf(qry_id, "SELECT MAX(id) FROM stations");
    int rc = sqlite3_prepare_v2(dbcon, qry_id, BUFFER_SIZE, &stmt, NULL);
    rc = sqlite3_step (stmt);
    int max_stn_id = sqlite3_column_int (stmt, 0);
    sqlite3_reset (stmt);
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
int get_stn_table_size (sqlite3 * dbcon)
{
    sqlite3_stmt * stmt;
    char qry_id [BUFFER_SIZE];
    sprintf(qry_id, "SELECT COUNT(*) FROM stations");
    int rc = sqlite3_prepare_v2(dbcon, qry_id, BUFFER_SIZE, &stmt, NULL);
    rc = sqlite3_step (stmt);
    int num_stns = sqlite3_column_int (stmt, 0);
    sqlite3_reset (stmt);
    (void) rc; // supress unused variable warning;

    return num_stns;
}
