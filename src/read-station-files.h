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

#include <unordered_set>
#include <curl/curl.h>

#include "sqlite3db-utils.h"

int import_to_station_table (sqlite3 * dbcon,
    std::map <std::string, std::string> stationqry);
int import_boston_stations (sqlite3 * dbcon);
int rcpp_import_ch_stns (const char * bikedb, 
        Rcpp::CharacterVector station_files);
std::string import_dc_stations ();
std::map <std::string, std::string> get_dc_stn_table (sqlite3 * dbcon);
std::unordered_set <std::string> get_dc_stn_ids (sqlite3 * dbcon);
int rcpp_import_lo_stns (const char * bikedb, Rcpp::DataFrame stn_data);


//' import_to_station_table
//'
//' Inserts data into the table of stations in the database. Applies to those
//' cities for which station data are included and read as part of the actual
//' raw trips data: ny, boston, la.
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
        std::string name = str_token (&line, ",");
        boost::replace_all (name, "\'", ""); // rm apostrophes
        std::string id = str_token (&line, ",");
        std::string lat = str_token (&line, ",");
        std::string lon = str_token (&line, ",");
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

//' import_dc_stations
//'
//' The Washington DC CapitalBikeShare system does not actually have any station
//' coordinates. These are instead provided by the DC government.  This
//' function reads the contents of the "Captial Bike Share Locations" file into a
//' std::string object used to construct the SQL query that inserts those data
//' into the 'sqlite3' database.
//'
//' @return std::string to be passed directly as SQLite3 query
//'
//' @noRd
std::string import_dc_stations ()
{
    // Step#1 / 3: Use lcurl to download the file
    CURL *curl;
    CURLcode res;
    std::string file_str, line;

    curl_global_init(CURL_GLOBAL_DEFAULT);

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, 
                "https://opendata.arcgis.com/datasets/a1f7acf65795451d89f0a38565a975b3_5.csv"); // nolint
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
    getline (iss, line); // header
    bool first = true;
    while (getline (iss, line))
    {
        std::string name = str_token (&line, ","); 
        name = str_token (&line, ","); // First 2 tokens are junk
        name = str_token (&line, ",");
        boost::replace_all (name, "\'", ""); // rm apostrophes
        std::string id = str_token (&line, ",");
        std::string lat = str_token (&line, ",");
        std::string lon = str_token (&line, ",");
        if (first)
            first = false;
        else
            stationqry += ", ";
        stationqry += "(\'dc\',\'dc" + id + "\'," + lon + "," + lat + ",\'" + 
            name + "\')";
    }
    stationqry += ";";

    return stationqry;
}

//' get_dc_stn_table
//'
//' Because some data files for Washington DC contain only the names of stations
//' and not their ID numbers, a std::map is generated here mapping those names
//' onto IDs for easy insertion into the trips data table.
//'
//' @param dbcon Active connection to SQLite3 database
//'
//' @return std::map of <station name, station ID>
//'
//' @note The map is tiny, so it's okay to return values rather than refs
//'
//' @noRd
std::map <std::string, std::string> get_dc_stn_table (sqlite3 * dbcon)
{
    std::string stn_id, stn_name;
    sqlite3_stmt * stmt;
    std::stringstream ss;
    std::map <std::string, std::string> stn_map;

    char qry_stns [BUFFER_SIZE] = "\0";
    sprintf (qry_stns, 
            "SELECT stn_id, name FROM stations WHERE city = 'dc'");

    int rc = sqlite3_prepare_v2 (dbcon, qry_stns, BUFFER_SIZE, &stmt, NULL);

    while (rc = sqlite3_step (stmt) == SQLITE_ROW)
    {
        const unsigned char * c1 = sqlite3_column_text (stmt, 0);
        ss << c1;
        std::string stn_id = ss.str ();
        ss.str ("");
        c1 = sqlite3_column_text (stmt, 1);
        ss << c1;
        std::string stn_name = ss.str ();
        ss.str ("");
        stn_map [stn_name] = stn_id;
    }

    sqlite3_reset(stmt);

    // Then add additional stations that have names different to those given in
    // the official DC govt data. Note that it's much easier this way than doing
    // any kind of string cleaning or grep searching.
    stn_map ["1st & N ST SE"] = "dc31209"; // Typo: "ST" instead of "St"
    stn_map ["4th St & Rhode Island Ave NE"] = "dc31500"; // 4th & W St NE
    stn_map ["4th & Adams St NE"] = "dc31500"; // 4th & W St NE
    stn_map ["4th St & Massachusetts Ave NW"] = "dc31604"; // 3rd & H St
    stn_map ["5th St & K St NW"] = "dc31600";
    stn_map ["5th & Kennedy St NW"] = "dc31403"; // 4th & Kennedy 
    stn_map ["7th & Water St SW / SW Waterfront"] = "dc31609";
    stn_map ["7th & F St NW / National Portrait Gallery"] = "dc31232";
    stn_map ["8th & F St NW / National Portrait Gallery"] = "dc31232"; // 7 & F 
    stn_map ["11th & K St NW"] = "dc31263"; // 10th & K 
    stn_map ["12th & Hayes St"] = "dc31005";
    stn_map ["12th & Hayes St /  Pentagon City Metro"] = "dc31005"; 
    stn_map ["13th & U St NW"] = "dc31268"; // 12th & U 
    stn_map ["16th & U St NW"] = "dc31229"; // New Hampshire Ave & T St 
    stn_map ["18th & Bell St"] = "dc31007";
    stn_map ["18th & Hayes St"] = "dc31004";
    stn_map ["18th & Wyoming Ave NW"] = "dc31114"; 
    stn_map ["22nd & Eads St"] = "dc31013"; 
    stn_map ["23rd & Eads"] = "dc31013";
    stn_map ["26th & Crystal Dr"] = "dc31012"; // 26th & Clark St 

    // American Land Title Assoc at 18th & M St NW
    stn_map ["Alta Tech Office"] = "dc31221"; 
    stn_map ["Bethesda Ave & Arlington Blvd"] = "dc32002"; // Arington ROAD! 
    stn_map ["Central Library"] = "dc31025"; 
    stn_map ["Connecticut Ave & Nebraska Ave NW"] = "dc31310"; 
    stn_map ["Court House Metro / Wilson Blvd & N Uhle St"] = "dc31089";
    stn_map ["Fallsgove Dr & W Montgomery Ave"] = "dc32016"; // FallsgRove!
    stn_map ["Idaho Ave & Newark St NW"] = "dc31302"; // Wisconsin & Newark
    stn_map ["L'Enfant Plaza / 7th & C St SW"] = "dc31218";
    stn_map ["McPherson Square - 14th & H St NW"] = "dc31216"; // 14th & NY Ave
    stn_map ["McPherson Square / 14th & H St NW"] = "dc31216"; 
    stn_map ["MLK Library/9th & G St NW"] = "dc31274"; // 10th & G 
    stn_map ["N Adams St & Lee Hwy"] = "dc31030"; 
    stn_map ["N Fillmore St & Clarendon Blvd"] = "dc31021"; 
    stn_map ["N Highland St & Wilson Blvd"] = "dc31022";
    stn_map ["N Nelson St & Lee Hwy"] = "dc31092"; 
    stn_map ["N Quincy St & Wilson Blvd"] = "dc31039"; 
    stn_map ["Pentagon City Metro / 12th & Hayes St"] = "dc31005"; 
    stn_map ["Randle Circle & Minnesota Ave NE"] = "dc31702"; // "NE" -> "SE"
    stn_map ["Smithsonian / Jefferson Dr & 12th St SW"] = "dc31248"; 
    stn_map ["Thomas Jefferson Cmty Ctr / 2nd St S & Ivy"] = "dc31074"; 
    stn_map ["Virginia Square"] = "dc31024"; 
    stn_map ["Wisconsin Ave & Macomb St NW"] = "dc31302"; // Wisconsin & Newark 

    // Next ones are uncertain:
    stn_map ["20th & Bell St"] = "dc31007"; // 18th & Bell - or 31002?
    stn_map ["15th & Hayes St"] = "dc31004"; // 18th & Hayes - or 31005?
    // 31702 is randle circle, which is 700m away, but nearest nonetheless
    stn_map ["34th St & Minnesota Ave SE"] = "dc31702"; 
    stn_map ["Fairfax Dr & Glebe Rd"] = "dc31038"; // glebe & 11th - or 31049?
    // King St Metro N is much closer to station than S = 31048:
    stn_map ["King St Metro"] = "dc31098"; 
    stn_map ["S Abingdon St & 36th St S"] = "dc31064"; // or 31908?

    // next two are excluded for obvious reasons
    stn_map ["Alta Bicycle Share Demonstration Station"] = "00000";
    stn_map ["Birthday Station"] = "00000";
    // And this one because i've no idea where it is, and is only 5 or so trips
    stn_map ["19th & New Hampshire Ave NW"] = "00000";

    return stn_map;
}

//' get_dc_stn_ids
//'
//' Returns vector of all station IDs in the official DC Govt file. Only
//' trips from and to stations with codes in this file are loaded into db.
//'
//' @param dbcon Active connection to SQLite3 database
//'
//' @return std::unordered_set of <std::string station ID>
//'
//' @note The map is tiny, so it's okay to return values rather than refs
//'
//' @noRd
std::unordered_set <std::string> get_dc_stn_ids (sqlite3 * dbcon)
{
    std::string stn_id, stn_name;
    sqlite3_stmt * stmt;
    std::stringstream ss;
    std::unordered_set <std::string> stn_ids;

    char qry_stns [BUFFER_SIZE] = "\0";
    sprintf (qry_stns, 
            "SELECT stn_id FROM stations WHERE city = 'dc'");

    int rc = sqlite3_prepare_v2 (dbcon, qry_stns, BUFFER_SIZE, &stmt, NULL);

    while (rc = sqlite3_step (stmt) == SQLITE_ROW)
    {
        const unsigned char * c1 = sqlite3_column_text (stmt, 0);
        ss << c1;
        std::string stn_id = ss.str ();
        stn_ids.insert (stn_id);
        ss.str ("");
    }

    sqlite3_reset(stmt);

    return stn_ids;
}

//' rcpp_import_ch_stns
//'
//' @param bikedb Name of SQLite3 database
//' @param stationqry Station query constructed during reading of data 
//'
//' @return Number of stations to potentially be added to stations table (if not
//'         already there).
//'
//' @noRd
// [[Rcpp::export]]
int rcpp_import_ch_stns (const char* bikedb, 
        Rcpp::CharacterVector station_files)
{
    sqlite3 *dbcon;
    char *zErrMsg = 0;

    int rc = sqlite3_open_v2(bikedb, &dbcon, SQLITE_OPEN_READWRITE, NULL);
    if (rc != SQLITE_OK)
        throw std::runtime_error ("Can't establish sqlite3 connection");

    std::map <std::string, std::string> stationqry;
    char in_line [BUFFER_SIZE] = "\0";
    const char * delim;
    delim = ",";

    int num_stns_old = get_stn_table_size (dbcon);

    for(unsigned int filenum = 0; filenum < station_files.length(); filenum++) 
    {
        FILE * pFile = fopen (station_files [filenum], "r");
        char * junk = fgets (in_line, BUFFER_SIZE, pFile);
        rm_dos_end (in_line);
        while (fgets (in_line, BUFFER_SIZE, pFile) != NULL) 
        {
            char * token = std::strtok (in_line, delim); // stn id
            std::string stn_id = token;
            if (stationqry.find (stn_id) == stationqry.end ())
            {
                std::string stn_name = std::strtok (NULL, delim);
                std::string stn_lat = std::strtok (NULL, delim);
                std::string stn_lon = std::strtok (NULL, delim);
                // Note: ch also pre-pended to station id
                stationqry [stn_id] = "(\'ch\',\'ch" + stn_id + "\',\'" + 
                    stn_name + "\'," + stn_lon + "," + stn_lat + ")";
            }
        }
    } // end for filenum over station_files

    std::string fullstationqry = "INSERT OR IGNORE INTO stations "
        "(city, stn_id, name, latitude, longitude) VALUES ";
    fullstationqry += stationqry.begin ()->second;
    for (auto stn = std::next (stationqry.begin ());
            stn != stationqry.end (); ++stn)
    {
        fullstationqry += ", " + stn->second;
    }
    fullstationqry += ";";

    rc = sqlite3_exec(dbcon, fullstationqry.c_str(), NULL, 0, &zErrMsg);
    if (rc != SQLITE_OK)
        throw std::runtime_error ("Unable to insert Chicago stations");

    rc = sqlite3_close_v2(dbcon);
    if (rc != SQLITE_OK)
        throw std::runtime_error ("Unable to close sqlite database");

    return get_stn_table_size (dbcon) - num_stns_old;
}


//' rcpp_import_lo_stns
//'
//' @param dbcon Active connection to sqlite3 database
//' @param stationqry Station query constructed during reading of data 
//'
//' @return Number of stations to potentially be added to stations table (if not
//'         already there).
//'
//' @noRd
// [[Rcpp::export]]
int rcpp_import_lo_stns (const char * bikedb, Rcpp::DataFrame stn_data)
{
    sqlite3 *dbcon;
    char *zErrMsg = 0;

    int rc = sqlite3_open_v2 (bikedb, &dbcon, SQLITE_OPEN_READWRITE, NULL);
    if (rc != SQLITE_OK)
        throw std::runtime_error ("Can't establish sqlite3 connection");

    std::string stationqry = "INSERT OR IGNORE INTO stations "
        "(city, stn_id, name, longitude, latitude) VALUES ";

    int num_stns_old = get_stn_table_size (dbcon);

    Rcpp::CharacterVector stn_id = stn_data ["id"];
    Rcpp::CharacterVector stn_name = stn_data ["name"];
    Rcpp::CharacterVector stn_lon = stn_data ["lon"];
    Rcpp::CharacterVector stn_lat = stn_data ["lat"];

    for (unsigned i = 0; i<stn_data.nrow (); i++)
    {
        stationqry += "(\'lo\',\'lo" + stn_id (i) + "\',\'" + stn_name (i) + 
            "\'," + stn_lon (i) + "," + stn_lat (i) + ")";
        if (i < (stn_data.nrow () - 1))
            stationqry += ",";
    }
    stationqry += ";";

    rc = sqlite3_exec(dbcon, stationqry.c_str(), NULL, 0, &zErrMsg);
    if (rc != SQLITE_OK)
        throw std::runtime_error ("Unable to insert London stations");

    int num_stns_added = get_stn_table_size (dbcon) - num_stns_old;

    rc = sqlite3_close_v2(dbcon);
    if (rc != SQLITE_OK)
        throw std::runtime_error ("Unable to close sqlite database");

    return num_stns_added;
}
