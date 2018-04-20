/***************************************************************************
 *  Project:    bikedata
 *  File:       splite3db-add-data.cpp
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

#include "sqlite3db-add-data.h"


/***************************************************************************
 *
 * EXTENDED DESCRIPTIONS
 * 
 * The SQLite3 database has the following 15 fields and column numbers:
 *    | number | field                   |
 *    | ----   | ----------------------- |
 *    | 0      | duration                |
 *    | 1      | start_time              |
 *    | 2      | end_time                |
 *    | 3      | start_station_id        |
 *    | 4      | start_station_name      |
 *    | 5      | start_station_latitude  |
 *    | 6      | start_station_longitude |
 *    | 7      | end_station_id          |
 *    | 8      | end_station_name        |
 *    | 9      | end_station_latitude    |
 *    | 10     | end_station_longitude   |
 *    | 11     | bike_id                 |
 *    | 12     | user_type               |
 *    | 13     | birth_year              |
 *    | 14     | gender                  |
 * The value of 15 is held in common.h/num_db_fields = 15
 *
 * Each data field is examined to map its structure on to these fields. First
 * the header is examined to fill the "position_file2db" vector. The first of
 * these vectors has length equal to the number of fields in the actual file,
 * with values then identifying which of the above fields is recorded at that
 * position.  Non-existent fields are coded -1. A "quoted" vector is also
 * constructed to indicate whether or not each field is embedded in quotes.
 *
 * Having established these header structures (in a HeaderStruct object),
 * reading a file then requires assembling a vector of string objects.
 *
 ***************************************************************************/

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
//' @param quiet If FALSE (0), progress is displayed on screen
//'
//' @return integer result code
//'
//' @noRd
// [[Rcpp::export]]
int rcpp_import_to_trip_table (const char* bikedb, 
        Rcpp::CharacterVector datafiles, std::string city,
        std::string header_file_name, bool data_has_stations, bool quiet)
{
    sqlite3 *dbcon;
    char *zErrMsg = nullptr;
    const char *zVfs = nullptr;
    size_t rc;

    rc = static_cast <size_t> (sqlite3_open_v2(bikedb, &dbcon, SQLITE_OPEN_READWRITE, zVfs));
    //rc = static_cast <size_t> (sqlite3_open(bikedb, &dbcon));
    if (rc != SQLITE_OK)
        throw std::runtime_error ("Can't establish sqlite3 connection");

    // dc stations have to be initially imported because for 3.5 years only
    // station addresses were given with no IDs. The stations table is needed in
    // these cases to extract the right IDs.
    // --> The stations are now hard-coded in R/sysdata.rda because the
    //     opendata.arcgis.com is too unreliable.
    // A stn_map is now also needed for Boston, because they've changed to
    // annual dumps for pre-2015, yet some trip files have only names and not
    // the station IDs in the station files now provided.
    std::map <std::string, std::string> stn_map;
    if (city == "dc")
    {
        stn_map = get_dc_stn_table (dbcon);
    } else if (city == "bo")
    {
        stn_map = get_bo_stn_table (dbcon);
    }

    FILE * pFile;
    char in_line [BUFFER_SIZE] = "\0";
    char sqlqry [BUFFER_SIZE] = "\0";

    sqlite3_stmt * stmt;
    std::map <std::string, std::string> stationqry;

    int ntrips = 0; // ntrips is added in this call

    sprintf(sqlqry, "INSERT INTO trips VALUES (NOT NULL, @CI, @TD, @ST, @ET, @SSID, @ESID, @BID, @UT, @BY, @GE)");

    sqlite3_prepare_v2(dbcon, sqlqry, BUFFER_SIZE, &stmt, nullptr);

    sqlite3_exec(dbcon, "BEGIN TRANSACTION", nullptr, nullptr, &zErrMsg);
    sqlite3_free (zErrMsg);

    for(int filenum = 0; filenum < datafiles.length(); filenum++) 
    {
        Rcpp::checkUserInterrupt ();
        if (!quiet)
            Rcpp::Rcout << "reading file " << filenum + 1 << "/" <<
                datafiles.size() << ": " <<
                datafiles [filenum] << std::endl;

        std::string filename_i = Rcpp::as <std::string> (datafiles [filenum]);
        HeaderStruct headers = get_field_positions (filename_i,
                header_file_name, data_has_stations, city);

        pFile = fopen (datafiles [filenum], "r");
        char * junk = fgets (in_line, BUFFER_SIZE, pFile);
        (void) junk; // suppress unused variable warning
        rm_dos_end (in_line);

        // One London file ("21JourneyDataExtract31Aug2016-06Sep2016.csv") has
        // "Start/EndStation Logical Terminal" numbers instead of IDs.  These
        // don't map on to any known station numbers and so can't be used.
        if (city == "lo")
        {
            std::string in_line2 = in_line;
            if (in_line2.find ("Logical Terminal") != std::string::npos)
                continue; // skip rest of that loop
        }

        bool get_structure = true;
        unsigned int temp = 0; // TODO: Delete that!
        while (fgets (in_line, BUFFER_SIZE, pFile) != nullptr) 
        {
            if (get_structure)
            {
                get_field_quotes (in_line, headers);
                get_structure = false;
                //dump_headers (headers);
            }

            rm_dos_end (in_line);
            sqlite3_bind_text (stmt, 1, city.c_str (), -1, SQLITE_TRANSIENT); 

            // London, LA, and Philly data are ballsed up and change format
            // within data files, so they are read with their own std::string
            // routines, rather than then generic char * routine.
            if (city == "lo")
                rc = read_one_line_london (stmt, in_line);
            else if (city == "la" || city == "ph")
                rc = read_one_line_nabsa (stmt, in_line, &stationqry, city);
            else
                rc = read_one_line_generic (stmt, in_line, &stationqry, city,
                        headers, stn_map);
            temp++;
            if (rc == 0) // only != 0 for LA, London, Boston, and MN
            {
                ntrips++;
                sqlite3_step (stmt);
            }
            sqlite3_reset (stmt);
        }
    }
    sqlite3_finalize(stmt);

    sqlite3_exec(dbcon, "END TRANSACTION", nullptr, nullptr, &zErrMsg);
    sqlite3_free (zErrMsg);

    if (city == "ny" || city == "la" || city == "ph" || city == "sf")
        import_to_station_table (dbcon, stationqry);

    rc = static_cast <size_t> (sqlite3_close_v2 (dbcon));
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
    char *zErrMsg = nullptr;
    int rc;

    rc = sqlite3_open_v2(bikedb, &dbcon, SQLITE_OPEN_READWRITE, nullptr);
    if (rc != SQLITE_OK)
        throw std::runtime_error ("Can't establish sqlite3 connection");

    for (auto i : datafiles)
    {
        std::string datafile_qry = "INSERT INTO datafiles "
                                   "(id, city, name) VALUES (";
        datafile_qry += std::to_string (nfiles) + ",\"" + city + "\",\"" + 
            i + "\");";

        const char *sql = datafile_qry.c_str ();
        rc = sqlite3_exec(dbcon, sql, nullptr, nullptr, &zErrMsg);
        sqlite3_free (zErrMsg);
        //rc = sqlite3_exec(dbcon, datafile_qry.c_str(), nullptr, nullptr, &zErrMsg);
        if (rc == 0)
            nfiles++;
    }

    rc = sqlite3_close_v2(dbcon);
    //rc = sqlite3_close(dbcon);
    if (rc != SQLITE_OK)
        throw std::runtime_error ("Unable to close sqlite database");
    sqlite3_free (zErrMsg);

    return nfiles;
}

//' Examine the header line of the data file to map the records on to the
//' corresponding columns in the database. The database has the following fields
//' and column numbers:
//'    | number | field                   |
//'    | ----   | ----------------------- |
//'    | 0      | duration                |
//'    | 1      | start_time              |
//'    | 2      | end_time                |
//'    | 3      | start_station_id        |
//'    | 4      | start_station_name      |
//'    | 5      | start_station_latitude  |
//'    | 6      | start_station_longitude |
//'    | 7      | end_station_id          |
//'    | 8      | end_station_name        |
//'    | 9      | end_station_latitude    |
//'    | 10     | end_station_longitude   |
//'    | 11     | bike_id                 |
//'    | 12     | user_type               |
//'    | 13     | birth_year              |
//'    | 14     | gender                  |
//' The HeaderStruct has vectors for "position" and "quoted". Each of these has
//' the same length as the number of entries in the actual file (not necessarily
//' equal to "num_db_fields = 15"), with "position" mapping each entry on to its
//' corresponding position in the database, and using -1 to denote no
//' corresponding field.
//' @noRd
HeaderStruct get_field_positions (const std::string fname,
        const std::string header_file_name, bool data_has_stations,
        const std::string city)
{
    std::ifstream in_file;
    // load file header variants - this file is very small, so no real loss
    // doing this repeatedly here. Note that this is where the R 1-indexed
    // positions are re-mapped to 0-indexed C++ versions.
    in_file.open (header_file_name.c_str (), std::ios_base::in);
    // field_name_map is int coz it's from headers.position_file2db which uses
    // -1 to flag no position
    std::unordered_map <std::string, int> field_name_map;
    std::string line;
    getline (in_file, line, '\n'); // header

    while (getline (in_file, line, '\n'))
    {
        size_t ipos = line.find (",");
        //std::string f1 = line.substr (0, ipos); // generic name: not used
        line = line.substr (ipos + 1, line.length () - ipos - 1);
        ipos = line.find (",");
        std::string f2 = line.substr (0, ipos);
        line = line.substr (ipos + 1, line.length () - ipos - 1);
        ipos = line.find (",");
        int indx = atoi (line.substr (0, ipos).c_str ());
        line = line.substr (ipos + 1, line.length () - ipos - 1);
        // line is then the city
        if (line.find ("all") != std::string::npos ||
                line.find (city) != std::string::npos)
            field_name_map.emplace (f2, indx - 1);
    }
    in_file.close ();

    in_file.open (fname.c_str(), std::ios_base::in);
    getline (in_file, line, '\n');
    // remove all quotes, whitespace, underscores, and convert to lower:
    boost::replace_all (line, "\"", "");
    boost::replace_all (line, " ", "");
    boost::replace_all (line, "_", "");
    boost::replace_all (line, ".", ""); // csv test files replace " " with "."
    boost::replace_all (line, "\n","");
    boost::replace_all (line, "\r","");
    // Note that this only works because all systems to date are from the
    // English-speaking world; see
    // https://stackoverflow.com/questions/313970/how-to-convert-stdstring-to-lower-case
    std::transform (line.begin (), line.end (), line.begin (), ::tolower);
    // Alternative
    /*
    std::locale loc;
    for (std::string::size_type i = 0; i < line.length (); i++)
        line [i] = std::tolower (line [i], loc);
    */
    unsigned int len = static_cast <unsigned int> (
            std::count (line.begin (), line.end (), ','));
    
    HeaderStruct headers;
    headers.data_has_stations = data_has_stations;
    headers.position_file2db.resize (len + 1); // one more fields than commas
    std::fill (headers.position_file2db.begin (), headers.position_file2db.end (), -1);

    for (unsigned int i = 0; i < len; i++)
    {
        size_t ipos = static_cast <size_t> (line.find (","));
        std::string field = line.substr (0, ipos);
        if (field_name_map.find (field) != field_name_map.end ())
        {
            headers.position_file2db [i] = field_name_map.at (field);
        }
        line = line.substr (ipos + 1, line.length () - ipos - 1);
    }
    if (field_name_map.find (line) != field_name_map.end ())
    {
        headers.position_file2db [len] = field_name_map.at (line);
    }

    headers.nvalues = len + 1;

    return headers;
}

//' get_field_quotes
//'
//' The quotation structure of the header line does not always reflect the
//' actual structure of the data, so the patterns of quotations are determined by
//' the first data line rather than in "get_field_positions".
//' @noRd
void get_field_quotes (const std::string line, HeaderStruct &headers)
{
    std::string l = line;
    headers.quoted.resize (headers.position_file2db.size ());
    for (unsigned int i = 0; i < (headers.nvalues - 1); i++)
    {
        if (l.find ("\"") < l.find (","))
            headers.quoted [i] = true;
        else
            headers.quoted [i] = false;

        size_t ipos;
        if (headers.quoted [i])
        {
            ipos = l.find ("\",");
            l = l.substr (ipos + 2, l.length () - ipos - 2);
        } else
        {
            ipos = l.find (",");
            l = l.substr (ipos + 1, l.length () - ipos - 1);
        }
    }
    if (l.find ("\"") == std::string::npos)
    {
        headers.quoted [headers.nvalues - 1] = false;
        headers.terminal_quote = false;
    } else
    {
        headers.quoted [headers.nvalues - 1] = true;
        headers.terminal_quote = true;
    }
}

void dump_headers (const HeaderStruct &headers)
{
    Rcpp::Rcout << "Header has " << headers.nvalues <<
        " with [quoted, pos_file2db, _db2file] having [" <<
        headers.quoted.size () << ", " << headers.position_file2db.size () <<
         "]" << std::endl;

    for (size_t i = 0; i < headers.position_file2db.size (); i++)
    {
        Rcpp::Rcout << "pos [" << i << "] = " <<
            headers.position_file2db [i];
        if (headers.quoted [i])
            Rcpp::Rcout << " is quoted" << std::endl;
        else
            Rcpp::Rcout << " is NOT quoted" << std::endl;
    }
}
