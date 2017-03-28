/***************************************************************************
 *  Project:    bikedata
 *  File:       splite3db-add-data.h
 *  Language:   C++
 *
 *  Author:     Mark Padgham 
 *  E-Mail:     mark.padgham@email.com 
 *
 *  Description:    Routines to store and add data to sqlite3 database.
 *                  Routines to construct sqlite3 database and associated
 *                  indexes are in 'sqlite3db-add-data.cpp'.
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

// Function defs here just so main 'rcpp_import_to_trip_table' function can be
// given first, followed by individual functions for each city 
int rcpp_import_to_station_table (sqlite3 * dbcon,
    std::map <std::string, std::string> stationqry);
int rcpp_import_to_trip_table (const char* bikedb, 
        Rcpp::CharacterVector datafiles, std::string city, bool quiet);
int rcpp_import_to_datafile_table (const char * bikedb,
        Rcpp::CharacterVector datafiles, std::string city);
void read_one_line_nyc (sqlite3_stmt * stmt, char * line,
        std::map <std::string, std::string> * stationqry, const char * delim);

//' rcpp_import_to_trip_table
//'
//' Extracts bike data for NYC citibike
//' 
//' @param bikedb A string containing the path to the Sqlite3 database to 
//'        use. It will be created automatically.
//' @param datafiles A character vector containin the paths to the citibike 
//'        .csv files to import.
//' @param city First two letters of city for which data are to be added (thus
//'        far, "ny", "bo", "ch", "dc", and "la")
//' @param quiet If FALSE, progress is displayed on screen
//'
//' @return integer result code
//'
//' @noRd
// [[Rcpp::export]]
int rcpp_import_to_trip_table (const char* bikedb, 
        Rcpp::CharacterVector datafiles, std::string city, bool quiet)
{
    sqlite3 *dbcon;
    char *zErrMsg = 0;
    int rc;

    rc = sqlite3_open_v2(bikedb, &dbcon, SQLITE_OPEN_READWRITE, NULL);
    if (rc != SQLITE_OK)
        throw std::runtime_error ("Can't establish sqlite3 connection");

    FILE * pFile;
    char in_line [BUFFER_SIZE] = "\0";
    char sqlqry [BUFFER_SIZE] = "\0";

    sqlite3_stmt * stmt;
    char * tail = 0;
    std::map <std::string, std::string> stationqry;

    // Get max trip_id = # trips already in db
    int trip_id, ntrips = 0; // ntrips is # added in this call
    char qry_id [BUFFER_SIZE];
    sprintf(qry_id, "SELECT MAX(trip_id) FROM trips");
    rc = sqlite3_prepare_v2(dbcon, qry_id, BUFFER_SIZE, &stmt, NULL);
    rc = sqlite3_step (stmt);
    trip_id = sqlite3_column_int (stmt, 0);
    sqlite3_reset(stmt);
    trip_id++;

    sprintf(sqlqry, "INSERT INTO trips VALUES (NULL, @II, @CI, @TD, @ST, @ET, @SSID, @ESID, @BID, @UT, @BY, @GE)");

    sqlite3_prepare_v2(dbcon, sqlqry, BUFFER_SIZE, &stmt, NULL);

    sqlite3_exec(dbcon, "BEGIN TRANSACTION", NULL, NULL, &zErrMsg);

    for(unsigned int filenum = 0; filenum < datafiles.length(); filenum++) 
    {
        if (!quiet)
            Rcpp::Rcout << "reading file " << filenum + 1 << "/" <<
                datafiles.size() << ": " <<
                datafiles [filenum] << std::endl;

        pFile = fopen(datafiles[filenum], "r");
        char * junk = fgets(in_line, BUFFER_SIZE, pFile);
        rm_dos_end (in_line);

        const char * delim;
        if (line_has_quotes (in_line))
            delim = "\",\"";
        else
            delim = ",";

        while (fgets (in_line, BUFFER_SIZE, pFile) != NULL) 
        {
            rm_dos_end (in_line);
            sqlite3_bind_text(stmt, 1, std::to_string (trip_id).c_str(), -1, 
                    SQLITE_TRANSIENT); // Trip ID
            sqlite3_bind_text(stmt, 2, city.c_str (), -1, SQLITE_TRANSIENT); 
            read_one_line_nyc (stmt, in_line, &stationqry, delim);
            trip_id++;
            ntrips++;

            sqlite3_step(stmt);
            sqlite3_reset(stmt);
        }
    }

    sqlite3_exec(dbcon, "END TRANSACTION", NULL, NULL, &zErrMsg);

    rcpp_import_to_station_table (dbcon, stationqry);

    rc = sqlite3_close_v2(dbcon);
    if (rc != SQLITE_OK)
        throw std::runtime_error ("Unable to close sqlite database");

    return (ntrips);
}

//' rcpp_import_to_station_table
//'
//' Creates and/or updates the table of stations in the database
//' 
//' @param dbcon Active connection to sqlite3 database
//' @param stationqry Station query constructed during reading of data with
//'        rcpp_import_to_trip_table ()
//'
//' @return integer result code
//'
//' @noRd
int rcpp_import_to_station_table (sqlite3 * dbcon,
    std::map <std::string, std::string> stationqry)
{
    char *zErrMsg = 0;
    int rc;

    std::string fullstationqry = "INSERT INTO stations "
                                 "(id, city, longitude, latitude, name) VALUES ";
    fullstationqry = fullstationqry + stationqry.begin ()->second;
    for (auto thisstation = std::next (stationqry.begin ());
            thisstation != stationqry.end (); ++thisstation)
        fullstationqry = fullstationqry + ", " + thisstation->second;
    fullstationqry = fullstationqry + ";";

    rc = sqlite3_exec(dbcon, fullstationqry.c_str(), NULL, 0, &zErrMsg);
    std::string qry = "SELECT AddGeometryColumn"
                      "('stations', 'geom', 4326, 'POINT', 'XY');";
    rc = sqlite3_exec(dbcon, qry.c_str (), NULL, 0, &zErrMsg);

    qry = "UPDATE stations SET geom = MakePoint(longitude, latitude, 4326);";
    rc = sqlite3_exec(dbcon, qry.c_str (), NULL, 0, &zErrMsg);

    return rc;
}

//' rcpp_import_to_datafile_table
//'
//' Creates and/or updates the table of datafile names in the database
//' 
//' @param bikedb A string containing the path to the Sqlite3 database to 
//'        use. 
//' @param datafiles List of names of files to be added - must be names of
//'        compressed \code{.zip} archives, not expanded \code{.csv} files
//' @param city Name of city associated with datafile
//'
//' @return Number of datafile names added to database table
//'
//' @noRd
// [[Rcpp::export]]
int rcpp_import_to_datafile_table (const char * bikedb,
        Rcpp::CharacterVector datafiles, std::string city, int nfiles)
{
    sqlite3 *dbcon;
    char *zErrMsg = 0;
    int rc;

    nfiles++;

    rc = sqlite3_open_v2(bikedb, &dbcon, SQLITE_OPEN_READWRITE, NULL);
    if (rc != SQLITE_OK)
        throw std::runtime_error ("Can't establish sqlite3 connection");

    for (auto i : datafiles)
    {
        std::string datafile_qry = "INSERT INTO datafiles "
                                   "(id, city, name) VALUES (";
        datafile_qry += std::to_string (nfiles++) + ",\"" + city + "\",\"" + 
            i + "\");";

        rc = sqlite3_exec(dbcon, datafile_qry.c_str(), NULL, 0, &zErrMsg);
    }

    rc = sqlite3_close_v2(dbcon);
    if (rc != SQLITE_OK)
        throw std::runtime_error ("Unable to close sqlite database");

    return rc;
}

//' read_one_line_nyc
//'
//' @param stmt An sqlit3 statement to be assembled by reading the line of data
//' @param line Line of data read from citibike file
//' @param stationqry Sqlite3 query for station data table to be subsequently
//'        passed to 'rcpp_import_to_station_table()'
//' @param delim Delimeter for data files (changes from 2015-> from
//'        double-quoted fields to plain comma-separators.
//'
//' @noRd
void read_one_line_nyc (sqlite3_stmt * stmt, char * line,
        std::map <std::string, std::string> * stationqry, const char * delim)
{
    std::string in_line2 = line;
    char * token;
    if (strncmp (delim, "\",\"", 3) == 0)
    {
        // Example of the following on L#19 of 2014-07
        boost::replace_all(in_line2, "\\N","\"\"");
        token = strtokm(&in_line2[0u], "\""); //First double speech marks
        token = strtokm(NULL, delim); 
    } else
        token = strtokm(&in_line2[0u], delim);

    sqlite3_bind_text(stmt, 3, token, -1, SQLITE_TRANSIENT); // Trip duration

    std::string tempstr = convert_datetime (strtokm(NULL, delim)); // Start time
    sqlite3_bind_text(stmt, 4, tempstr.c_str(), -1, SQLITE_TRANSIENT); 

    tempstr = convert_datetime (strtokm(NULL, delim)); // Stop time
    sqlite3_bind_text(stmt, 5, tempstr.c_str(), -1, SQLITE_TRANSIENT); 
    std::string startstationid = strtokm(NULL, delim);
    if (stationqry->count(startstationid) == 0) {
        (*stationqry)[startstationid] = "(" + startstationid + ",\"ny\"," + 
            strtokm(NULL, delim) + "," + strtokm(NULL, delim) + ",'" + 
            strtokm(NULL, delim) + "')";
    }
    else {
        strtokm(NULL, delim);
        strtokm(NULL, delim);
        strtokm(NULL, delim);
    }

    sqlite3_bind_text(stmt, 6, startstationid.c_str(), -1, SQLITE_TRANSIENT); 

    std::string endstationid = strtokm(NULL, delim);
    if (stationqry->count(endstationid) == 0) {
        (*stationqry)[endstationid] = "(" + endstationid + ",\"ny\"," + 
            strtokm(NULL, delim) + "," + strtokm(NULL, delim) + ",'" + 
            strtokm(NULL, delim) + "')";
    }
    else {
        strtokm(NULL, delim);
        strtokm(NULL, delim);
        strtokm(NULL, delim);
    }

    sqlite3_bind_text(stmt, 7, endstationid.c_str(), -1, SQLITE_TRANSIENT); 
    // next two are bike id and user type
    sqlite3_bind_text(stmt, 8, strtokm(NULL, delim), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 9, strtokm(NULL, delim), -1, SQLITE_TRANSIENT); 
    std::string birthyear = strtokm(NULL, delim);
    std::string gender = strtokm(NULL, delim);
    if (gender.length () == 2) // gender still has a terminal quote
        gender = gender [0];
    if (birthyear.empty()) {
        birthyear = "NULL";
    }
    if (gender.empty()) {
        gender = "NULL";
    }
    sqlite3_bind_text(stmt, 10, birthyear.c_str(), -1, SQLITE_TRANSIENT); // Birth Year
    sqlite3_bind_text(stmt, 11, gender.c_str(), -1, SQLITE_TRANSIENT); // Gender
}

//' read_one_line_boston
//'
//' @param stmt An sqlit3 statement to be assembled by reading the line of data
//' @param line Line of data read from citibike file
//' @param stationqry Sqlite3 query for station data table to be subsequently
//'        passed to 'rcpp_import_to_station_table()'
//'
//' @noRd
void read_one_line_boston (sqlite3_stmt * stmt, char * line,
        std::map <std::string, std::string> * stationqry)
{
    // TDOD: Replace strokm with strok here!
    const char * delim = ",";

    std::string in_line2 = line;
    char * token = strtokm(&in_line2[0u], delim);
    strtokm(NULL, delim);
    strtokm(NULL, delim);
    std::string duration = strtokm(NULL, delim);
    std::string start_time = convert_datetime (strtokm(NULL, delim)); 
    std::string start_station_id = strtokm(NULL, delim);
    std::string end_time = convert_datetime (strtokm(NULL, delim)); 
    std::string end_station_id = strtokm(NULL, delim);
    std::string bike_number = strtokm(NULL, delim);
    std::string zip_code = NULL, birth_year = NULL, gender = NULL;
    std::string user_type = strtokm(NULL, delim);
    if (user_type == "Registered")
    {
        zip_code = strtokm(NULL, delim);
        birth_year = strtokm(NULL, delim);
        gender = strtokm(NULL, delim);
    }

    sqlite3_bind_text(stmt, 2, duration.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 3, start_time.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 4, end_time.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 5, start_station_id.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 6, end_station_id.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 7, bike_number.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 8, user_type.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 9, birth_year.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 10, gender.c_str(), -1, SQLITE_TRANSIENT); 
}
