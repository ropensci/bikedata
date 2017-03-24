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

#include <string>
#include <vector>
#include <map>
#include <sqlite3.h>
#include <boost/algorithm/string/replace.hpp>

#define BUFFER_SIZE 512

// [[Rcpp::depends(BH)]]
#include <Rcpp.h>
#include "utils.h"
#include "sqlite3db-add-data.h"


//' create_sqlite3_db
//'
//' Initial creation of SQLite3 database
//' 
//' @param spdb A string containing the path to the Sqlite3 database to 
//' be created.
//'
//' @return integer result code
//'
//' @noRd
// [[Rcpp::export]]
int create_sqlite3_db (const char * spdb)
{
    sqlite3 *dbcon;
    char *zErrMsg = 0;
    int rc;

    rc = sqlite3_open_v2(spdb, &dbcon, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (rc != SQLITE_OK)
        throw std::runtime_error ("Can't establish sqlite3 connection");

    rc = sqlite3_exec(dbcon, "SELECT InitSpatialMetadata(1);", NULL, NULL, &zErrMsg);

    std::string createqry = "CREATE TABLE trips ("
        "id serial primary key,"
        "trip_id integer,"
        "trip_duration numeric,"
        "start_time timestamp without time zone,"
        "stop_time timestamp without time zone,"
        "start_station_id integer,"
        "end_station_id integer,"
        "bike_id integer,"
        "user_type varchar,"
        "birth_year integer,"
        "gender integer"
        ");"
        "CREATE TABLE stations ("
        "    id integer primary key,"
        "    name varchar,"
        "    latitude numeric,"
        "    longitude numeric"
        ");";

    rc = sqlite3_exec(dbcon, createqry.c_str(), NULL, NULL, &zErrMsg);

    rc = sqlite3_close_v2(dbcon);
    if (rc != SQLITE_OK)
        throw std::runtime_error ("Unable to close sqlite database");

    return rc;
}


//' create_db_indexes
//'
//' Creates the specified indexes in the database to speed up queries. Note
//' that for the full dataset this may take some time.
//' 
//' @param spdb A string containing the path to the sqlite3 database to use.
//' @param tables A vector with the tables for which to create indexes. This
//' vector should be the same length as the cols vector.
//' @param cols A vector with the fields for which to create indexes.
//' @param reindex If false, indexes are created, otherwise they are simply
//' reindexed.
//'
//' @return integer result code
//'
//' @noRd
// [[Rcpp::export]]
int create_db_indexes (const char* spdb,
                            Rcpp::CharacterVector tables,
                            Rcpp::CharacterVector cols,
                            bool reindex) 
{
  
  sqlite3 *dbcon;
  char *zErrMsg = 0;
  const char *zStmtMsg;
  int rc;
  
  rc = sqlite3_open_v2(spdb, &dbcon, SQLITE_OPEN_READWRITE, NULL);
  if (rc != SQLITE_OK)
    throw std::runtime_error ("Can't establish sqlite3 connection");
  
  char *idxsql = NULL;
  sqlite3_stmt *versionstmt;
  char *sqliteversion = (char *)"0.1";
  
  if (rc == SQLITE_OK) {
    
    rc = sqlite3_prepare_v2(dbcon, "SELECT sqlite_version()", -1, &versionstmt, 0);
    if (rc != SQLITE_OK) {
      throw std::runtime_error ("Unable to retrieve sqlite version");
    }
    rc = sqlite3_step(versionstmt);
    
    if (rc == SQLITE_ROW) {
      sqliteversion = (char *)sqlite3_column_text(versionstmt, 0);
    }
    rc = sqlite3_reset(versionstmt);

    for (unsigned int i = 0; i < cols.length(); ++i) {
    
      if (((std::string)cols[i]).find("(") == std::string::npos || 
          compare_version_numbers(sqliteversion, "3.9.0") >= 0) {
        
        std::string idxname = "idx_" + tables[i] + "_" + (std::string)cols[i];
        boost::replace_all(idxname, "(", "_");
        boost::replace_all(idxname, ")", "_");
        boost::replace_all(idxname, " ", "_");
        
        if (reindex)
            int sprrc = asprintf(&idxsql, "REINDEX %s ", idxname.c_str());
        else
            int sprrc = asprintf(&idxsql, "CREATE INDEX %s ON %s(%s)", idxname.c_str(), (char *)(tables[i]), (char *)(cols[i]));

        rc = sqlite3_exec(dbcon, idxsql, NULL, NULL, &zErrMsg);
        if (rc != SQLITE_OK) {
          throw std::runtime_error ("Unable to execute index query: " + (std::string)idxsql);
        }
        
      }
      else {
        Rcpp::warning("Unable to create index on " + cols[i] + ", expression not supported in SQLite version < 3.9.0");
      }
    
    } 
  
  }
  
  rc = sqlite3_close_v2(dbcon);
  if (rc != SQLITE_OK) {
    throw std::runtime_error ("Unable to close sqlite database");
  }
  
  return(rc);

}
