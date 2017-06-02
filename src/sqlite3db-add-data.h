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

#include <sstream>
#include <unordered_set>
#include <boost/algorithm/string/replace.hpp>

// [[Rcpp::depends(BH)]]
#include <Rcpp.h>
#include "utils.h"
#include "read-city-files.h"
#include "read-station-files.h"
#include "sqlite3db-utils.h"
#include "vendor/sqlite3/sqlite3.h"

int rcpp_import_to_trip_table (const char* bikedb, 
        Rcpp::CharacterVector datafiles, std::string city, bool quiet);
int rcpp_import_to_file_table (const char * bikedb,
        Rcpp::CharacterVector datafiles, std::string city);

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

    // dc stations have to be initially imported because for 3.5 years only
    // station addresses were given with no IDs. The stations table is needed in
    // these cases to extract the right IDs.
    // --> The stations are now hard-coded in R/sysdata.rda because the
    //     opendata.arcgis.com is too unreliable.
    std::map <std::string, std::string> dc_stn_map;
    std::unordered_set <std::string> dc_stn_ids;
    if (city == "dc")
    {
        /*
        std::string dc_stn_qry = import_dc_stations ();
        rc = sqlite3_exec (dbcon, dc_stn_qry.c_str(), NULL, 0, &zErrMsg);
        if (rc != SQLITE_OK)
            throw std::runtime_error ("Unable to insert Washington DC stations");
        */
        dc_stn_map = get_dc_stn_table (dbcon);
        dc_stn_ids = get_dc_stn_ids (dbcon);
    }

    FILE * pFile;
    char in_line [BUFFER_SIZE] = "\0";
    char sqlqry [BUFFER_SIZE] = "\0";

    sqlite3_stmt * stmt;
    std::map <std::string, std::string> stationqry;

    int ntrips = 0; // ntrips is # added in this call

    sprintf(sqlqry, "INSERT INTO trips VALUES (NOT NULL, @CI, @TD, @ST, @ET, @SSID, @ESID, @BID, @UT, @BY, @GE)");

    sqlite3_prepare_v2(dbcon, sqlqry, BUFFER_SIZE, &stmt, NULL);

    sqlite3_exec(dbcon, "BEGIN TRANSACTION", NULL, NULL, &zErrMsg);

    for(int filenum = 0; filenum < datafiles.length(); filenum++) 
    {
        if (!quiet)
            Rcpp::Rcout << "reading file " << filenum + 1 << "/" <<
                datafiles.size() << ": " <<
                datafiles [filenum] << std::endl;

        pFile = fopen (datafiles [filenum], "r");
        char * junk = fgets (in_line, BUFFER_SIZE, pFile);
        (void) junk; // suppress unused variable warning
        rm_dos_end (in_line);

        // DC files change structure a lot, with explicit station ID fields only
        // appearing in later files, and files changing order from 2013Q3 with
        // (Start date, End date, Start Station, End Station) to 2013Q4 with
        // (Start date, Start Station, End date, End Station)
        bool id_in_dc_file = false, dc_end_date_first = true;
        // One London file ("21JourneyDataExtract31Aug2016-06Sep2016.csv") has
        // "Start/EndStation Logical Terminal" numbers instead of IDs.  These
        // don't map on to any known station numbers and so can't be used.
        bool london_is_junk = false;

        if (city == "dc")
        {
            std::string in_line2 = in_line;
            if (in_line2.find ("station number") != std::string::npos)
                id_in_dc_file = true;
            size_t ss_pos = in_line2.find ("Start Station");
            if (ss_pos == std::string::npos)
                ss_pos = in_line2.find ("Start station");
            if (ss_pos == std::string::npos)
                throw std::runtime_error ("DC trip file has no Start Station field"); //nolint

            if (in_line2.find ("End date", 0) > ss_pos)
                dc_end_date_first = false;
        } else if (city == "lo")
        {
            std::string in_line2 = in_line;
            if (in_line2.find ("Logical Terminal") != std::string::npos)
                london_is_junk = true;
        }

        const char * delim;
        // both ny and chicago sometimes place fields in double quotes,
        // sometimes not.
        if (line_has_quotes (in_line))
            delim = "\",\"";
        else
            delim = ",";

        if (!london_is_junk)
        {
            while (fgets (in_line, BUFFER_SIZE, pFile) != NULL) 
            {
                rm_dos_end (in_line);
                sqlite3_bind_text (stmt, 1, city.c_str (), -1, SQLITE_TRANSIENT); 

                if (city == "ny")
                    rc = read_one_line_nyc (stmt, in_line, &stationqry, delim);
                else if (city == "bo")
                    rc = read_one_line_boston (stmt, in_line, &stationqry);
                else if (city == "ch")
                    rc = read_one_line_chicago (stmt, in_line, delim);
                else if (city == "dc")
                    rc = read_one_line_dc (stmt, in_line, dc_stn_map, dc_stn_ids,
                            id_in_dc_file, dc_end_date_first);
                else if (city == "lo")
                    rc = read_one_line_london (stmt, in_line);
                else if (city == "la" || city == "ph")
                    rc = read_one_line_nabsa (stmt, in_line, &stationqry, city);

                if (rc == 0) // only != 0 for LA and London
                {
                    ntrips++;
                    sqlite3_step (stmt);
                }
                sqlite3_reset (stmt);
            }
        }
    }

    sqlite3_exec(dbcon, "END TRANSACTION", NULL, NULL, &zErrMsg);

    if (city == "ny" || city == "bo" || city == "la" || city == "ph")
        import_to_station_table (dbcon, stationqry);

    rc = sqlite3_close_v2 (dbcon);
    if (rc != SQLITE_OK)
        throw std::runtime_error ("Unable to close sqlite database");

    return ntrips;
}


//' rcpp_import_to_file_table
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
int rcpp_import_to_file_table (const char * bikedb,
        Rcpp::CharacterVector datafiles, std::string city, int nfiles)
{
    sqlite3 *dbcon;
    char *zErrMsg = 0;
    int rc;

    rc = sqlite3_open_v2(bikedb, &dbcon, SQLITE_OPEN_READWRITE, NULL);
    if (rc != SQLITE_OK)
        throw std::runtime_error ("Can't establish sqlite3 connection");

    for (auto i : datafiles)
    {
        std::string datafile_qry = "INSERT INTO datafiles "
                                   "(id, city, name) VALUES (";
        datafile_qry += std::to_string (nfiles) + ",\"" + city + "\",\"" + 
            i + "\");";

        rc = sqlite3_exec(dbcon, datafile_qry.c_str(), NULL, 0, &zErrMsg);
        if (rc == 0)
            nfiles++;
    }

    rc = sqlite3_close_v2(dbcon);
    if (rc != SQLITE_OK)
        throw std::runtime_error ("Unable to close sqlite database");

    return nfiles;
}
