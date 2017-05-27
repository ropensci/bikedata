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

#define BUFFER_SIZE 512

#include <stdio.h>

// [[Rcpp::depends(BH)]]
#include <Rcpp.h>
#include "utils.h"
#include "sqlite3db-add-data.h"
#include "sqlite3/sqlite3.h"


//' rcpp_create_sqlite3_db
//'
//' Initial creation of SQLite3 database
//' 
//' @param bikedb A string containing the path to the Sqlite3 database to 
//'        be created.
//'
//' @return integer result code
//'
//' @noRd
// [[Rcpp::export]]
int rcpp_create_sqlite3_db (const char * bikedb)
{
    sqlite3 *dbcon;
    char *zErrMsg = 0;
    int rc;

    rc = sqlite3_open_v2(bikedb, &dbcon, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (rc != SQLITE_OK)
        throw std::runtime_error ("Can't establish sqlite3 connection");

    rc = sqlite3_exec(dbcon, "SELECT InitSpatialMetadata(1);", NULL, NULL, &zErrMsg);

    // NOTE: Database structure is ordered according to the order of the NYC
    // citibike system, so each line of data from that city can be injected
    // straight into the db. All other cities require re-ordering of data to
    // this citibike sequence prior to injection into db.

    std::string createqry = "CREATE TABLE trips ("
        "id integer primary key,"
        "city text,"
        "trip_duration numeric,"
        "start_time timestamp without time zone,"
        "stop_time timestamp without time zone,"
        "start_station_id integer,"
        "end_station_id integer,"
        "bike_id integer,"
        "user_type text,"
        "birth_year integer,"
        "gender integer"
        ");"
        "CREATE TABLE stations ("
        "    id integer primary key,"
        "    city text,"
        "    stn_id text,"
        "    name text,"
        "    longitude numeric,"
        "    latitude numeric,"
        "    UNIQUE (stn_id, name)"
        ");"
        "CREATE TABLE datafiles ("
        "    id integer primary key,"
        "    city text,"
        "    name text"
        ");";

    rc = sqlite3_exec(dbcon, createqry.c_str(), NULL, NULL, &zErrMsg);

    rc = sqlite3_close_v2(dbcon);
    if (rc != SQLITE_OK)
        throw std::runtime_error ("Unable to close sqlite database");

    return rc;
}


//' rcpp_create_db_indexes
//'
//' Creates the specified indexes in the database to speed up queries. Note
//' that for the full dataset this may take some time.
//' 
//' @param bikedb A string containing the path to the sqlite3 database to use.
//' @param tables A vector with the tables for which to create indexes. This
//'        vector should be the same length as the cols vector.
//' @param cols A vector with the fields for which to create indexes.
//' @param reindex If false, indexes are created, otherwise they are simply
//'        reindexed.
//'
//' @return integer result code
//'
//' @noRd
// [[Rcpp::export]]
int rcpp_create_db_indexes (const char* bikedb, Rcpp::CharacterVector tables,
        Rcpp::CharacterVector cols, bool reindex) 
{
    sqlite3 *dbcon;
    char *zErrMsg = 0;
    int rc;

    rc = sqlite3_open_v2(bikedb, &dbcon, SQLITE_OPEN_READWRITE, NULL);
    if (rc != SQLITE_OK)
        throw std::runtime_error ("Can't establish sqlite3 connection");

    sqlite3_stmt *versionstmt;
    char *sqliteversion = (char *)"0.1";

    rc = sqlite3_prepare_v2(dbcon, "SELECT sqlite_version()", -1, 
            &versionstmt, 0);
    if (rc != SQLITE_OK) 
        throw std::runtime_error ("Unable to retrieve sqlite version");
    rc = sqlite3_step(versionstmt);

    if (rc == SQLITE_ROW) 
        sqliteversion = (char *)sqlite3_column_text(versionstmt, 0);
    rc = sqlite3_reset(versionstmt);

    for (int i = 0; i < cols.length(); ++i) 
    {
        std::string idxname = "idx_" + tables[i] + "_" + 
            (std::string)cols[i];
        boost::replace_all(idxname, "(", "_");
        boost::replace_all(idxname, ")", "_");
        boost::replace_all(idxname, " ", "_");

        std::string idxqry;
        if (reindex)
            idxqry = "REINDEX " + idxname;
        else
            idxqry = "CREATE INDEX " + idxname + " ON " +
                (char *)(tables [i]) + "(" + (char *)(cols [i]) + ")";

        rc = sqlite3_exec(dbcon, idxqry.c_str(), NULL, NULL, &zErrMsg);
        if (rc != SQLITE_OK) 
        {
            std::string errMsg = "Unable to execute index query: " + 
                idxqry;
            throw std::runtime_error (errMsg);
        }
    } 

    rc = sqlite3_close_v2(dbcon);
    if (rc != SQLITE_OK) 
        throw std::runtime_error ("Unable to close sqlite database");
  
    return(rc);
}

//' rcpp_create_city_index
//'
//' Creates city index in the database. This function is *always* run, while the
//' 'create_db_indexes' function is optionally run.
//' 
//' @param bikedb A string containing the path to the sqlite3 database to use.
//' @param reindex If false, indexes are created, otherwise they are simply
//'        reindexed.
//'
//' @return integer result code
//'
//' @noRd
// [[Rcpp::export]]
int rcpp_create_city_index (const char* bikedb, bool reindex) 
{
    sqlite3 *dbcon;
    char *zErrMsg = 0;
    int rc;

    rc = sqlite3_open_v2(bikedb, &dbcon, SQLITE_OPEN_READWRITE, NULL);
    if (rc != SQLITE_OK)
        throw std::runtime_error ("Can't establish sqlite3 connection");

    std::string idxname = "idx_trips_city";
    std::string idxqry;
    if (reindex)
        idxqry = "REINDEX " + idxname;
    else
        idxqry = "CREATE INDEX " + idxname + " ON trips(city)";

    rc = sqlite3_exec(dbcon, idxqry.c_str(), NULL, NULL, &zErrMsg);

    if (rc != SQLITE_OK) 
    {
        std::string errMsg = "Unable to execute index query: " + idxqry;
        throw std::runtime_error (errMsg);
    }

    rc = sqlite3_close_v2(dbcon);
    if (rc != SQLITE_OK) 
        throw std::runtime_error ("Unable to close sqlite database");
  
    return(rc);
}
