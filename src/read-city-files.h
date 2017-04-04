#pragma once
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

void read_one_line_nyc (sqlite3_stmt * stmt, char * line,
        std::map <std::string, std::string> * stationqry, const char * delim);
void read_one_line_boston (sqlite3_stmt * stmt, char * line,
        std::map <std::string, std::string> * stationqry);
void read_one_line_chicago (sqlite3_stmt * stmt, char * line,
        const char * delim);


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

    sqlite3_bind_text(stmt, 2, token, -1, SQLITE_TRANSIENT); // Trip duration

    std::string tempstr = convert_datetime (strtokm(NULL, delim)); // Start time
    sqlite3_bind_text(stmt, 3, tempstr.c_str(), -1, SQLITE_TRANSIENT); 

    tempstr = convert_datetime (strtokm(NULL, delim)); // Stop time
    sqlite3_bind_text(stmt, 4, tempstr.c_str(), -1, SQLITE_TRANSIENT); 
    std::string start_station_id = strtokm(NULL, delim);
    start_station_id = "ny" + start_station_id;
    if (stationqry->count(start_station_id) == 0) {
        std::string start_station_name = strtokm(NULL, delim);
        std::string start_station_lat = strtokm(NULL, delim);
        std::string start_station_lon = strtokm(NULL, delim);
        (*stationqry)[start_station_id] = "(\'ny\',\'" + 
            start_station_id + "\',\'" + start_station_name + "\'," +
            start_station_lat + "," + start_station_lon + ")";
    }
    else {
        strtokm(NULL, delim); // station name
        strtokm(NULL, delim); // lat
        strtokm(NULL, delim); // lon
    }

    sqlite3_bind_text(stmt, 5, start_station_id.c_str(), -1, SQLITE_TRANSIENT); 

    std::string end_station_id = strtokm(NULL, delim);
    end_station_id = "ny" + end_station_id;
    if (stationqry->count(end_station_id) == 0) {
        std::string end_station_name = strtokm(NULL, delim);
        std::string end_station_lat = strtokm(NULL, delim);
        std::string end_station_lon = strtokm(NULL, delim);
        (*stationqry)[end_station_id] = "(\'ny\',\'" + 
            end_station_id + "\',\'" + end_station_name + "\'," +
            end_station_lat + "," + end_station_lon + ")";
    }
    else {
        strtokm(NULL, delim); // station name
        strtokm(NULL, delim); // lat
        strtokm(NULL, delim); // lon
    }

    sqlite3_bind_text(stmt, 6, end_station_id.c_str(), -1, SQLITE_TRANSIENT); 
    std::string user_type = strtokm(NULL, delim);
    if (user_type == "Subscriber")
        user_type = "1";
    else
        user_type = "0";
    sqlite3_bind_text(stmt, 7, user_type.c_str(), -1, SQLITE_TRANSIENT); 
    // next is bike id 
    sqlite3_bind_text(stmt, 8, strtokm(NULL, delim), -1, SQLITE_TRANSIENT); 
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
    sqlite3_bind_text(stmt, 9, birthyear.c_str(), -1, SQLITE_TRANSIENT); // Birth Year
    sqlite3_bind_text(stmt, 10, gender.c_str(), -1, SQLITE_TRANSIENT); // Gender
}

//' read_one_line_boston
//'
//' @param stmt An sqlit3 statement to be assembled by reading the line of data
//' @param line Line of data read from citibike file
//' @param stationqry Sqlite3 query for station data table to be subsequently
//'        passed to 'import_to_station_table()'
//'
//' @noRd
void read_one_line_boston (sqlite3_stmt * stmt, char * line,
        std::map <std::string, std::string> * stationqry)
{
    // TDOD: Replace strokm with strok here!
    const char * delim = "\",\"";

    std::string in_line2 = line;
    boost::replace_all(in_line2, "\\N","\"\"");
    char * token = strtokm(&in_line2[0u], "\""); // opening quote
    std::string duration = strtokm(NULL, delim);
    std::string start_time = convert_datetime (strtokm(NULL, delim)); 
    std::string end_time = convert_datetime (strtokm(NULL, delim)); 

    std::string start_station_id = strtokm(NULL, delim);
    start_station_id = "bo" + start_station_id;
    if (stationqry->count (start_station_id) == 0) {
        std::string start_station_name = strtokm(NULL, delim);
        boost::replace_all (start_station_name, "\'", ""); // rm apostrophes
        std::string start_station_lat = strtokm(NULL, delim);
        std::string start_station_lon = strtokm(NULL, delim);
        (*stationqry)[start_station_id] = "(\'bo\',\'" + 
            start_station_id + "\',\'" + start_station_name + "\'," +
            start_station_lat + "," + start_station_lon + ")";
    } else
    {
        strtokm(NULL, delim);
        strtokm(NULL, delim);
        strtokm(NULL, delim);
    }


    std::string end_station_id = strtokm(NULL, delim);
    end_station_id = "bo" + end_station_id;
    if (stationqry->count (end_station_id) == 0) {
        std::string end_station_name = strtokm(NULL, delim);
        boost::replace_all (end_station_name, "\'", ""); // rm apostrophes
        std::string end_station_lat = strtokm(NULL, delim);
        std::string end_station_lon = strtokm(NULL, delim);
        (*stationqry)[end_station_id] = "(\'bo\',\'" + 
            end_station_id + "\',\'" + end_station_name + "\'," +
            end_station_lat + "," + end_station_lon + ")";
    } else
    {
        strtokm(NULL, delim);
        strtokm(NULL, delim);
        strtokm(NULL, delim);
    }

    std::string bike_number = strtokm(NULL, delim);
    std::string user_type = strtokm(NULL, delim);
    std::string birth_year = "", gender = "";
    if (user_type == "Subscriber")
    {
        birth_year = strtokm(NULL, delim);
        gender = strtokm(NULL, delim);
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
}

//' read_one_line_chicago
//'
//' @param stmt An sqlit3 statement to be assembled by reading the line of data
//' @param line Line of data read from citibike file
//'
//' @noRd
void read_one_line_chicago (sqlite3_stmt * stmt, char * line,
        const char * delim)
{
    std::string in_line2 = line;
    char * token;
    if (strncmp (delim, "\",\"", 3) == 0)
    {
        //boost::replace_all(in_line2, "\\N","\"\"");
        token = strtokm(&in_line2[0u], "\""); //First double speech marks
        token = strtokm(NULL, delim); 
    } else
        token = strtokm(&in_line2[0u], delim);
    // First token is trip ID, which is not used here

    // trip_id,starttime,stoptime,bikeid,tripduration,
    // from_station_id,from_station_name,
    // to_station_id,to_station_name,usertype,gender,birthday

    std::string start_time = convert_datetime (strtokm(NULL, delim)); 
    std::string end_time = convert_datetime (strtokm(NULL, delim)); 
    std::string bike_id = strtokm(NULL, delim); 
    std::string duration = strtokm(NULL, delim);

    std::string start_station_id = strtokm(NULL, delim);
    start_station_id = "ch" + start_station_id;
    strtokm(NULL, delim); // start station name
    std::string end_station_id = strtokm(NULL, delim);
    end_station_id = "ch" + end_station_id;
    strtokm(NULL, delim); // end station name

    std::string user_type = strtokm(NULL, delim);
    std::string birth_year = "", gender = "";
    if (user_type == "Subscriber")
    {
        gender = strtokm(NULL, delim);
        birth_year = strtokm(NULL, delim);
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
}
