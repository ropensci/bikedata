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

//' read_one_line_generic
//'
//' Generic routine that works for all systems with well structure data files -
//' meaning files that do not change structure (including patterns of quotation)
//' within a single file. Systems may change structure between files, because
//' file structure is auto-detected at start of each read.
//'
//' @param stmt An sqlit3 statement to be assembled by reading the line of data
//' @param line Line of data read from citibike file
//' @param stationqry Sqlite3 query for station data table to be subsequently
//'        passed to 'import_to_station_table()'
//' @param HeaderStruct from common.h; if filled by examining the file.
//' @param stn_map Only used for cities which don't have proper station ID
//' codes, so that names can be mapped to these (currently just BO & DC).
//'
//' @noRd
unsigned int city::read_one_line_generic (sqlite3_stmt * stmt, char * line,
        std::map <std::string, std::string> * stationqry,
        const std::string city, const HeaderStruct &headers,
        std::map <std::string, std::string> &stn_map)
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
        token = utils::strtokm (&linestr[0u], "\""); // opening quote
        if (headers.quoted [1])
            token = utils::strtokm (nullptr, delim_q_q);
        else
            token = utils::strtokm (nullptr, delim_q_noq);
        (void) token; // suppress unused variable warning
    } else
    {
        if (headers.quoted [1])
            token = utils::strtokm (&linestr[0u], delim_noq_q);
        else
            token = utils::strtokm (&linestr[0u], delim_noq_noq);
    }

    unsigned int pos;
    if (headers.position_file2db [0] >= 0)
    {
        pos = static_cast <unsigned int> (headers.position_file2db [0]);
        values [static_cast <size_t> (headers.position_file2db [0])] = token;
    } else
        pos = INT_MAX;

    if (pos == 1 || pos == 2) // happens for MN
    {
        if (values [pos].length () == 0)
            return 1;
        values [pos] = utils::convert_datetime (values [pos]);
    }

    // These don't arise in any data processed to date
    if (pos == 3 || pos == 7) // add city prefixes to station names
        values [pos] = city + values [pos];
    if (pos == 12) // user type
        values [pos] = city::convert_usertype (values [pos]);
    if (pos == 14) // gender
        values [pos] = city::convert_gender (values [pos]);

    for (unsigned int i = 1; i < headers.nvalues; i++)
    {
        if (headers.position_file2db [i] >= 0)
            pos = static_cast <unsigned int> (headers.position_file2db [i]);
        else
            pos = INT_MAX;

        if (headers.quoted [i])
        {
            if (headers.quoted [i + 1])
                token = utils::strtokm (nullptr, delim_q_q);
            else
                token = utils::strtokm (nullptr, delim_q_noq);
        } else
        {
            if (headers.quoted [i + 1])
                token = utils::strtokm (nullptr, delim_noq_q);
            else
                token = utils::strtokm (nullptr, delim_noq_noq);
        }

        std::string tks = token;
        // sometimes (in London) string that should be quoted yet are missing
        // have no empty quotes, and so the parsing is mucked up. This
        // nevertheless always leaves empty commas at the start, so
        if (tks.substr (0, 1) == ",")
            return 1;
        if (i == (headers.nvalues - 1) && headers.terminal_quote)
            boost::replace_all (tks, "\"", "");

        if (pos < INT_MAX)
        {
            values [pos] = tks;

            if (pos == 1 || pos == 2)
            {
                // some London files have missing datetime strings:
                if (values [pos].length () == 0)
                    return 1;
                values [pos] = utils::convert_datetime (values [pos]);
            }

            if (pos == 3 || pos == 7) // add city prefixes to station names
                values [pos] = city + values [pos];

            if (pos == 12) // user type
                values [pos] = city::convert_usertype (values [pos]);

            if (pos == 14) // gender
                values [pos] = city::convert_gender (values [pos]);
        }
    }

    if (values [0] == "\"\"")
        values [0] = std::to_string (utils::timediff (values [1], values [2]));

    // Use stn_maps for cities which don't have proper station ID values
    if (utils::strfound (city, "bo"))
    {
        values [3] = city::convert_bo_stn_name (values [4], stn_map);
        values [7] = city::convert_bo_stn_name (values [8], stn_map);
    }

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
        std::string stn_id = values [3], stn_name = values [4], lon, lat;
        if (utils::strfound (city, "sf"))
        {
            lon = values [5];
            lat = values [6];
        }
        else
        {
            lon = values [5];
            lat = values [6];
        }
        boost::replace_all (stn_name, "\'", "");
        if (stationqry->count (stn_id) == 0 && lon != "0.0" && lat != "0.0" &&
                lon != "" && lat != "")
            (*stationqry)[stn_id] = "(\'" + city + "\',\'" + stn_id + "\',\'" +
                stn_name + "\'," + lon + "," + lat + ")";

        // end station:
        stn_id = values [7];
        stn_name = values [8];
        if (utils::strfound (city, "sf"))
        {
            lon = values [9];
            lat = values [10];
        } else
        {
            lon = values [9];
            lat = values [10];
        }
        boost::replace_all (stn_name, "\'", "");
        if (stationqry->count (stn_id) == 0 && lon != "0.0" && lat != "0.0" &&
                lon != "" && lat != "")
            (*stationqry)[stn_id] = "(\'" + city + "\',\'" + stn_id + "\',\'" +
                stn_name + "\'," + lon + "," + lat + ")";
    }

    return 0;
}

//' read_one_line_london
//'
//' @param stmt An sqlit3 statement to be assembled by reading the line of data
//' @param line Line of data read from Santander cycles file
//'
//' @noRd
unsigned int city::read_one_line_london (sqlite3_stmt * stmt, char * line)
{
    std::string in_line = line;

    // London is done with str_token which uses std::strings because
    // end_station_names are sometimes but not always surrounded by double
    // quotes.  They also sometimes have commas, but if so they  always have
    // double quotes. It is therefore necessary to get relative positions of
    // commas and double quotes, and this is much easier to do with strings than
    // with char arrays. Only disadvantage: Somewhat slower.
    std::string duration = utils::str_token (&in_line, ","); // Rental ID: not used
    duration = utils::str_token (&in_line, ",");
    std::string bike_id = utils::str_token (&in_line, ",");
    std::string end_date = utils::convert_datetime_dmy (utils::str_token (&in_line, ","));
    std::string end_station_id = utils::str_token (&in_line, ",");
    end_station_id = "lo" + end_station_id;
    std::string end_station_name;
    if (strcspn (in_line.c_str (), "\"") == 0) // name in quotes
    {
        end_station_name = utils::str_token (&in_line, "\",");
        end_station_name = end_station_name.substr (1, 
                end_station_name.length ()); // rm quote from start
        in_line = in_line.substr (1, in_line.length ()); // rm comma from start
    } else
        end_station_name = utils::str_token (&in_line, ",");
    std::string start_date = utils::convert_datetime_dmy (utils::str_token (&in_line, ","));
    std::string start_station_id = utils::str_token (&in_line, ",");
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
unsigned int city::read_one_line_nabsa (sqlite3_stmt * stmt, char * line,
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
    start_date = utils::convert_datetime (start_date);
    std::string end_date = std::strtok (nullptr, delim);
    end_date = utils::convert_datetime (end_date);
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


std::string city::convert_usertype (std::string ut)
{
    // see comment in sqlite3db-add-data.cpp/get_field_positions - this is not
    // locale-safe!
    std::transform (ut.begin (), ut.end (), ut.begin (), ::tolower);
    boost::replace_all (ut, " ", "");
    if (utils::strfound (ut, "member") || utils::strfound (ut, "subscriber") ||
            utils::strfound (ut, "flex") || utils::strfound (ut, "monthly") ||
            utils::strfound (ut, "indego30") || utils::strfound (ut, "1"))
        ut = "1";
    else
        ut = "0";
    return ut;
}

std::string city::convert_gender (std::string g)
{
    if (utils::strfound (g, "Female") || utils::strfound (g, "F") ||
            utils::strfound (g, "2"))
        g = "2";
    else if (utils::strfound (g, "Male") || utils::strfound (g, "M") ||
            utils::strfound (g, "1"))
        g = "1";
    else
        g = "0";
    return g;
}

//' Convert names of Boston stations as given in trip files to standard names
//'
//' @param station_name String as read from trip file
//' @param stn_map Map of station names to IDs
//'
//' @noRd
std::string city::convert_bo_stn_name (std::string &station_name,
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
std::string city::convert_dc_stn_name (std::string &station_name, bool id,
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
