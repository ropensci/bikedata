#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <sys/stat.h>
#include <sqlite3.h>
#include <spatialite/gaiageo.h>
#include <spatialite.h>
#include <time.h>

#define BUFFER_SIZE 512

// [[Rcpp::depends(BH)]]
#include <Rcpp.h>
using namespace Rcpp;

#include <boost/algorithm/string/replace.hpp>

// A string delimiter function based on strtok
// Accessed from StackOverflow (using M Oehm):
// http://stackoverflow.com/questions/29847915/implementing-strtok-whose-delimiter-has-more-than-one-character
char *strtokm(char *str, const char *delim)
{
  static char *tok;
  static char *next;
  char *m;
  
  if (delim == NULL) return NULL;
  
  tok = (str) ? str : next;
  if (tok == NULL) return NULL;
  
  m = strstr(tok, delim);
  
  if (m) {
    next = m + strlen(delim);
    *m = '\0';
  } else {
    next = NULL;
  }
  
  return tok;
}

//' import data using spatialite
//'
//' Extracts bike data for NYC citibike
//' 
//' @param datafiles A character vector containin the paths to the citibike 
//' .csv files to import.
//' @param spdb A string containing the path to the spatialite database to 
//' use. It will be created automatically.
//' @param quiet If FALSE, progress is displayed on screen
//'
//' @return integer result code
// [[Rcpp::export]]
int importDataToSpatialite(CharacterVector datafiles, const char* spdb, bool quiet) 
{
  
  sqlite3 *dbcon;
  char *zErrMsg = 0;
  int rc;
  void* cache = spatialite_alloc_connection();
  
  rc = sqlite3_open_v2(spdb, &dbcon, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
  if (rc != SQLITE_OK)
      throw std::runtime_error ("Can't establish sqlite3 connection");
  spatialite_init_ex(dbcon, cache, 0);
  
  rc = sqlite3_exec(dbcon, "SELECT InitSpatialMetadata(1);", NULL, NULL, &zErrMsg);
  
  std::string createqry = "CREATE TABLE trips ("
    "id serial primary key,"
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
  FILE * pFile;
  char in_line [BUFFER_SIZE] = "\0";
  char sqlqry [BUFFER_SIZE] = "\0";
  
  sqlite3_stmt * stmt;
  char * tail = 0;
  std::map <std::string, std::string> stationqry;
  
  sprintf(sqlqry, "INSERT INTO trips VALUES (NULL, @TD, @ST, @ET, @SSID, @ESID, @BID, @UT, @BY, @GE)");
  
  sqlite3_prepare_v2(dbcon, sqlqry, BUFFER_SIZE, &stmt, NULL);
  
  sqlite3_exec(dbcon, "BEGIN TRANSACTION", NULL, NULL, &zErrMsg);
  
  int ntrips = 0;
  for(unsigned int filenum = 0; filenum < datafiles.length(); filenum++) {

    if (!quiet)
        Rcpp::Rcout << "reading file#" << filenum << ": " <<
            datafiles [filenum] << std::endl;

    pFile = fopen(datafiles[filenum], "r");
    char * junk = fgets(in_line, BUFFER_SIZE, pFile);
    
    std::string tripqrystart = "INSERT INTO trips (trip_duration, start_time, stop_time, start_station_id, end_station_id, bike_id, user_type, birth_year, gender) VALUES ";
    std::string tripqry = tripqrystart;
    
    std::string birthyear = "", gender = "", 
        startstationid = "", endstationid = "";

    std::vector< std::string > vec;
    
    while (fgets (in_line, BUFFER_SIZE, pFile) != NULL) {
      
      ntrips++;
      std::string in_line2 = in_line;
      boost::replace_all(in_line2, "\\N","\"\"");
      strtokm(&in_line2[0u], "\""); //First double speech marks
      sqlite3_bind_text(stmt, 1, strtokm(NULL, "\",\""), -1, SQLITE_TRANSIENT); // Trip duration
      sqlite3_bind_text(stmt, 2, strtokm(NULL, "\",\""), -1, SQLITE_TRANSIENT); // Start time
      sqlite3_bind_text(stmt, 3, strtokm(NULL, "\",\""), -1, SQLITE_TRANSIENT); // Stop time
      startstationid = strtokm(NULL, "\",\"");
      if (stationqry.count(startstationid) == 0) {
        stationqry[startstationid] = "(" + startstationid + "," + 
            strtokm(NULL, "\",\"") + "," + strtokm(NULL, "\",\"") + ",'" + 
            strtokm(NULL, "\",\"") + "')";
      }
      else {
        strtokm(NULL, "\",\"");
        strtokm(NULL, "\",\"");
        strtokm(NULL, "\",\"");
      }
      
      sqlite3_bind_text(stmt, 4, startstationid.c_str(), -1, SQLITE_TRANSIENT); // Start station ID
      
      endstationid = strtokm(NULL, "\",\"");
      if (stationqry.count(endstationid) == 0) {
        stationqry[endstationid] = "(" + endstationid + "," + 
            strtokm(NULL, "\",\"") + "," + strtokm(NULL, "\",\"") + ",'" + 
            strtokm(NULL, "\",\"") + "')";
      }
      else {
        strtokm(NULL, "\",\"");
        strtokm(NULL, "\",\"");
        strtokm(NULL, "\",\"");
      }
      
      sqlite3_bind_text(stmt, 5, endstationid.c_str(), -1, SQLITE_TRANSIENT); // End station ID
      sqlite3_bind_text(stmt, 6, strtokm(NULL, "\",\""), -1, SQLITE_TRANSIENT); // Bicycle ID
      sqlite3_bind_text(stmt, 7, strtokm(NULL, "\",\""), -1, SQLITE_TRANSIENT); // User type
      birthyear = strtokm(NULL, "\",\"");
      gender = strtokm(NULL, "\",\"");
      if (birthyear.empty()) {
        birthyear = "NULL";
      }
      if (gender.empty()) {
        gender = "NULL";
      }
      sqlite3_bind_text(stmt, 8, birthyear.c_str(), -1, SQLITE_TRANSIENT); // Birth Year
      sqlite3_bind_text(stmt, 9, gender.c_str(), -1, SQLITE_TRANSIENT); // Gender
      
      sqlite3_step(stmt);
      
      sqlite3_reset(stmt);
      
    }
    
  }
  
  sqlite3_exec(dbcon, "END TRANSACTION", NULL, NULL, &zErrMsg);
  
  
   std::string fullstationqry = "INSERT INTO stations (id, longitude, latitude, name) VALUES ";
   fullstationqry = fullstationqry + stationqry.begin ()->second;
   for (auto thisstation = std::next (stationqry.begin ());
           thisstation != stationqry.end (); ++thisstation)
       fullstationqry = fullstationqry + ", " + thisstation->second;
   fullstationqry = fullstationqry + ";";

   rc = sqlite3_exec(dbcon, fullstationqry.c_str(), NULL, 0, &zErrMsg);
   rc = sqlite3_exec(dbcon, "SELECT AddGeometryColumn('stations', 'geom', 4326, 'POINT', 'XY');", NULL, 0, &zErrMsg);

   rc = sqlite3_exec(dbcon, "UPDATE stations SET geom = MakePoint(longitude, latitude, 4326);", NULL, 0, &zErrMsg);

  rc = sqlite3_close_v2(dbcon);
  if (rc != SQLITE_OK)
      throw std::runtime_error ("Unable to close sqlite database");
  
  return(ntrips);
}
