#include <string>
#include <vector>
#include <map>
#include <sqlite3.h>
#include <spatialite.h>

#define BUFFER_SIZE 512

// [[Rcpp::depends(BH)]]
#include <Rcpp.h>

#include <boost/algorithm/string/replace.hpp>

//' strtokm
//'
//' A string delimiter function based on strtok
//' Accessed from StackOverflow (using M Oehm):
//' http://stackoverflow.com/questions/29847915/implementing-strtok-whose-delimiter-has-more-than-one-character
//'
//' @noRd
char *strtokm(char *str, const char *delim)
{
    static char *tok;
    static char *next;
    char *m;

    if (delim == NULL) return NULL;

    tok = (str) ? str : next;
    if (tok == NULL) return NULL;

    m = strstr(tok, delim);

    if (m) {
        next = m + strlen(delim);
        *m = '\0';
    } else {
        next = NULL;
    }

    return tok;
}

//' compare_version_numbers
//'
//' Function to compare version numbers
//' First argument is compared to the second argument
//' Return value:
//' -1 = Argument one version lower than Argument two version
//' 0 = Argument one version equal to Argument two version
//' 1 = Argument one version higher than Argument two version
//'
//' @noRd
int compare_version_numbers (std::string vstro, std::string compvstro) {
  
  int versiondiff = 0;
  
  char *vstr = (char *)vstro.c_str();
  char *compvstr = (char *)compvstro.c_str();
  
  char *vstrtok, *compvstrtok;
  char *vstrtokptr, *compvstrtokptr;
  
  vstrtok = strtok_r(vstr, ".", &vstrtokptr);
  compvstrtok = strtok_r (compvstr, ".", &compvstrtokptr);

  if (atoi(vstrtok) < atoi(compvstrtok)) {
    versiondiff = -1;
  }
  else if (atoi(vstrtok) > atoi(compvstrtok)) {
    versiondiff = 1;
  }
  else {
    while (vstrtok != NULL && compvstrtok != NULL && versiondiff == 0) {

      vstrtok = strtok_r (NULL, ".", &vstrtokptr);
      compvstrtok = strtok_r (NULL, ".", &compvstrtokptr);

      if (vstrtok == NULL && compvstrtok == NULL) {
        versiondiff = 0;
      }
      else if (vstrtok == NULL && compvstrtok != NULL) {
        if (atoi(compvstrtok) == 0) {
          versiondiff = 0;
        }
        else {
          versiondiff = -1;
        }
      }
      else if (vstrtok != NULL && compvstrtok == NULL) {
        if (atoi(vstrtok) == 0) {
          versiondiff = 0;
        }
        else {
          versiondiff = 1;
        }
      }
      else if (atoi(vstrtok) < atoi(compvstrtok)) {
        versiondiff = -1;
      }
      else if (atoi(vstrtok) > atoi(compvstrtok)) {
        versiondiff = 1;
      }

    }

  }
  
  return versiondiff;

}

//' rm_dos_end
//'
//' Remove dos line ending from a character string
//'
//' @noRd
void rm_dos_end (char *str)
{
    char *p = strrchr (str, '\r');
    if (p && p[1]=='\n' && p[2]=='\0') 
        p[0] = '\0';
}

//' line_has_quotes
//'
//' Determine whether or not fields within a line are separated by double quotes
//' and a comma, or just comma separated.
//'
//' @param line Character string with or without double-quote-comma separation
//'
//' @return true if line is delimited by double quotes and commas, false if
//' commas only.
//'
//' @noRd
bool line_has_quotes (char * line)
{
    bool has_quotes = false;
    unsigned slen = strlen (line);
    char * b;
    b = (char*) line;
    char ch;
    for (unsigned i = 0; i<slen; ++i)
    {
        strncpy (&ch, b+i, 1);
        if (ch == '\"')
        {
            has_quotes = true;
            break;
        }
    }
    return has_quotes;
}

//' convert_datetime
//'
//' Datetime strings for NYC change between 08/2014 and 09/2014 from
//' yyyy-mm-dd HH:MM:SS to m/d/yyyy HH:MM:SS. sqlite3 can't combine dates in
//' different formats, so this converts the latter to former formats.
//'
//' @noRd
std::string convert_datetime (std::string str)
{
    // NOTE that the following does not work for some reason?
    //if (size_t ipos = str.find ("/") != std::string::npos)
    if (str.find ("/") != std::string::npos)
    {
        size_t ipos = str.find ("/");
        std::string mm = str.substr (0, ipos);
        if (ipos == 1)
            mm = std::string ("0") + mm;
        str = str.substr (ipos + 1, str.length () - ipos - 1);
        ipos = str.find ("/");
        std::string dd = str.substr (0, ipos);
        if (ipos == 1)
            dd = std::string ("0") + dd;
        str = str.substr (ipos + 1, str.length () - ipos - 1);
        ipos = str.find (" ");
        std::string yy = str.substr (0, ipos);
        str = str.substr (ipos + 1, str.length () - ipos - 1);
        str = yy + "-" + mm + "-" + dd + " " + str;
    }

    return str;
}

//' read_one_line
//'
//' @noRd
void read_one_line (sqlite3_stmt * stmt, char * line,
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
    std::string startstationid = strtokm(NULL, delim);
    if (stationqry->count(startstationid) == 0) {
        (*stationqry)[startstationid] = "(" + startstationid + "," + 
            strtokm(NULL, delim) + "," + strtokm(NULL, delim) + ",'" + 
            strtokm(NULL, delim) + "')";
    }
    else {
        strtokm(NULL, delim);
        strtokm(NULL, delim);
        strtokm(NULL, delim);
    }

    sqlite3_bind_text(stmt, 5, startstationid.c_str(), -1, SQLITE_TRANSIENT); 

    std::string endstationid = strtokm(NULL, delim);
    if (stationqry->count(endstationid) == 0) {
        (*stationqry)[endstationid] = "(" + endstationid + "," + 
            strtokm(NULL, delim) + "," + strtokm(NULL, delim) + ",'" + 
            strtokm(NULL, delim) + "')";
    }
    else {
        strtokm(NULL, delim);
        strtokm(NULL, delim);
        strtokm(NULL, delim);
    }

    sqlite3_bind_text(stmt, 6, endstationid.c_str(), -1, SQLITE_TRANSIENT); 
    // next two are bike id and user type
    sqlite3_bind_text(stmt, 7, strtokm(NULL, delim), -1, SQLITE_TRANSIENT); 
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

//' importDataToSpatialite
//'
//' Extracts bike data for NYC citibike
//' 
//' @param datafiles A character vector containin the paths to the citibike 
//' .csv files to import.
//' @param spdb A string containing the path to the spatialite database to 
//' use. It will be created automatically.
//' @param quiet If FALSE, progress is displayed on screen
//'
//' @return integer result code
//'
//' @noRd
// [[Rcpp::export]]
int importDataToSpatialite (Rcpp::CharacterVector datafiles, 
        const char* spdb, bool quiet) 
{

    sqlite3 *dbcon;
    char *zErrMsg = 0;
    int rc;
    void* cache = spatialite_alloc_connection();

    rc = sqlite3_open_v2(spdb, &dbcon, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (rc != SQLITE_OK)
        throw std::runtime_error ("Can't establish sqlite3 connection");
    spatialite_init_ex(dbcon, cache, 0);

    rc = sqlite3_exec(dbcon, "SELECT InitSpatialMetadata(1);", NULL, NULL, &zErrMsg);

    std::string createqry = "CREATE TABLE trips ("
        "id serial primary key,"
        "trip_id integer,"
        "trip_duration numeric,"
        "start_time timestamp without time zone,"
        "stop_time timestamp without time zone,"
        "start_station_id integer,"
        "end_station_id integer,"
        "bike_id integer,"
        "user_type varchar,"
        "birth_year integer,"
        "gender integer"
        ");"
        "CREATE TABLE stations ("
        "    id integer primary key,"
        "    name varchar,"
        "    latitude numeric,"
        "    longitude numeric"
        ");";

    rc = sqlite3_exec(dbcon, createqry.c_str(), NULL, NULL, &zErrMsg);
    FILE * pFile;
    char in_line [BUFFER_SIZE] = "\0";
    char sqlqry [BUFFER_SIZE] = "\0";

    sqlite3_stmt * stmt;
    char * tail = 0;
    std::map <std::string, std::string> stationqry;

    sprintf(sqlqry, "INSERT INTO trips VALUES (NULL, @TI, @TD, @ST, @ET, @SSID, @ESID, @BID, @UT, @BY, @GE)");

    sqlite3_prepare_v2(dbcon, sqlqry, BUFFER_SIZE, &stmt, NULL);

    sqlite3_exec(dbcon, "BEGIN TRANSACTION", NULL, NULL, &zErrMsg);

    int trip_id = 0;
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
            sqlite3_bind_text(stmt, 1, std::to_string (trip_id).c_str(), -1, SQLITE_TRANSIENT); // Trip ID
            read_one_line (stmt, in_line, &stationqry, delim);
            trip_id++;

            sqlite3_step(stmt);
            sqlite3_reset(stmt);
        }
    }

    sqlite3_exec(dbcon, "END TRANSACTION", NULL, NULL, &zErrMsg);

    std::string fullstationqry = "INSERT INTO stations "
                                 "(id, longitude, latitude, name) VALUES ";
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

    rc = sqlite3_close_v2(dbcon);
    if (rc != SQLITE_OK)
        throw std::runtime_error ("Unable to close sqlite database");

    return(trip_id);
}


//' create_db_indexes
//'
//' Creates the specified indexes in the database to speed up queries. Note
//' that for the full dataset this may take some time.
//' 
//' @param spdb A string containing the path to the spatialite database to 
//' use.
//' @param tables A vector with the tables for which to create indexes. This
//' vector should be the same length as the cols vector.
//' @param cols A vector with the fields for which to create indexes.
//'
//' @return integer result code
//'
//' @noRd
// [[Rcpp::export]]
int create_db_indexes (const char* spdb,
                            Rcpp::CharacterVector tables,
                            Rcpp::CharacterVector cols) 
{
  
  sqlite3 *dbcon;
  char *zErrMsg = 0;
  const char *zStmtMsg;
  int rc;
  void* cache = spatialite_alloc_connection();
  
  rc = sqlite3_open_v2(spdb, &dbcon, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
  if (rc != SQLITE_OK)
    throw std::runtime_error ("Can't establish sqlite3 connection");
  spatialite_init_ex(dbcon, cache, 0);
  
  char *idxsql = NULL;
  sqlite3_stmt *versionstmt;
  char *sqliteversion = (char *)"0.1";
  
  
  
  if (rc == SQLITE_OK) {
    
    rc = sqlite3_prepare_v2(dbcon, "SELECT sqlite_version()", -1, &versionstmt, 0);
    if (rc != SQLITE_OK) {
      throw std::runtime_error ("Unable to retrieve sqlite version");
    }
    rc = sqlite3_step(versionstmt);
    
    if (rc == SQLITE_ROW) {
      sqliteversion = (char *)sqlite3_column_text(versionstmt, 0);
    }
    rc = sqlite3_reset(versionstmt);

    for (unsigned int i = 0; i < cols.length(); ++i) {
    
      if (((std::string)cols[i]).find("(") == std::string::npos || 
          compare_version_numbers(sqliteversion, "3.9.0") >= 0) {
        
        std::string idxname = "idx_" + tables[i] + "_" + (std::string)cols[i];
        boost::replace_all(idxname, "(", "_");
        boost::replace_all(idxname, ")", "_");
        boost::replace_all(idxname, " ", "_");
        
        int sprrc = asprintf(&idxsql, "CREATE INDEX %s ON %s(%s)", idxname.c_str(), (char *)(tables[i]), (char *)(cols[i]));
        
        rc = sqlite3_exec(dbcon, idxsql, NULL, NULL, &zErrMsg);
        if (rc != SQLITE_OK) {
          throw std::runtime_error ("Unable to execute index query: " + (std::string)idxsql);
        }
        
      }
      else {
        Rcpp::warning("Unable to create index on " + cols[i] + ", expression not supported in SQLite version < 3.9.0");
      }
    
    } 
  
  }
  
  rc = sqlite3_close_v2(dbcon);
  if (rc != SQLITE_OK) {
    throw std::runtime_error ("Unable to close sqlite database");
  }
  
  return(rc);

}
