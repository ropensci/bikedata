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
#pragma once

#include <unordered_set>
#include "vendor/sqlite3/sqlite3.h"

// NOTE: Return values are only used for LA

unsigned int read_one_line_nyc (sqlite3_stmt * stmt, char * line,
        std::map <std::string, std::string> * stationqry, const char * delim);
unsigned int read_one_line_nyc_standard (sqlite3_stmt * stmt,
        std::string &in_line2,
        std::map <std::string, std::string> * stationqry, const char * delim);
unsigned int read_one_line_nyc_mixed (sqlite3_stmt * stmt,
        std::string &in_line2,
        std::map <std::string, std::string> * stationqry);

unsigned int read_one_line_boston (sqlite3_stmt * stmt, char * line,
        std::map <std::string, std::string> * stationqry);
unsigned int read_one_line_chicago (sqlite3_stmt * stmt, char * line,
        const char * delim);
unsigned int read_one_line_dc (sqlite3_stmt * stmt, char * line, 
        std::map <std::string, std::string> &stn_map, 
        std::unordered_set <std::string> &stn_ids,
        bool id, bool end_date_first);
std::string convert_dc_stn_name (std::string &station_name, bool id,
        std::map <std::string, std::string> &stn_map);
unsigned int read_one_line_london (sqlite3_stmt * stmt, char * line);
std::string add_0_to_time (std::string time);
unsigned int read_one_line_nabsa (sqlite3_stmt * stmt, char * line,
        std::map <std::string, std::string> * stationqry,
        std::string city);


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
//' @param stmt An sqlit3 statement to be assembled by reading the line of data
//' @param line Line of data read from citibike file
//' @param stationqry Sqlite3 query for station data table to be subsequently
//'        passed to 'import_to_station_table()'
//'
//' @noRd
unsigned int read_one_line_boston (sqlite3_stmt * stmt, char * line,
        std::map <std::string, std::string> * stationqry)
{
    // TDOD: Replace strokm with strok here!
    const char * delim = "\",\"";

    std::string in_line2 = line;
    boost::replace_all (in_line2, "\\N","\"\"");
    char * token = strtokm (&in_line2[0u], "\""); // opening quote
    (void) token; // supress unused variable warning;
    std::string duration = strtokm (nullptr, delim);
    std::string start_time = strtokm (nullptr, delim); // no need to convert
    std::string end_time = strtokm (nullptr, delim); 

    std::string start_station_id = strtokm (nullptr, delim);
    start_station_id = "bo" + start_station_id;
    if (stationqry->count (start_station_id) == 0) {
        std::string start_station_name = strtokm (nullptr, delim);
        boost::replace_all (start_station_name, "\'", ""); // rm apostrophes
        std::string start_station_lat = strtokm (nullptr, delim);
        std::string start_station_lon = strtokm (nullptr, delim);
        if (start_station_lat != "" && start_station_lon != "")
            (*stationqry)[start_station_id] = "(\'bo\',\'" + 
                start_station_id + "\',\'" + start_station_name + "\'," +
                start_station_lat + "," + start_station_lon + ")";
    } else
    {
        strtokm (nullptr, delim);
        strtokm (nullptr, delim);
        strtokm (nullptr, delim);
    }


    std::string end_station_id = strtokm (nullptr, delim);
    end_station_id = "bo" + end_station_id;
    if (stationqry->count (end_station_id) == 0) {
        std::string end_station_name = strtokm (nullptr, delim);
        boost::replace_all (end_station_name, "\'", ""); // rm apostrophes
        std::string end_station_lat = strtokm (nullptr, delim);
        std::string end_station_lon = strtokm (nullptr, delim);
        if (end_station_lat != "" && end_station_lon != "")
            (*stationqry)[end_station_id] = "(\'bo\',\'" + 
                end_station_id + "\',\'" + end_station_name + "\'," +
                end_station_lat + "," + end_station_lon + ")";
    } else
    {
        strtokm (nullptr, delim);
        strtokm (nullptr, delim);
        strtokm (nullptr, delim);
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

    return 0;
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
        std::unordered_set <std::string> &stn_ids, bool id, bool end_date_first)
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

    std::string start_station_name, start_station_id = "0",
        end_station_name, end_station_id = "0", end_date;
    if (id)
    {
        end_date = convert_datetime_dc (strtokm (nullptr, ",")); 
        start_station_id = strtokm (nullptr, ",");
        start_station_id = "dc" + start_station_id;
        start_station_name = strtokm (nullptr, ",");
        end_station_id = strtokm (nullptr, ",");
        end_station_id = "dc" + end_station_id;
        end_station_name = strtokm (nullptr, ",");
    } else
    {
        if (end_date_first)
        {
            end_date = convert_datetime_dc (strtokm (nullptr, ",")); 
            start_station_name = strtokm (nullptr, ",");
        } else
        {
            start_station_name = strtokm (nullptr, ",");
            end_date = convert_datetime_dc (strtokm (nullptr, ",")); 
        }
        end_station_name = strtokm (nullptr, ",");

        if (start_station_name != "" && end_station_name != "")
        {
            start_station_id = convert_dc_stn_name (start_station_name, id,
                    stn_map);
            end_station_id = convert_dc_stn_name (end_station_name, id,
                    stn_map);
        }
    }

    std::string bike_id, user_type;
    if (id)
    {
    } else
    {
        // Only personal data for DC is user_type
        bike_id = strtokm (nullptr, ",");
        user_type = strtokm (nullptr, ",");
        if (user_type == "Casual")
            user_type = "0";
        else // sometimes "Member", sometimes "Registered"
            user_type = "1";
    }

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

//' Convert names of DC stations as given in trip files to standard names
//'
//' @param station_name String as read from trip file
//' @param id True if trip file contains separate station ID field
//' @param stn_map Map of station names to IDs
//'
//' @note ' Start and end stations were initially station addresses with ID
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
    std::string end_date = convert_datetime_lo (str_token (&in_line, ","));
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
    std::string start_date = convert_datetime_lo (str_token (&in_line, ","));
    std::string start_station_id = str_token (&in_line, ",");
    start_station_id = "lo" + start_station_id;

    sqlite3_bind_text(stmt, 2, duration.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 3, start_date.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 4, end_date.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 5, start_station_id.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 6, end_station_id.c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 7, bike_id.c_str(), -1, SQLITE_TRANSIENT); 

    unsigned int res = 0;
    if (start_date == "" || end_date == "")
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
