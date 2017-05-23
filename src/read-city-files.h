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
#include "sqlite3/sqlite3.h"

// NOTE: Return values are only used for LA

unsigned read_one_line_nyc (sqlite3_stmt * stmt, char * line,
        std::map <std::string, std::string> * stationqry, const char * delim);
unsigned read_one_line_boston (sqlite3_stmt * stmt, char * line,
        std::map <std::string, std::string> * stationqry);
unsigned read_one_line_chicago (sqlite3_stmt * stmt, char * line,
        const char * delim);
unsigned read_one_line_dc (sqlite3_stmt * stmt, char * line, 
        std::map <std::string, std::string> &stn_map, 
        std::unordered_set <std::string> &stn_ids,
        bool id, bool end_date_first);
std::string convert_dc_stn_name (std::string &station_name, bool id,
        std::map <std::string, std::string> &stn_map);
unsigned read_one_line_london (sqlite3_stmt * stmt, char * line);
std::string add_0_to_la_time (std::string time);
unsigned read_one_line_la (sqlite3_stmt * stmt, char * line,
        std::map <std::string, std::string> * stationqry);


//' read_one_line_nyc
//'
//' @param stmt An sqlit3 statement to be assembled by reading the line of data
//' @param line Line of data read from citibike file
//' @param stationqry Sqlite3 query for station data table to be subsequently
//'        passed to 'import_to_station_table()'
//' @param delim Delimeter for data files (changes from 2015-> from
//'        double-quoted fields to plain comma-separators.
//'
//' @noRd
unsigned read_one_line_nyc (sqlite3_stmt * stmt, char * line,
        std::map <std::string, std::string> * stationqry, const char * delim)
{
    std::string in_line2 = line;
    char * duration;
    if (strncmp (delim, "\",\"", 3) == 0)
    {
        // Example of the following on L#19 of 2014-07
        boost::replace_all(in_line2, "\\N","\"\"");
        duration = strtokm (&in_line2[0u], "\""); //First double speech marks
        duration = strtokm (NULL, delim); 
    } else
        duration = strtokm (&in_line2[0u], delim);


    std::string start_time = convert_datetime_ny (strtokm (NULL, delim)); 
    std::string end_time = convert_datetime_ny (strtokm (NULL, delim)); 
    std::string start_station_id = strtokm (NULL, delim);
    start_station_id = "ny" + start_station_id;
    if (stationqry->count(start_station_id) == 0) {
        std::string start_station_name = strtokm (NULL, delim);
        std::string start_station_lat = strtokm (NULL, delim);
        std::string start_station_lon = strtokm (NULL, delim);
        (*stationqry)[start_station_id] = "(\'ny\',\'" + 
            start_station_id + "\',\'" + start_station_name + "\'," +
            start_station_lat + "," + start_station_lon + ")";
    }
    else {
        strtokm (NULL, delim); // station name
        strtokm (NULL, delim); // lat
        strtokm (NULL, delim); // lon
    }

    std::string end_station_id = strtokm (NULL, delim);
    end_station_id = "ny" + end_station_id;
    if (stationqry->count(end_station_id) == 0) {
        std::string end_station_name = strtokm (NULL, delim);
        std::string end_station_lat = strtokm (NULL, delim);
        std::string end_station_lon = strtokm (NULL, delim);
        (*stationqry)[end_station_id] = "(\'ny\',\'" + 
            end_station_id + "\',\'" + end_station_name + "\'," +
            end_station_lat + "," + end_station_lon + ")";
    }
    else {
        strtokm (NULL, delim); // station name
        strtokm (NULL, delim); // lat
        strtokm (NULL, delim); // lon
    }

    std::string bike_id = strtokm (NULL, delim);
    std::string user_type = strtokm (NULL, delim);
    if (user_type == "Subscriber")
        user_type = "1";
    else
        user_type = "0";

    std::string birthyear = strtokm (NULL, delim);
    std::string gender = strtokm (NULL, delim);
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
unsigned read_one_line_boston (sqlite3_stmt * stmt, char * line,
        std::map <std::string, std::string> * stationqry)
{
    // TDOD: Replace strokm with strok here!
    const char * delim = "\",\"";

    std::string in_line2 = line;
    boost::replace_all (in_line2, "\\N","\"\"");
    char * token = strtokm (&in_line2[0u], "\""); // opening quote
    (void) token; // supress unused variable warning;
    std::string duration = strtokm (NULL, delim);
    std::string start_time = strtokm (NULL, delim); // no need to convert
    std::string end_time = strtokm (NULL, delim); 

    std::string start_station_id = strtokm (NULL, delim);
    start_station_id = "bo" + start_station_id;
    if (stationqry->count (start_station_id) == 0) {
        std::string start_station_name = strtokm (NULL, delim);
        boost::replace_all (start_station_name, "\'", ""); // rm apostrophes
        std::string start_station_lat = strtokm (NULL, delim);
        std::string start_station_lon = strtokm (NULL, delim);
        if (start_station_lat != "" && start_station_lon != "")
            (*stationqry)[start_station_id] = "(\'bo\',\'" + 
                start_station_id + "\',\'" + start_station_name + "\'," +
                start_station_lat + "," + start_station_lon + ")";
    } else
    {
        strtokm (NULL, delim);
        strtokm (NULL, delim);
        strtokm (NULL, delim);
    }


    std::string end_station_id = strtokm (NULL, delim);
    end_station_id = "bo" + end_station_id;
    if (stationqry->count (end_station_id) == 0) {
        std::string end_station_name = strtokm (NULL, delim);
        boost::replace_all (end_station_name, "\'", ""); // rm apostrophes
        std::string end_station_lat = strtokm (NULL, delim);
        std::string end_station_lon = strtokm (NULL, delim);
        if (end_station_lat != "" && end_station_lon != "")
            (*stationqry)[end_station_id] = "(\'bo\',\'" + 
                end_station_id + "\',\'" + end_station_name + "\'," +
                end_station_lat + "," + end_station_lon + ")";
    } else
    {
        strtokm (NULL, delim);
        strtokm (NULL, delim);
        strtokm (NULL, delim);
    }

    std::string bike_number = strtokm (NULL, delim);
    std::string user_type = strtokm (NULL, delim);
    std::string birth_year = "", gender = "";
    if (user_type == "Subscriber")
    {
        birth_year = strtokm (NULL, delim);
        gender = strtokm (NULL, delim);
        boost::replace_all (gender, "\"", ""); // Remove terminal quote
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
//' @param line Line of data read from citibike file
//'
//' @noRd
unsigned read_one_line_chicago (sqlite3_stmt * stmt, char * line,
        const char * delim)
{
    std::string in_line2 = line;
    char * token;
    if (strncmp (delim, "\",\"", 3) == 0)
    {
        //boost::replace_all(in_line2, "\\N","\"\"");
        token = strtokm (&in_line2[0u], "\""); //First double speech marks
        token = strtokm (NULL, delim); 
    } else
        token = strtokm (&in_line2[0u], delim);
    (void) token; // supress unused variable warning;
    // First token is trip ID, which is not used here

    std::string start_time = convert_datetime_ch (strtokm (NULL, delim)); 
    std::string end_time = convert_datetime_ch (strtokm (NULL, delim)); 
    std::string bike_id = strtokm (NULL, delim); 
    std::string duration = strtokm (NULL, delim);

    std::string start_station_id = strtokm (NULL, delim);
    start_station_id = "ch" + start_station_id;
    strtokm (NULL, delim); // start station name
    std::string end_station_id = strtokm (NULL, delim);
    end_station_id = "ch" + end_station_id;
    strtokm (NULL, delim); // end station name

    std::string user_type = strtokm (NULL, delim);
    std::string birth_year = "", gender = "";
    if (user_type == "Subscriber")
    {
        gender = strtokm (NULL, delim);
        if (gender == "Female")
            gender = "2";
        else if (gender == "Male")
            gender = "1";
        else
            gender = "0";
        birth_year = strtokm (NULL, delim);
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
//' @param line Line of data read from citibike file
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
unsigned read_one_line_dc (sqlite3_stmt * stmt, char * line, 
        std::map <std::string, std::string> &stn_map, 
        std::unordered_set <std::string> &stn_ids, bool id, bool end_date_first)
{
    std::string in_line2 = line;

    char * token = strtokm (&in_line2[0u], ",");
    std::string duration = token;
    size_t ipos = duration.find ("h", 0);
    if (ipos != std::string::npos)
    {
        unsigned hh = atoi (duration.substr (0, ipos).c_str ());
        ipos = duration.find (" ", 0);
        duration = duration.substr (ipos + 1, duration.length () - ipos - 1);
        ipos = duration.find ("m", 0);
        unsigned mm = atoi (duration.substr (0, ipos).c_str ());
        ipos = duration.find (" ", 0);
        duration = duration.substr (ipos + 1, duration.length () - ipos - 1);
        ipos = duration.find ("s", 0);
        unsigned ss = atoi (duration.substr (0, ipos).c_str ());
        hh = hh * 3600 + mm * 60 + ss;
        hh = hh * 1000; // milliseconds
        duration = std::to_string (hh);
    }

    std::string start_date = convert_datetime_dc (strtokm (NULL, ",")); 

    std::string start_station_name, start_station_id = "0",
        end_station_name, end_station_id = "0", end_date;
    if (id)
    {
        end_date = convert_datetime_dc (strtokm (NULL, ",")); 
        start_station_id = strtokm (NULL, ",");
        start_station_id = "dc" + start_station_id;
        start_station_name = strtokm (NULL, ",");
        end_station_id = strtokm (NULL, ",");
        end_station_id = "dc" + end_station_id;
        end_station_name = strtokm (NULL, ",");
    } else
    {
        if (end_date_first)
        {
            end_date = convert_datetime_dc (strtokm (NULL, ",")); 
            start_station_name = strtokm (NULL, ",");
        } else
        {
            start_station_name = strtokm (NULL, ",");
            end_date = convert_datetime_dc (strtokm (NULL, ",")); 
        }
        end_station_name = strtokm (NULL, ",");

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
        bike_id = strtokm (NULL, ",");
        user_type = strtokm (NULL, ",");
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
//' @param line Line of data read from citibike file
//'
//' @noRd
unsigned read_one_line_london (sqlite3_stmt * stmt, char * line)
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

    unsigned res = 0;
    if (start_date == "" || end_date == "")
        res = 1;

    return res;
}


//' add_0_to_la_time
//'
//' The hours part of LA times are single digit for HH < 10. SQLite requires
//' strict HH, so this function inserts an extra zero where needed.
//'
//' @param time A datetime column from the LA data
//'
//' @return datetime with two-digit hour field
//'
//' @noRd
std::string add_0_to_la_time (std::string time)
{
    unsigned gap_pos = time.find (" ");
    unsigned time_div_pos = time.find (":");
    if ((time_div_pos - gap_pos) == 2)
        time = time.substr (0, gap_pos + 1) + "0" +
            time.substr (time_div_pos - 1, time.length () - 1);

    return time;
}


//' read_one_line_la
//'
//' @param stmt An sqlit3 statement to be assembled by reading the line of data
//' @param line Line of data read from citibike file
//' @param stationqry Sqlite3 query for station data table to be subsequently
//'        passed to 'import_to_station_table()'
//'
//' @noRd
unsigned read_one_line_la (sqlite3_stmt * stmt, char * line,
        std::map <std::string, std::string> * stationqry)
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
    unsigned ret = 0;

    std::string trip_duration = std::strtok (NULL, delim);
    std::string start_date = std::strtok (NULL, delim);
    start_date = add_0_to_la_time (start_date);
    start_date = convert_datetime_la (start_date);
    std::string end_date = std::strtok (NULL, delim);
    end_date = add_0_to_la_time (end_date);
    end_date = convert_datetime_la (end_date);
    std::string start_station_id = std::strtok (NULL, delim);
    if (start_station_id == " ")
        ret = 1;
    start_station_id = "la" + start_station_id;
    std::string start_station_lat = std::strtok (NULL, delim);
    std::string start_station_lon = std::strtok (NULL, delim);
    // lat and lons are sometimes empty, which is useless 
    if (stationqry->count(start_station_id) == 0 &&
            start_station_lat != " " && start_station_lon != " ") 
    {
        std::string start_station_name = "";
        (*stationqry)[start_station_id] = "(\'la\',\'" + 
            start_station_id + "\',\'\'," + // no start_station_name
            start_station_lat + delim + start_station_lon + ")";
    }

    std::string end_station_id = std::strtok (NULL, delim);
    if (end_station_id == " ")
        ret = 1;
    end_station_id = "la" + end_station_id;
    std::string end_station_lat = std::strtok (NULL, delim);
    std::string end_station_lon = std::strtok (NULL, delim);
    if (stationqry->count(end_station_id) == 0 &&
            end_station_lat != " " && end_station_lon != " ")
    {
        std::string end_station_name = "";
        (*stationqry)[end_station_id] = "(\'la\',\'" + 
            end_station_id + "\',\'\'," +
            end_station_lat + "," + end_station_lon + ")";
    }
    // LA only has duration of membership as (30 = monthly, etc)
    std::string user_type = std::strtok (NULL, delim);
    if (user_type == "")
        user_type = "0";
    else
        user_type = "1";

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
