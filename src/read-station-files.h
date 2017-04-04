#pragma once
/***************************************************************************
 *  Project:    bikedata
 *  File:       read-station-files.h
 *  Language:   C++
 *
 *  Author:     Mark Padgham 
 *  E-Mail:     mark.padgham@email.com 
 *
 *  Description:    Routines to read and store data on bike docking stations in
 *                  the stations table of the SQLite3 database.
 *
 *  Compiler Options:   -std=c++11
 ***************************************************************************/

#include <curl/curl.h>

#include "sqlite3db-utils.h"

int import_to_station_table (sqlite3 * dbcon,
    std::map <std::string, std::string> stationqry);
int import_boston_stations (sqlite3 * dbcon);


//' import_to_station_table
//'
//' Inserts data into the table of stations in the database
//' 
//' @param dbcon Active connection to sqlite3 database
//' @param stationqry Station query constructed during reading of data with
//'        rcpp_import_to_trip_table ()
//'
//' @return integer result code
//'
//' @noRd
int import_to_station_table (sqlite3 * dbcon,
    std::map <std::string, std::string> stationqry)
{
    char *zErrMsg = 0;
    int rc;

    int n = 0;

    // http://stackoverflow.com/questions/19337029/insert-if-not-exists-statement-in-sqlite
    std::string fullstationqry = "INSERT OR IGNORE INTO stations "
        "(city, stn_id, name, latitude, longitude) VALUES ";
    fullstationqry += stationqry.begin ()->second;
    for (auto thisstation = std::next (stationqry.begin ());
            thisstation != stationqry.end (); ++thisstation)
    {
        fullstationqry += ", " + thisstation->second;
    }
    fullstationqry += ";";

    rc = sqlite3_exec(dbcon, fullstationqry.c_str(), NULL, 0, &zErrMsg);
    if (rc != SQLITE_OK)
        throw std::runtime_error ("Unable to insert stations into station table");

    std::string qry = "SELECT AddGeometryColumn"
                      "('stations', 'geom', 4326, 'POINT', 'XY');";
    rc = sqlite3_exec(dbcon, qry.c_str (), NULL, 0, &zErrMsg);

    qry = "UPDATE stations SET geom = MakePoint(longitude, latitude, 4326);";
    rc = sqlite3_exec(dbcon, qry.c_str (), NULL, 0, &zErrMsg);

    return rc;
}

//' import_boston_stations
//'
//' The Boston Hubway system has a separate \code{.csv} table with station data.
//' This function reads the contents of that file into a std::string object used
//' to construct the SQL query that inserts those data into the ' sqlite3
//' database.
//'
//' @param stationqry Station query constructed during reading of data 
//'
//' @return Number of stations in the Hubway system
//'
//' @note This station table is actually useless, because the station ID values
//' given do not match those used in the raw data files! The latter are simple
//' integer codes, while IDs in the "official" \code{.csv} file are like
//' "A32000" - all beginning with an alpha and then five digits. These codes
//' do not appear anywhere in the trip data files, and so this whole function is
//' not used. It is nevertheless kept for the plausible day when the Hubway folk
//' fix up this inconsistency.
//'
//' Note also that the curl dependency is only required for this function, so
//' this might actually be better being deleted? It can always be recovered in
//' the git tree if needed later.
//'
//' @noRd
int import_boston_stations (sqlite3 * dbcon)
{
    // Step#1 / 3: Use lcurl to download the file
    CURL *curl;
    CURLcode res;
    std::string file_str, line;

    curl_global_init(CURL_GLOBAL_DEFAULT);

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, 
                "https://s3.amazonaws.com/hubway-data/Hubway_Stations_2011_2016.csv");
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); 
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L); 
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, 
                CurlWrite_CallbackFunc_StdString);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &file_str);

        res = curl_easy_perform(curl);

        if(res != CURLE_OK)
            throw std::runtime_error ("curl_easy_perform() failed");

        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();

    std::string stationqry = "INSERT OR IGNORE INTO stations "
        "(city, stn_id, longitude, latitude, name) VALUES ";
    std::istringstream iss (file_str);
    int count = 0;
    int num_stns_old = get_stn_table_size (dbcon);
    getline (iss, line); // header
    bool first = true;
    while (getline (iss, line))
    {
        std::string name = str_token (&line);
        boost::replace_all (name, "\'", ""); // rm apostrophes
        std::string id = str_token (&line);
        std::string lat = str_token (&line);
        std::string lon = str_token (&line);
        if (first)
            first = false;
        else
            stationqry += ", ";
        stationqry += "(\'bo\',\'" + id + "\'," + lon + "," + lat + ",\'" + 
            name + "\')";
        count++;
    }
    stationqry += ";";
    char *zErrMsg = 0;

    int rc = sqlite3_exec (dbcon, stationqry.c_str(), NULL, 0, &zErrMsg);
    if (rc != SQLITE_OK)
        throw std::runtime_error ("Unable to insert Boston stations");

    return get_stn_table_size (dbcon) - num_stns_old;
}
