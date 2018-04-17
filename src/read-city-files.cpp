/***************************************************************************
 *  Project:    bikedata
 *  File:       read-city-files
 *  Language:   C++
 *
 *  Author:     Mark Padgham 
 *  E-Mail:     mark.padgham@email.com 
 *
 *  Description:    Routines to read single lines of the data files for
 *                  different cities.
 *
 *  Compiler Options:   -std=c++11
 ***************************************************************************/

#include "read-city-files.h"


/***************************************************************************
 *  This maps the data onto the fields defined in headers which follow this
 *  structure, as detailed in, and derived from, data-raw/sysdata.Rmd
 *  
 *  | number | field                   |
 *  | ----   | ----------------------- |
 *  | 0      | duration                |
 *  | 1      | start_time              |
 *  | 2      | end_time                |
 *  | 3      | start_station_id        |
 *  | 4      | start_station_name      |
 *  | 5      | start_station_latitude  |
 *  | 6      | start_station_longitude |
 *  | 7      | end_station_id          |
 *  | 8      | end_station_name        |
 *  | 9      | end_station_latitude    |
 *  | 10     | end_station_longitude   |
 *  | 11     | bike_id                 |
 *  | 12     | user_type               |
 *  | 13     | birth_year              |
 *  | 14     | gender                  |
 * 
 ***************************************************************************/

unsigned int read_one_line_generic (sqlite3_stmt * stmt, char * line,
        std::map <std::string, std::string> * stationqry,
        const std::string city, const HeaderStruct &headers, bool dump)
{
    const char * delim_noq_noq = ",";
    const char * delim_noq_q = ",\"";
    const char * delim_q_noq = "\",";
    const char * delim_q_q = "\",\"";

    std::string linestr = line;
    if (headers.terminal_quote)
        boost::replace_all (linestr, "\\N", "\"\"");
    else
        boost::replace_all (linestr, "\\N", "");

    std::vector <std::string> values (num_db_fields);
    std::fill (values.begin (), values.end (), "\"\"");
    // first field has to be done separately to feed the startline linestr
    // pointer
    char * token;
    if (headers.quoted [0])
    {
        token = strtokm (&linestr[0u], "\""); // opening quote
        if (headers.quoted [1])
            token = strtokm (nullptr, delim_q_q);
        else
            token = strtokm (nullptr, delim_q_noq);
        (void) token; // suppress unused variable warning
    } else
    {
        if (headers.quoted [1])
            token = strtokm (&linestr[0u], delim_noq_q);
        else
            token = strtokm (&linestr[0u], delim_noq_noq);
    }
    if (headers.position_file2db [0] >= 0)
        values [headers.position_file2db [0]] = token;
    if (dump)
            Rcpp::Rcout << "values [0 -> " <<
                headers.position_file2db [0] << "] = " << token << std::endl;

    int pos = headers.position_file2db [0];
    if (pos == 1 || pos == 2) // happens for MN
    {
        if (values [pos].length () == 0)
            return 1;
        values [pos] = convert_datetime_generic (values [pos]);
    }

    // These don't arise in any data processed to date
    if (pos == 3 || pos == 7) // add city prefixes to station names
        values [pos] = city + values [pos];
    if (pos == 12) // user type
        values [pos] = convert_usertype (values [pos]);
    if (pos == 14) // gender
        values [pos] = convert_gender (values [pos]);

    for (unsigned int i = 1; i < headers.nvalues; i++)
    {
        int pos = headers.position_file2db [i];
        if (dump)
            Rcpp::Rcout << "values [" << i << " -> " << pos << "] with ";
        if (headers.quoted [i])
        {
            if (headers.quoted [i + 1])
            {
                if (dump)
                    Rcpp::Rcout << "delim_q_q ";
                token = strtokm (nullptr, delim_q_q);
            }
            else
            {
                if (dump)
                    Rcpp::Rcout << "delim_q_noq ";
                token = strtokm (nullptr, delim_q_noq);
            }
        } else
        {
            if (headers.quoted [i + 1])
            {
                if (dump)
                    Rcpp::Rcout << "delim_noq_q ";
                token = strtokm (nullptr, delim_noq_q);
            }
            else
            {
                if (dump)
                    Rcpp::Rcout << "delim_noq_noq ";
                token = strtokm (nullptr, delim_noq_noq);
            }
        }
        if (dump)
            Rcpp::Rcout << "[" << i << " / " << headers.nvalues << "] = " <<
                token << std::endl;
        std::string tks = token;
        // sometimes (in London) string that should be quoted yet are missing
        // have no empty quotes, and so the parsing is mucked up. This
        // nevertheless always leaves empty commas at the start, so
        if (tks.substr (0, 1) == ",")
            return 1;
        if (i == (headers.nvalues - 1) && headers.terminal_quote)
            boost::replace_all (tks, "\"", "");

        if (pos >= 0)
        {
            values [pos] = tks;

            if (pos == 1 || pos == 2)
            {
                // some London files have missing datetime strings:
                if (values [pos].length () == 0)
                    return 1;
                values [pos] = convert_datetime_generic (values [pos]);
            }

            if (pos == 3 || pos == 7) // add city prefixes to station names
                values [pos] = city + values [pos];

            if (pos == 12) // user type
                values [pos] = convert_usertype (values [pos]);

            if (pos == 14) // gender
                values [pos] = convert_gender (values [pos]);
        }
        if (dump)
            Rcpp::Rcout << std::endl;
    }

    if (values [0] == "\"\"")
        values [0] = std::to_string (timediff (values [1], values [2]));

    // Then bind the SQLITE statement
    // duration
    sqlite3_bind_text(stmt, 2, values [0].c_str (), -1, SQLITE_TRANSIENT);
    // starttime
    sqlite3_bind_text(stmt, 3, values [1].c_str (), -1, SQLITE_TRANSIENT);
    // endtime
    sqlite3_bind_text(stmt, 4, values [2].c_str (), -1, SQLITE_TRANSIENT);
    // startid
    sqlite3_bind_text(stmt, 5, values [3].c_str (), -1, SQLITE_TRANSIENT);
    // endid
    sqlite3_bind_text(stmt, 6, values [7].c_str (), -1, SQLITE_TRANSIENT);
    // bikeid
    sqlite3_bind_text(stmt, 7, values [11].c_str (), -1, SQLITE_TRANSIENT);
    // user
    sqlite3_bind_text(stmt, 8, values [12].c_str (), -1, SQLITE_TRANSIENT);
    // birthyear
    sqlite3_bind_text(stmt, 9, values [13].c_str (), -1, SQLITE_TRANSIENT);
    // gender
    sqlite3_bind_text(stmt, 10, values [14].c_str (), -1, SQLITE_TRANSIENT);

    // and add station queries if needed
    if (headers.data_has_stations)
    {
        // start station:
        std::string stn_id = values [3], stn_name = values [4],
            lon = values [6], lat = values [5];
        boost::replace_all (stn_name, "\'", "");
        if (stationqry->count (stn_id) == 0 && lon != "0.0" && lat != "0.0" &&
                lon != "" && lat != "")
            (*stationqry)[stn_id] = "(\'" + city + "\',\'" + stn_id + "\',\'" +
                stn_name + "\'," + lon + "," + lat + ")";

        // end station:
        stn_id = values [7];
        stn_name = values [8];
        lon = values [10];
        lat = values [9];
        boost::replace_all (stn_name, "\'", "");
        if (stationqry->count (stn_id) == 0 && lon != "0.0" && lat != "0.0" &&
                lon != "" && lat != "")
            (*stationqry)[stn_id] = "(\'" + city + "\',\'" + stn_id + "\',\'" +
                stn_name + "\'," + lon + "," + lat + ")";
    }

    return 0;
}

std::string convert_usertype (std::string ut)
{
    // see comment in sqlite3db-add-data.cpp/get_field_positions - this is not
    // locale-safe!
    std::transform (ut.begin (), ut.end (), ut.begin (), ::tolower);
    boost::replace_all (ut, " ", "");
    if (ut.find ("member") != std::string::npos ||
            ut.find ("subscriber") != std::string::npos ||
            ut.find ("flex") != std::string::npos ||
            ut.find ("monthly") != std::string::npos ||
            ut.find ("indego30") != std::string::npos)
        ut = "1";
    else
        ut = "0";
    return ut;
}

std::string convert_gender (std::string g)
{
    if (g == "Female" || g == "2")
        g = "2";
    else if (g == "Male" || g == "1")
        g = "1";
    else
        g = "0";
    return g;
}

//' read_one_line_nyc
//'
//' NYC has really mixed up formats: sometimes no quotes at all; sometimes all
//' fields are separated by double-quotes; and then sometimes only some fields
//' are while others are not (201704 onwards). Quotes can not, however, simply
//' be stripped off all fields in the latter case, because station names have
//' commas in them. Cases with all double quotes (2016, for example) seem
//' **NOT** to have any station names with commas in them, so they are simply
//' de-quoted. The mixed case requires a separate routine to parse the differing
//' tokens.
//'
//' @param stmt An sqlit3 statement to be assembled by reading the line of data
//' @param line Line of data read from citibike file
//' @param stationqry Sqlite3 query for station data table to be subsequently
//'        passed to 'import_to_station_table()'
//' @param delim Delimeter for data files (changes from 2015-> from
//'        double-quoted fields to plain comma-separators.
//'
//' @noRd
unsigned int read_one_line_nyc (sqlite3_stmt * stmt, char * line,
        std::map <std::string, std::string> * stationqry, const char * delim)
{
    std::string in_line2 = line;
    unsigned int ret;
    if (strncmp (delim, ",", 1) == 0 ||
            (strncmp (delim, "\",\"", 3) == 0 && in_line2.substr (0, 1) == "\""))
        ret = read_one_line_nyc_standard (stmt, in_line2, stationqry, delim);
    else
        ret = read_one_line_nyc_mixed (stmt, in_line2, stationqry);

    return ret;
}

unsigned int read_one_line_nyc_standard (sqlite3_stmt * stmt,
        std::string &in_line2,
        std::map <std::string, std::string> * stationqry, const char * delim)
{
    char * duration;
    if (strncmp (delim, "\",\"", 3) == 0)
    {
        // Example of the following on L#19 of 2014-07
        boost::replace_all(in_line2, "\\N","\"\"");
        duration = strtokm (&in_line2[0u], "\""); //First double speech marks
        duration = strtokm (nullptr, delim); 
    } else
        duration = strtokm (&in_line2[0u], delim);


    std::string start_time = convert_datetime_ny (strtokm (nullptr, delim)); 
    std::string end_time = convert_datetime_ny (strtokm (nullptr, delim)); 
    std::string start_station_id = strtokm (nullptr, delim);
    start_station_id = "ny" + start_station_id;
    if (stationqry->count(start_station_id) == 0) {
        std::string start_station_name = strtokm (nullptr, delim);
        std::string start_station_lat = strtokm (nullptr, delim);
        std::string start_station_lon = strtokm (nullptr, delim);
        if (start_station_lat != "0.0" && start_station_lon != "0.0")
        {
            (*stationqry)[start_station_id] = "(\'ny\',\'" + 
                start_station_id + "\',\'" + start_station_name + "\'," +
                start_station_lat + "," + start_station_lon + ")";
        }
    }
    else {
        strtokm (nullptr, delim); // station name
        strtokm (nullptr, delim); // lat
        strtokm (nullptr, delim); // lon
    }

    std::string end_station_id = strtokm (nullptr, delim);
    end_station_id = "ny" + end_station_id;
    if (stationqry->count(end_station_id) == 0) {
        std::string end_station_name = strtokm (nullptr, delim);
        std::string end_station_lat = strtokm (nullptr, delim);
        std::string end_station_lon = strtokm (nullptr, delim);
        if (end_station_lat != "0.0" && end_station_lon != "0.0")
        {
            (*stationqry)[end_station_id] = "(\'ny\',\'" + 
                end_station_id + "\',\'" + end_station_name + "\'," +
                end_station_lat + "," + end_station_lon + ")";
        }
    }
    else {
        strtokm (nullptr, delim); // station name
        strtokm (nullptr, delim); // lat
        strtokm (nullptr, delim); // lon
    }

    std::string bike_id = strtokm (nullptr, delim);
    std::string user_type = strtokm (nullptr, delim);
    if (user_type == "Subscriber")
        user_type = "1";
    else
        user_type = "0";

    std::string birthyear = strtokm (nullptr, delim);
    std::string gender = strtokm (nullptr, delim);
    if (gender.length () == 2) // gender still has a terminal quote
        gender = gender [0];
    if (birthyear.empty()) {
        birthyear = "NULL";
    }
    if (gender.empty()) {
        gender = "NULL";
    }

    sqlite3_bind_text(stmt, 2, duration, -1, SQLITE_TRANSIENT); // Trip duration
    sqlite3_bind_text(stmt, 3, start_time.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 4, end_time.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 5, start_station_id.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 6, end_station_id.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 7, bike_id.c_str (), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 8, user_type.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 9, birthyear.c_str(), -1, SQLITE_TRANSIENT); // Birth Year
    sqlite3_bind_text(stmt, 10, gender.c_str(), -1, SQLITE_TRANSIENT); // Gender

    return 0;
}

unsigned int read_one_line_nyc_mixed (sqlite3_stmt * stmt,
        std::string &in_line2,
        std::map <std::string, std::string> * stationqry)
{
    const char * delim = ",";
    const char * delimq_st = ",\"";
    const char * delimq_end = "\",";
    const char * delimq_2 = "\",\"";

    boost::replace_all(in_line2, "\\N","\"\"");
    char * duration = strtokm (&in_line2[0u], delim);
    std::string start_time = strtokm (nullptr, delimq_2);
    start_time.erase (0, 1);
    start_time = convert_datetime_ny (start_time);
    std::string end_time = convert_datetime_ny (strtokm (nullptr, delimq_end)); 
    std::string start_station_id = strtokm (nullptr, delimq_st);
    start_station_id = "ny" + start_station_id;
    if (stationqry->count(start_station_id) == 0) {
        std::string start_station_name = strtokm (nullptr, delimq_end);
        std::string start_station_lat = strtokm (nullptr, delim);
        std::string start_station_lon = strtokm (nullptr, delim);
        if (start_station_lat != "0.0" && start_station_lon != "0.0")
        {
            (*stationqry)[start_station_id] = "(\'ny\',\'" + 
                start_station_id + "\',\'" + start_station_name + "\'," +
                start_station_lat + "," + start_station_lon + ")";
        }
    }
    else {
        strtokm (nullptr, delimq_end); // station name
        strtokm (nullptr, delim); // lat
        strtokm (nullptr, delim); // lon
    }

    std::string end_station_id = strtokm (nullptr, delimq_st);
    end_station_id = "ny" + end_station_id;
    if (stationqry->count(end_station_id) == 0) {
        std::string end_station_name = strtokm (nullptr, delimq_end);
        std::string end_station_lat = strtokm (nullptr, delim);
        std::string end_station_lon = strtokm (nullptr, delim);
        if (end_station_lat != "0.0" && end_station_lon != "0.0")
        {
            (*stationqry)[end_station_id] = "(\'ny\',\'" + 
                end_station_id + "\',\'" + end_station_name + "\'," +
                end_station_lat + "," + end_station_lon + ")";
        }
    }
    else {
        strtokm (nullptr, delimq_end); // station name
        strtokm (nullptr, delim); // lat
        strtokm (nullptr, delim); // lon
    }

    std::string bike_id = strtokm (nullptr, delimq_st);
    std::string user_type = strtokm (nullptr, delimq_end);
    if (user_type == "Subscriber")
        user_type = "1";
    else
        user_type = "0";

    std::string birthyear = strtokm (nullptr, delim);
    std::string gender = strtokm (nullptr, delim);
    if (gender.length () == 2) // gender still has a terminal quote
        gender = gender [0];
    if (birthyear.empty()) {
        birthyear = "NULL";
    }
    if (gender.empty()) {
        gender = "NULL";
    }

    sqlite3_bind_text(stmt, 2, duration, -1, SQLITE_TRANSIENT); // Trip duration
    sqlite3_bind_text(stmt, 3, start_time.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 4, end_time.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 5, start_station_id.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 6, end_station_id.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 7, bike_id.c_str (), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 8, user_type.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 9, birthyear.c_str(), -1, SQLITE_TRANSIENT); // Birth Year
    sqlite3_bind_text(stmt, 10, gender.c_str(), -1, SQLITE_TRANSIENT); // Gender

    return 0;
}

//' read_one_line_boston
//'
//' Like NYC, Boston has now also changed format (201801). All fields were
//' formerly embedded in quotes; now only non-numeric fields are quoted.
//' 
//' @param stmt An sqlit3 statement to be assembled by reading the line of data
//' @param line Line of data read from citibike file
//' @param stationqry Sqlite3 query for station data table to be subsequently
//'        passed to 'import_to_station_table()'
//'
//' @noRd
unsigned int read_one_line_boston (sqlite3_stmt * stmt, char * line,
        std::map <std::string, std::string> &stn_map, 
        std::unordered_set <std::string> &stn_ids)
{
    unsigned int ret;
    if (strpbrk (line, "\"") == nullptr) // no quotes at all
        ret = read_one_line_boston_pre15 (stmt, line, stn_map, stn_ids);
    else if (strncmp (line, "\"", 1) == 0) // first char is quote
        ret = read_one_line_boston_pre18 (stmt, line, stn_map, stn_ids);
    else // mixed case
        ret = read_one_line_boston_post18 (stmt, line, stn_map, stn_ids);

    return ret;
}

//' read_one_line_boston_pre15
//'
//' Parser for data files with no fields at all embedded in quotes
//'
//' @param stmt An sqlit3 statement to be assembled by reading the line of data
//' @param line Line of data read from citibike file
//' @param stationqry Sqlite3 query for station data table to be subsequently
//'        passed to 'import_to_station_table()'
//'
//' @noRd
unsigned int read_one_line_boston_pre15 (sqlite3_stmt * stmt, char * line,
        std::map <std::string, std::string> &stn_map, 
        std::unordered_set <std::string> &stn_ids)
{
    // TDOD: Replace strokm with strok here!
    const char * delim = ",";

    std::string in_line2 = line;
    boost::replace_all (in_line2, "\\N","");
    std::string duration = strtokm (&in_line2[0u], delim);
    // These are now stupidly in ms, so are rounded here to seconds
    duration = std::to_string (atoi (duration.c_str ()) / 1000.0);
    // start and end times are also in compact format with no leading zeros
    std::string start_time = strtokm (nullptr, delim);
    start_time = add_0_to_time (start_time);
    start_time = convert_datetime_nabsa (start_time);
    std::string end_time = strtokm (nullptr, delim); 
    end_time = add_0_to_time (end_time);
    end_time = convert_datetime_nabsa (end_time);

    // These station IDs in the annual dump files map directly on to those given
    // in the station files
    std::string start_station_id = strtokm (nullptr, delim);
    start_station_id = "bo" + start_station_id;
    std::string start_station_name = strtokm (nullptr, delim);
    boost::replace_all (start_station_name, "\'", ""); // rm apostrophes

    std::string end_station_id = strtokm (nullptr, delim);
    end_station_id = "bo" + end_station_id;
    std::string end_station_name = strtokm (nullptr, delim);
    boost::replace_all (end_station_name, "\'", ""); // rm apostrophes

    std::string bike_number = strtokm (nullptr, delim);
    std::string user_type = strtokm (nullptr, delim);
    std::string birth_year = "", gender = "";
    if (user_type == "Member")
    {
        birth_year = strtokm (nullptr, delim);
        gender = strtokm (nullptr, delim);
        boost::replace_all (gender, "\n","");
        boost::replace_all (birth_year, "\n","");
        user_type = "1";
    } else
        user_type = "0";

    // It's actually no longer birth year, rather zip code, so
    birth_year = "";

    sqlite3_bind_text(stmt, 2, duration.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 3, start_time.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 4, end_time.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 5, start_station_id.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 6, end_station_id.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 7, bike_number.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 8, user_type.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 9, birth_year.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 10, gender.c_str(), -1, SQLITE_TRANSIENT); 

    unsigned int ret = 0;
    if (stn_ids.find (start_station_id) == stn_ids.end () ||
            stn_ids.find (end_station_id) == stn_ids.end ())
        ret = 1;

    return ret;
}

//' read_one_line_boston_pre18
//'
//' Parser for data files with all fields embedded in quotes
//'
//' @param stmt An sqlit3 statement to be assembled by reading the line of data
//' @param line Line of data read from citibike file
//' @param stationqry Sqlite3 query for station data table to be subsequently
//'        passed to 'import_to_station_table()'
//'
//' @noRd
unsigned int read_one_line_boston_pre18 (sqlite3_stmt * stmt, char * line,
        std::map <std::string, std::string> &stn_map, 
        std::unordered_set <std::string> &stn_ids)
{
    // TDOD: Replace strokm with strok here!
    const char * delim = "\",\"";

    std::string in_line2 = line;
    boost::replace_all (in_line2, "\\N","\"\"");
    char * token = strtokm (&in_line2[0u], "\""); // opening quote
    (void) token; // supress unused variable warning;
    std::string duration = strtokm (nullptr, delim);
    std::string start_time = strtokm (nullptr, delim);
    start_time = add_0_to_time (start_time);
    start_time = convert_datetime_nabsa (start_time);
    std::string end_time = strtokm (nullptr, delim); 
    end_time = add_0_to_time (end_time);
    end_time = convert_datetime_nabsa (end_time);

    bool map_stns = false;
    std::string start_station_id = strtokm (nullptr, delim);
    if (start_station_id.length () <= 4)
    {
        map_stns = true;
        start_station_id = "bo" + start_station_id;
    }
    std::string start_station_name = strtokm (nullptr, delim);
    boost::replace_all (start_station_name, "\'", ""); // rm apostrophes
    std::string start_station_lat = strtokm (nullptr, delim);
    std::string start_station_lon = strtokm (nullptr, delim);

    std::string end_station_id = strtokm (nullptr, delim);
    if (end_station_id.length () > 4)
        end_station_id = "bo" + end_station_id;
    std::string end_station_name = strtokm (nullptr, delim);
    boost::replace_all (end_station_name, "\'", ""); // rm apostrophes
    std::string end_station_lat = strtokm (nullptr, delim);
    std::string end_station_lon = strtokm (nullptr, delim);

    if (start_station_name != "" && end_station_name != "")
    {
        start_station_id = convert_bo_stn_name (start_station_name, stn_map);
        end_station_id = convert_bo_stn_name (end_station_name, stn_map);
    }

    std::string bike_number = strtokm (nullptr, delim);
    std::string user_type = strtokm (nullptr, delim);
    std::string birth_year = "", gender = "";
    if (user_type == "Subscriber")
    {
        birth_year = strtokm (nullptr, delim);
        gender = strtokm (nullptr, delim);
        boost::replace_all (gender, "\n","");
        boost::replace_all (gender, "\"", "");
        boost::replace_all (birth_year, "\n","");
        boost::replace_all (birth_year, "\"", "");
        user_type = "1";
    } else
        user_type = "0";

    sqlite3_bind_text(stmt, 2, duration.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 3, start_time.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 4, end_time.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 5, start_station_id.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 6, end_station_id.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 7, bike_number.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 8, user_type.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 9, birth_year.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 10, gender.c_str(), -1, SQLITE_TRANSIENT); 

    unsigned int ret = 0;
    if (stn_ids.find (start_station_id) == stn_ids.end () ||
            stn_ids.find (end_station_id) == stn_ids.end ())
        ret = 1;

    return ret;
}

//' read_one_line_boston_post18
//'
//' Parser for data files in which only non-numeric fields are embedded in
//' quotes
//'
//' @param stmt An sqlit3 statement to be assembled by reading the line of data
//' @param line Line of data read from citibike file
//' @param stationqry Sqlite3 query for station data table to be subsequently
//'        passed to 'import_to_station_table()'
//'
//' @noRd
unsigned int read_one_line_boston_post18 (sqlite3_stmt * stmt, char * line,
        std::map <std::string, std::string> &stn_map, 
        std::unordered_set <std::string> &stn_ids)
{
    // TDOD: Replace strokm with strok here!
    const char * delim = ",";
    const char * delim_nq_q = ",\""; // no quote followed by quote
    const char * delim_q_nq = "\","; // quote followed by noquote
    const char * delim_q_q = "\",\""; // quote followed by quote

    std::string in_line2 = line;
    boost::replace_all (in_line2, "\\N","\"\"");

    std::string duration = strtokm (&in_line2[0u], delim_nq_q);
    std::string start_time = strtokm (nullptr, delim_q_q); // no need to convert
    std::string end_time = strtokm (nullptr, delim_q_nq); 

    bool map_stns = false;
    std::string start_station_id = strtokm (nullptr, delim_nq_q);
    if (start_station_id.length () <= 4)
    {
        map_stns = true;
        start_station_id = "bo" + start_station_id;
    }
    std::string start_station_name = strtokm (nullptr, delim_q_nq);
    boost::replace_all (start_station_name, "\'", ""); // rm apostrophes
    std::string start_station_lat = strtokm (nullptr, delim);
    std::string start_station_lon = strtokm (nullptr, delim);

    std::string end_station_id = strtokm (nullptr, delim_nq_q);
    if (end_station_id.length () > 4)
        end_station_id = "bo" + end_station_id;
    std::string end_station_name = strtokm (nullptr, delim_q_nq);
    boost::replace_all (end_station_name, "\'", ""); // rm apostrophes
    std::string end_station_lat = strtokm (nullptr, delim);
    std::string end_station_lon = strtokm (nullptr, delim);

    if (map_stns && !(start_station_name == "" || end_station_name == ""))
    {
        start_station_id = convert_bo_stn_name (start_station_name, stn_map);
        end_station_id = convert_bo_stn_name (end_station_name, stn_map);
    }

    std::string bike_number = strtokm (nullptr, delim_nq_q);
    std::string user_type = strtokm (nullptr, delim_q_nq);
    std::string birth_year = "", gender = "";
    if (user_type == "Subscriber")
    {
        birth_year = strtokm (nullptr, delim);
        gender = strtokm (nullptr, delim);
        boost::replace_all (gender, "\n","");
        boost::replace_all (gender, "\"", "");
        boost::replace_all (birth_year, "\n","");
        boost::replace_all (birth_year, "\"", "");
        user_type = "1";
    } else
        user_type = "0";

    sqlite3_bind_text(stmt, 2, duration.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 3, start_time.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 4, end_time.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 5, start_station_id.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 6, end_station_id.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 7, bike_number.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 8, user_type.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 9, birth_year.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 10, gender.c_str(), -1, SQLITE_TRANSIENT); 

    unsigned int ret = 0;
    if (stn_ids.find (start_station_id) == stn_ids.end () ||
            stn_ids.find (end_station_id) == stn_ids.end ())
        ret = 1;

    return ret;
}

//' read_one_line_chicago
//'
//' @param stmt An sqlit3 statement to be assembled by reading the line of data
//' @param line Line of data read from divvy bike file
//'
//' @noRd
unsigned int read_one_line_chicago (sqlite3_stmt * stmt, char * line,
        const char * delim)
{
    std::string in_line2 = line;
    char * token;
    if (strncmp (delim, "\",\"", 3) == 0)
    {
        //boost::replace_all(in_line2, "\\N","\"\"");
        token = strtokm (&in_line2[0u], "\""); //First double speech marks
        token = strtokm (nullptr, delim); 
    } else
        token = strtokm (&in_line2[0u], delim);
    (void) token; // supress unused variable warning;
    // First token is trip ID, which is not used here

    std::string start_time = convert_datetime_ch (strtokm (nullptr, delim)); 
    std::string end_time = convert_datetime_ch (strtokm (nullptr, delim)); 
    std::string bike_id = strtokm (nullptr, delim); 
    std::string duration = strtokm (nullptr, delim);

    std::string start_station_id = strtokm (nullptr, delim);
    start_station_id = "ch" + start_station_id;
    strtokm (nullptr, delim); // start station name
    std::string end_station_id = strtokm (nullptr, delim);
    end_station_id = "ch" + end_station_id;
    strtokm (nullptr, delim); // end station name

    std::string user_type = strtokm (nullptr, delim);
    std::string birth_year = "", gender = "";
    if (user_type == "Subscriber")
    {
        gender = strtokm (nullptr, delim);
        if (gender == "Female")
            gender = "2";
        else if (gender == "Male")
            gender = "1";
        else
            gender = "0";
        birth_year = strtokm (nullptr, delim);
        boost::replace_all (birth_year, "\n","");
        boost::replace_all (birth_year, "\"", ""); // Remove terminal quote
        user_type = "1";
    } else
        user_type = "0";

    sqlite3_bind_text(stmt, 2, duration.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 3, start_time.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 4, end_time.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 5, start_station_id.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 6, end_station_id.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 7, bike_id.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 8, user_type.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 9, birth_year.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 10, gender.c_str(), -1, SQLITE_TRANSIENT); 

    return 0;
}

//' read_one_line_dc
//'
//' @param stmt An sqlit3 statement to be assembled by reading the line of data
//' @param line Line of data read from capital bikeshare file
//' @param dur_ms True if duration is single number (in ms); otherwise duration
//' has to be explicitly parsed
//' @param id True if file contains station ID columns
//'
//' @note Trip durations were initially reported as "0h 5min. 2sec.", then
//' changed for 2012Q1 only to "0h 5m 2sec.", then again from 2012Q2 to
//' "0h 5m 2s" and remained in that form until end 2014. They nevertheless are
//' always separated by spaces. From 2015Q1, these were replaced with single
//' durations in milliseconds.
//' Start and end stations were initially station addresses with ID numbers
//' parenthesised within the same records, but then from 2012Q1 the ID numbers
//' disappeared and only addresses were given. Only from 2015Q3 onwards do
//' trip records contain separate station IDs
//'
//' @noRd
unsigned int read_one_line_dc (sqlite3_stmt * stmt, char * line, 
        std::map <std::string, std::string> &stn_map, 
        std::unordered_set <std::string> &stn_ids)
{
    std::string in_line2 = line;

    char * token = strtokm (&in_line2[0u], ",");
    std::string duration = token;
    size_t ipos = duration.find ("h", 0);
    if (ipos != std::string::npos)
    {
        int hh = atoi (duration.substr (0, ipos).c_str ());
        ipos = duration.find (" ", 0);
        duration = duration.substr (ipos + 1, duration.length () - ipos - 1);
        ipos = duration.find ("m", 0);
        int mm = atoi (duration.substr (0, ipos).c_str ());
        ipos = duration.find (" ", 0);
        duration = duration.substr (ipos + 1, duration.length () - ipos - 1);
        ipos = duration.find ("s", 0);
        int ss = atoi (duration.substr (0, ipos).c_str ());
        hh = hh * 3600 + mm * 60 + ss;
        hh = hh * 1000; // milliseconds
        duration = std::to_string (hh);
    }

    std::string start_date = convert_datetime_dc (strtokm (nullptr, ",")); 
    std::string end_date = convert_datetime_dc (strtokm (nullptr, ",")); 
    std::string start_station_id = strtokm (nullptr, ",");
    start_station_id = "dc" + start_station_id;
    std::string start_station_name = strtokm (nullptr, ",");
    std::string end_station_id = strtokm (nullptr, ",");
    end_station_id = "dc" + end_station_id;
    std::string end_station_name = strtokm (nullptr, ",");

    /*
    if (start_station_name != "" && end_station_name != "")
    {
        start_station_id = convert_dc_stn_name (start_station_name, id,
                stn_map);
        end_station_id = convert_dc_stn_name (end_station_name, id,
                stn_map);
    }
    */

        // Only personal data for DC is user_type
    std::string bike_id = strtokm (nullptr, ",");
    std::string user_type = strtokm (nullptr, ",");
    if (user_type == "Casual")
        user_type = "0";
    else // sometimes "Member", sometimes "Registered"
        user_type = "1";

    // Only store those trip with stations in the official list:
    if (stn_ids.find (start_station_id) != stn_ids.end () &&
            stn_ids.find (end_station_id) != stn_ids.end ())
    {
        sqlite3_bind_text(stmt, 2, duration.c_str(), -1, SQLITE_TRANSIENT); 
        sqlite3_bind_text(stmt, 3, start_date.c_str(), -1, SQLITE_TRANSIENT); 
        sqlite3_bind_text(stmt, 4, end_date.c_str(), -1, SQLITE_TRANSIENT); 
        sqlite3_bind_text(stmt, 5, start_station_id.c_str(), -1, SQLITE_TRANSIENT); 
        sqlite3_bind_text(stmt, 6, end_station_id.c_str(), -1, SQLITE_TRANSIENT); 
        sqlite3_bind_text(stmt, 7, bike_id.c_str(), -1, SQLITE_TRANSIENT); 
        sqlite3_bind_text(stmt, 8, user_type.c_str(), -1, SQLITE_TRANSIENT); 
    }

    return 0;
}

//' Convert names of Boston stations as given in trip files to standard names
//'
//' @param station_name String as read from trip file
//' @param stn_map Map of station names to IDs
//'
//' @noRd
std::string convert_bo_stn_name (std::string &station_name,
        std::map <std::string, std::string> &stn_map)
{
    std::string station, station_id = "";
    boost::replace_all (station_name, "\'", ""); // rm apostrophes

    size_t ipos = station_name.find ("(", 0);
    if (ipos != std::string::npos)
    {
        station_id = "bo" + station_name.substr (ipos + 1,
                station_name.length () - ipos - 2);
        station_name = station_name.substr (0, ipos - 1);
    } 
    std::map <std::string, std::string>::const_iterator mpos;
    mpos = stn_map.find (station_name);
    if (mpos != stn_map.end ())
        station_id = mpos->second;

    return station_id;
}

//' Convert names of DC stations as given in trip files to standard names
//'
//' @param station_name String as read from trip file
//' @param id True if trip file contains separate station ID field
//' @param stn_map Map of station names to IDs
//'
//' @note Start and end stations were initially station addresses with ID
//' numbers parenthesised within the same records, but then from 2012Q1 the ID
//' numbers disappeared and only addresses were given. Some station names in
//' trip files also contain "[formerly ...]" guff, where the former names never
//' appear in the official station file, and so this must be removed.
//'
//' @noRd
std::string convert_dc_stn_name (std::string &station_name, bool id,
        std::map <std::string, std::string> &stn_map)
{
    std::string station, station_id = "";
    boost::replace_all (station_name, "\'", ""); // rm apostrophes

    size_t ipos = station_name.find ("(", 0);
    bool id_in_namestr = false;
    if (ipos != std::string::npos)
    {
        station_id = "dc" + station_name.substr (ipos + 1,
                station_name.length () - ipos - 2);
        station_name = station_name.substr (0, ipos - 1);
        id_in_namestr = true;
    } 
    // Strip off instances of " [formerly ...]"
    ipos = station_name.find (" [", 0);
    if (ipos != std::string::npos)
        station_name = station_name.substr (0, ipos);
    // Some of these also have additional trailing white space:
    while (station_name.substr (station_name.length () - 1,
                station_name.length () - 0) == " ")
        station_name = station_name.substr (0,
                station_name.length () - 1);
    if (!id && !id_in_namestr)
    {
        std::map <std::string, std::string>::const_iterator mpos;
        mpos = stn_map.find (station_name);
        if (mpos != stn_map.end ())
            station_id = mpos->second;
    }

    return station_id;
}

//' read_one_line_london
//'
//' @param stmt An sqlit3 statement to be assembled by reading the line of data
//' @param line Line of data read from Santander cycles file
//'
//' @noRd
unsigned int read_one_line_london (sqlite3_stmt * stmt, char * line)
{
    std::string in_line = line;

    // London is done with str_token which uses std::strings because
    // end_station_names are sometimes but not always surrounded by double
    // quotes.  They also sometimes have commas, but if so they  always have
    // double quotes. It is therefore necessary to get relative positions of
    // commas and double quotes, and this is much easier to do with strings than
    // with char arrays. Only disadvantage: Somewhat slower.
    std::string duration = str_token (&in_line, ","); // Rental ID: not used
    duration = str_token (&in_line, ",");
    std::string bike_id = str_token (&in_line, ",");
    std::string end_date = convert_datetime_generic (str_token (&in_line, ","));
    std::string end_station_id = str_token (&in_line, ",");
    end_station_id = "lo" + end_station_id;
    std::string end_station_name;
    if (strcspn (in_line.c_str (), "\"") == 0) // name in quotes
    {
        end_station_name = str_token (&in_line, "\",");
        end_station_name = end_station_name.substr (1, 
                end_station_name.length ()); // rm quote from start
        in_line = in_line.substr (1, in_line.length ()); // rm comma from start
    } else
        end_station_name = str_token (&in_line, ",");
    std::string start_date = convert_datetime_generic (str_token (&in_line, ","));
    std::string start_station_id = str_token (&in_line, ",");
    start_station_id = "lo" + start_station_id;

    sqlite3_bind_text(stmt, 2, duration.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 3, start_date.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 4, end_date.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 5, start_station_id.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 6, end_station_id.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 7, bike_id.c_str(), -1, SQLITE_TRANSIENT); 

    unsigned int res = 0;
    if (start_date == "" || end_date == "" ||
            start_date == "NA" || end_date == "NA")
        res = 1;

    return res;
}


//' add_0_to_time
//'
//' The hours part of LA and Philly times are single digit for HH < 10. SQLite
//' requires ' strict HH, so this function inserts an extra zero where needed.
//'
//' @param time A datetime column from the LA or Philly data
//'
//' @return datetime with two-digit hour field
//'
//' @noRd
std::string add_0_to_time (std::string time)
{
    size_t gap_pos = time.find (" ");
    size_t time_div_pos = time.find (":");
    if ((time_div_pos - gap_pos) == 2)
        time = time.substr (0, gap_pos + 1) + "0" +
            time.substr (time_div_pos - 1, time.length () - 1);

    return time;
}


//' read_one_line_nabsa
//'
//' North American Bike Share Association open data standard (LA and
//' Philadelpia) have identical file formats
//'
//' @param stmt An sqlit3 statement to be assembled by reading the line of data
//' @param line Line of data read from LA metro or Philadelphia Indego file
//' @param stationqry Sqlite3 query for station data table to be subsequently
//'        passed to 'import_to_station_table()'
//'
//' @noRd
unsigned int read_one_line_nabsa (sqlite3_stmt * stmt, char * line,
        std::map <std::string, std::string> * stationqry, std::string city)
{
    std::string in_line = line;
    boost::replace_all (in_line, "\\N"," "); 
    // replace_all only works with the following two lines, NOT with a single
    // attempt to replace all ",,"!
    boost::replace_all (in_line, ",,,",", , ,");
    boost::replace_all (in_line, ",,",", ,");

    const char * delim;
    delim = ",";
    char * trip_id = std::strtok (&in_line[0u], delim); 
    (void) trip_id; // supress unused variable warning;
    unsigned int ret = 0;

    std::string trip_duration = std::strtok (nullptr, delim);
    std::string start_date = std::strtok (nullptr, delim);
    start_date = add_0_to_time (start_date);
    start_date = convert_datetime_nabsa (start_date);
    std::string end_date = std::strtok (nullptr, delim);
    end_date = add_0_to_time (end_date);
    end_date = convert_datetime_nabsa (end_date);
    std::string start_station_id = std::strtok (nullptr, delim);
    if (start_station_id == " " || start_station_id == "#N/A")
        ret = 1;
    start_station_id = city + start_station_id;
    std::string start_station_lat = std::strtok (nullptr, delim);
    std::string start_station_lon = std::strtok (nullptr, delim);
    // lat and lons are sometimes empty, which is useless 
    if (stationqry->count(start_station_id) == 0 && ret == 0 &&
            start_station_lat != " " && start_station_lon != " " &&
            start_station_lat != "0" && start_station_lon != "0")
    {
        std::string start_station_name = "";
        (*stationqry)[start_station_id] = "(\'" + city + "\',\'" +
            start_station_id + "\',\'\'," + 
            start_station_lat + delim + start_station_lon + ")";
    }

    std::string end_station_id = std::strtok (nullptr, delim);
    if (end_station_id == " " || end_station_id == "#N/A")
        ret = 1;
    end_station_id = city + end_station_id;
    std::string end_station_lat = std::strtok (nullptr, delim);
    std::string end_station_lon = std::strtok (nullptr, delim);
    if (stationqry->count(end_station_id) == 0 && ret == 0 &&
            end_station_lat != " " && end_station_lon != " " &&
            end_station_lat != "0" && end_station_lon != "0")
    {
        std::string end_station_name = "";
        (*stationqry)[end_station_id] = "(\'" + city + "\',\'" +
            end_station_id + "\',\'\'," + 
            end_station_lat + "," + end_station_lon + ")";
    }
    // NABSA systems only have duration of membership as (30 = monthly, etc)
    std::string user_type = std::strtok (nullptr, delim); // bike_id
    user_type = std::strtok (nullptr, delim); // plan_duration
    user_type = std::strtok (nullptr, delim); // trip_route_category
    user_type = std::strtok (nullptr, delim); // finally, "passholder_type"
    if (user_type == "" || user_type == "Walk-up")
        user_type = "0"; // casual
    else
        user_type = "1"; // subscriber

    sqlite3_bind_text(stmt, 2, trip_duration.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 3, start_date.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 4, end_date.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 5, start_station_id.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 6, end_station_id.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 7, "", -1, SQLITE_TRANSIENT); // bike ID
    sqlite3_bind_text(stmt, 8, user_type.c_str(), -1, SQLITE_TRANSIENT); 

    // The boost::replace_all above ensures void values are all single spaces
    if (start_station_id == " " || end_station_id == " " ||
            start_station_lat == " " || start_station_lon == " " ||
            end_station_lat == " " || end_station_lon == " ")
        ret = 1; // trip data not stored!

    return ret;
}

unsigned int read_one_line_mn (sqlite3_stmt * stmt, char * line)
{
    const char * delim = ",";

    std::string in_line2 = line;
    boost::replace_all (in_line2, "\\N","");

    std::string start_date = strtokm (&in_line2[0u], delim);
    start_date = add_0_to_time (start_date);
    start_date = convert_datetime_nabsa (start_date);

    std::string start_station_name = strtokm (nullptr, delim);
    boost::replace_all (start_station_name, "\'", ""); // rm apostrophes
    std::string start_station_id = strtokm (nullptr, delim);

    std::string end_date = strtokm (nullptr, delim);
    end_date = add_0_to_time (end_date);
    end_date = convert_datetime_nabsa (end_date);
    std::string end_station_name = strtokm (nullptr, delim);
    boost::replace_all (start_station_name, "\'", ""); // rm apostrophes
    std::string end_station_id = strtokm (nullptr, delim);

    // The data do have trip durations, but these are in variable columns, so
    // can be reconstructed from the start-end time data if really desired.
    std::string duration = "", bike_number = "", user_type = "",
        birth_year = "", gender = "";

    unsigned int ret = 0;
    if (start_date == "" || end_station_id == "")
    {
        ret = 1;
    } else
    {
        int dur = timediff (start_date, end_date);
        duration = std::to_string (dur);
    }

    sqlite3_bind_text(stmt, 2, duration.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 3, start_date.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 4, end_date.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 5, start_station_id.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 6, end_station_id.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 7, bike_number.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 8, user_type.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 9, birth_year.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 10, gender.c_str(), -1, SQLITE_TRANSIENT); 

    return ret;
}


//' read_one_line_sf
//'
//' This function tokenizes strings from one line of san francisco bay area's bikeshare data
//' 
//' @param stmt An sqlit3 statement to be assembled by reading the line of data
//' @param line Line of data read from citibike file
//' @param stationqry Sqlite3 query for station data table to be subsequently
//'        passed to 'import_to_station_table()'
//'
//' @noRd
//' read_one_line_sf
//'
//' Parser for data files in which only non-numeric fields are embedded in
//' quotes, like Boston post 2018
//'
//' @param stmt An sqlit3 statement to be assembled by reading the line of data
//' @param line Line of data read from citibike file
//' @param stationqry Sqlite3 query for station data table to be subsequently
//'        passed to 'import_to_station_table()'
//'
//' @noRd
unsigned int read_one_line_sf (sqlite3_stmt * stmt, char * line,
                               std::map <std::string, std::string> * stationqry, 
                                std::string city)
{ 
    const char * delim = ",";
    const char * delim_nq_q = ",\""; // no quote followed by quote
    const char * delim_q_nq = "\","; // quote followed by noquote
    const char * delim_q_q = "\",\""; // quote followed by quote

    std::string in_line2 = line;
    boost::replace_all (in_line2, "\\N","\"\"");

// from read_one_boston

    std::string duration = strtokm (&in_line2[0u], delim_nq_q);
    std::string start_time = strtokm (nullptr, delim_q_q); // no need to convert
    std::string end_time = strtokm (nullptr, delim_q_nq); 

    std::string start_station_id = strtokm (nullptr, delim_nq_q);
    std::string start_station_name = strtokm (nullptr, delim_q_nq);
    boost::replace_all (start_station_name, "\'", ""); // rm apostrophes
    std::string start_station_lat = strtokm (nullptr, delim);
    std::string start_station_lon = strtokm (nullptr, delim);

    std::string end_station_id = strtokm (nullptr, delim_nq_q);
    std::string end_station_name = strtokm (nullptr, delim_q_nq);
    boost::replace_all (end_station_name, "\'", ""); // rm apostrophes
    std::string end_station_lat = strtokm (nullptr, delim);
    std::string end_station_lon = strtokm (nullptr, delim);

// end from read_one_boston

// select from read_one_nabsa

    unsigned int ret = 0;

    start_station_id = city + start_station_id;
    end_station_id = city + end_station_id;

    if (stationqry->count(end_station_id) == 0 && ret == 0 &&
            end_station_lat != " " && end_station_lon != " " &&
            end_station_lat != "0" && end_station_lon != "0")
    {
        std::string end_station_name = "";
        (*stationqry)[end_station_id] = "(\'" + city + "\',\'" +
            end_station_id + "\',\'\'," + 
            end_station_lat + "," + end_station_lon + ")";
    }
// end from read_one_nabsa

    std::string bike_number = strtokm (nullptr, delim_nq_q);
    std::string user_type = strtokm (nullptr, delim_q_nq);
    std::string birth_year = "", gender = "";
    if (user_type == "Subscriber")
    {
        birth_year = strtokm (nullptr, delim);
        boost::replace_all (birth_year, "\n","");
        boost::replace_all (birth_year, "\"", "");
        gender = strtokm (nullptr, delim_nq_q);
        boost::replace_all (gender, "\n","");
        boost::replace_all (gender, "\"", "");
        //from read_one_chicago
        if (gender == "Female")
            gender = "2";
        else if (gender == "Male")
            gender = "1";
        else
            gender = "0";
        //end from read_one_chicago
        user_type = "1";
    } else
        user_type = "0";

    sqlite3_bind_text(stmt, 2, duration.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 3, start_time.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 4, end_time.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 5, start_station_id.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 6, end_station_id.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 7, bike_number.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 8, user_type.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 9, birth_year.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 10, gender.c_str(), -1, SQLITE_TRANSIENT); 

    return ret;
}
