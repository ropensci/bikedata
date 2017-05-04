# miscellaneous functions for retrieving summary stats from database
#
# *****************************************************
# *****************************************************
# ***                                               ***
# ***            NON-EXPORTED FUNCTIONS             ***
# ***                                               ***
# *****************************************************
# *****************************************************

#' Check whether indexes have been created for database
#'
#' @param bikedb A string containing the path to the SQLite3 database.
#'
#' @noRd
indexes_exist <- function (bikedb)
{
    db <- RSQLite::dbConnect (RSQLite::SQLite(), bikedb, create = FALSE)
    idx_list <- dbGetQuery (db, "PRAGMA index_list (trips)")
    RSQLite::dbDisconnect (db)
    nrow (idx_list) > 2 # 2 because city index is automatically created
}

#' Count number of entries in sqlite3 database tables
#'
#' @param bikedb A string containing the path to the SQLite3 database.
#' @param trips If true, numbers of trips are counted; otherwise numbers of
#' stations
#' @param city Optional city for which numbers of trips are to be counted
#'
#' @noRd
bike_db_totals <- function (bikedb, trips = TRUE, city)
{
    db <- RSQLite::dbConnect (RSQLite::SQLite(), bikedb, create = FALSE)
    if (trips)
        qry <- "SELECT Count(*) FROM trips"
    else
        qry <- "SELECT Count(*) FROM stations"
    if (!missing (city))
        qry <- paste0 (qry, " WHERE city = '", city, "'")
    ntrips <- dbGetQuery (db, qry)
    RSQLite::dbDisconnect (db)
    return (as.numeric (ntrips))
}

#' Count number of datafiles in sqlite3 database
#'
#' @param bikedb A string containing the path to the SQLite3 database.
#'
#' @noRd
num_datafiles_in_db <- function (bikedb)
{
    db <- RSQLite::dbConnect (RSQLite::SQLite(), bikedb, create = FALSE)
    ntrips <- dbGetQuery (db, "SELECT Count(*) FROM datafiles")
    RSQLite::dbDisconnect (db)
    return (as.numeric (ntrips))
}

#' List the cities with data containined in SQLite3 database
#'
#' @param bikedb A string containing the path to the SQLite3 database.
#'
#' @noRd
bike_cities_in_db <- function (bikedb)
{
    db <- RSQLite::dbConnect (RSQLite::SQLite(), bikedb, create = FALSE)
    cities <- dbGetQuery (db, "SELECT city FROM stations")
    RSQLite::dbDisconnect (db)
    cities <- unique (cities)
    rownames (table (cities)) # TODO: Find a better way to do that
}

#' Return names of files in nominated directory that are **not** already in
#' database
#'
#' @param bikedb A string containing the path to the SQLite3 database.
#' @param flist_zip A character vector listing the names of \code{.zip} files
#'          for a particular city as returned from \code{get_flist_city}
#'
#' @return Vector with members of \code{flist_zip} that have not already been
#'          read into the database
#'
#' @noRd
get_new_datafiles <- function (bikedb, flist_zip)
{
    db <- dplyr::src_sqlite (bikedb, create = F)
    old_files <- dplyr::collect (dplyr::tbl (db, 'datafiles'))$name
    flist_zip [which (!basename (flist_zip) %in% old_files)]
}

#' Get dates of first and last trips, and number of days in between, for each
#' station
#'
#' @param bikedb A string containing the path to the SQLite3 database.
#' @param city City for which dates are to be extracted
#'
#' @return Four-column \code{data.frame} of dates of first and last trips for
#' each station, number of days between those dates, and station IDs. 
#'
#' @noRd
bike_station_dates <- function (bikedb, city)
{
    db <- RSQLite::dbConnect(SQLite(), bikedb, create = FALSE)
    qry <- paste0 ("SELECT MIN (STRFTIME('%Y-%m-%d', start_time)) AS 'first',",
                   "MAX (STRFTIME('%Y-%m-%d', start_time)) AS 'last',",
                   "start_station_id AS 'station' FROM trips WHERE city = '",
                   city, "' GROUP BY start_station_id")
    dates <- RSQLite::dbGetQuery(db, qry)
    RSQLite::dbDisconnect(db)
    # re-order stations to numeric order
    stn <- as.numeric (substr (dates$station, 3, 10)) # 10 = arbitrarily length
    dates <- dates [order (stn), c (3, 1, 2)] # station ID in 1st column
    # Then add numbers of days in operation
    ndays <- lubridate::interval (dates$first, dates$last) / lubridate::ddays (1)
    dates <- data.frame (first = dates$first,
                         last = dates$last,
                         ndays = ndays,
                         station = dates$station)
    rownames (dates) <- NULL

    return (dates)
}

# *****************************************************
# *****************************************************
# ***                                               ***
# ***              EXPORTED FUNCTIONS               ***
# ***                                               ***
# *****************************************************
# *****************************************************


#' Check whether files in database are the latest published files
#'
#' @param bikedb A string containing the path to the SQLite3 database.
#' If no directory specified, it is presumed to be in \code{tempdir()}.
#'
#' @return A named vector of binary values: TRUE is files in \code{bikedb} are
#' the latest versions; otherwise FALSE, in which case \code{store_bikedata}
#' could be run to update the database.
#'
#' @export
#'
#' @examples
#' data_dir <- getwd ()
#' bike_write_test_data (data_dir = data_dir)
#' # or download some real data!
#' # dl_bikedata (city = 'la', data_dir = data_dir)
#' bikedb <- file.path (data_dir, 'testdb')
#' store_bikedata (data_dir = data_dir, bikedb = bikedb)
#' # bike_latest_files (bikedb)
#' # All false because test data are not current, but would pass with real data
#' 
#' bike_rm_test_data (data_dir = data_dir)
#' bike_rm_db (bikedb)
#' # don't forget to remove real data!
#' # file.remove (list.files (data_dir, pattern = '.zip'))
bike_latest_files <- function (bikedb)
{
    if (!grepl ('/', bikedb) | !grepl ('*//*', bikedb))
        bikedb <- file.path (tempdir (), bikedb)

    db <- RSQLite::dbConnect (RSQLite::SQLite(), bikedb, create = FALSE)
    files <- dbGetQuery (db, "SELECT * FROM datafiles")
    cities <- unique (files$city)
    RSQLite::dbDisconnect (db)

    latest_files <- rep (TRUE, length (cities))
    names (latest_files) <- cities
    for (i in seq (cities))
    {
        files_i <- files$name [which (files$city == cities [i])]
        all_files <- basename (get_bike_files (city = cities [i]))
        if (sort (all_files, decreasing = TRUE) [1] !=
            sort (files_i, decreasing = TRUE) [1])
            latest_files [i] <- FALSE
    }
    return (latest_files)
}

#' Extract date-time limits from trip database
#'
#' @param bikedb A string containing the path to the SQLite3 database.
#' If no directory specified, it is presumed to be in \code{tempdir()}.
#' @param city If given, date limits are calculated only for trips in 
#'          that city.
#'
#' @return A vector of 2 elements giving the date-time of the first and last
#' trips
#'
#' @export
#'
#' @examples
#' bike_write_test_data (data_dir = '.')
#' # dl_bikedata (city = 'la', data_dir = '.') # or download some real data!
#' store_bikedata (data_dir = '.', bikedb = 'testdb')
#' bike_datelimits ('testdb') # overall limits for all cities
#' bike_datelimits ('testdb', city = 'NYC') 
#' bike_datelimits ('testdb', city = 'los angeles') 
#' bike_rm_test_data (data_dir = '.')
#' bike_rm_db ('testdb')
#' # don't forget to remove real data!
#' # file.remove (list.files ('.', pattern = '.zip'))
bike_datelimits <- function (bikedb, city)
{
    if (!grepl ('/', bikedb) | !grepl ('*//*', bikedb))
        bikedb <- file.path (tempdir (), bikedb)

    qry_min <- "SELECT MIN(start_time) FROM trips"
    qry_max <- "SELECT MAX(start_time) FROM trips"
    if (!missing (city))
    {
        city <- convert_city_names (city)
        qry_min <- paste0 (qry_min, " WHERE city = '", city, "'")
        qry_max <- paste0 (qry_max, " WHERE city = '", city, "'")
    }

    db <- RSQLite::dbConnect (RSQLite::SQLite(), bikedb, create = FALSE)
    first_trip <- RSQLite::dbGetQuery (db, qry_min) [1, 1]
    last_trip <- RSQLite::dbGetQuery (db, qry_max) [1, 1]
    RSQLite::dbDisconnect(db)

    res <- c (first_trip, last_trip)
    names (res) <- c ('first', 'last')
    return (res)
}

#' Extract summary statistics of database
#'
#' @param bikedb A string containing the path to the SQLite3 database.
#' If no directory specified, it is presumed to be in \code{tempdir()}.
#'
#' @return A \code{data.frame} containing numbers of trips and stations along
#' with times and dates of first and last trips for each city in database and a
#' final column indicating whether the files match the latest published
#' versions.
#'
#' @export
#'
#' @examples
#' bike_write_test_data (data_dir = '.')
#' # dl_bikedata (city = 'la', data_dir = '.') # or download some real data!
#' store_bikedata (data_dir = '.', bikedb = 'testdb')
#' bike_summary_stats ('testdb')
#' bike_rm_test_data (data_dir = '.')
#' bike_rm_db ('testdb')
#' # don't forget to remove real data!
#' # file.remove (list.files ('.', pattern = '.zip'))
bike_summary_stats <- function (bikedb)
{
    if (!grepl ('/', bikedb) | !grepl ('*//*', bikedb))
        bikedb <- file.path (tempdir (), bikedb)

    cities <- bike_cities_in_db (bikedb)

    num_trips <- bike_db_totals (bikedb, TRUE)
    num_stations <- bike_db_totals (bikedb, FALSE)
    dates <- rbind (c (NULL, NULL), bike_datelimits (bikedb)) # so [,1] works
    rnames <- cities

    latest_files <- bike_latest_files (bikedb)
    if (length (cities) > 1)
    {
        latest <- NULL # latest_files aren't necessarily in db order
        rnames <- c ('all', cities)
        for (ci in cities)
        {
            num_trips <- c (num_trips, bike_db_totals (bikedb, TRUE, city = ci))
            num_stations <- c (num_stations, bike_db_totals (bikedb, FALSE, 
                                                             city = ci))
            dates <- rbind (dates, bike_datelimits (bikedb, city = ci))
            latest <- c (latest, 
                         latest_files [which (names (latest_files) == ci)])
        }
        latest_files <- c (NA, latest)
    }

    res <- data.frame (num_trips = num_trips, num_stations = num_stations,
                       first_trip = dates [,1], last_trip = dates [,2],
                       latest_files = latest_files)
    rownames (res) <- rnames
    return (res)
}

#' Extract daily trip counts for all stations
#'
#' @param bikedb A string containing the path to the SQLite3 database.
#' If no directory specified, it is presumed to be in \code{tempdir()}.
#' @param city City for which trips are to be counted - mandatory if database
#' contains data for more than one city
#' @param station Optional argument specifying bike station for which trips are
#' to be counted
#'
#' @return A \code{data.frame} containing daily dates and total numbers of trips
#'
#' @export
#'
#' @examples
#' bike_write_test_data (data_dir = '.')
#' # dl_bikedata (city = 'la', data_dir = '.') # or download some real data!
#' store_bikedata (data_dir = '.', bikedb = 'testdb')
#' bike_daily_trips (bikedb = 'testdb', city = 'ny')
#' bike_rm_test_data (data_dir = '.')
#' bike_rm_db ('testdb')
#' # don't forget to remove real data!
#' # file.remove (list.files ('.', pattern = '.zip'))
bike_daily_trips <- function (bikedb, city, station)
{
    if (!grepl ('/', bikedb) | !grepl ('*//*', bikedb))
        bikedb <- file.path (tempdir (), bikedb)

    cities <- bike_cities_in_db (bikedb)
    if (missing (city))
    {
        if (length (cities) > 1)
            stop ('bikedb contains multiple cities; please specify one')
        else 
            city <- cities
    } else
        city <- convert_city_names (city)

    qry <- paste0 ("SELECT STRFTIME('%Y-%m-%d', start_time) AS 'date', COUNT() AS ",
                  "'ntrips' FROM trips WHERE city = '", city, "'")
    db <- RSQLite::dbConnect (RSQLite::SQLite(), bikedb, create = FALSE)
    if (!missing (station))
    {
        if (substring (stn, 1, 2) != city)
            stn <- paste0 (city, stn)
        # Then just check that station is in stations table
        stns <- dbGetQuery (db, "SELECT stn_id FROM stations")$stn_id
        if (!stn %in% stns)
            stop ('Station ', stn, ' does not exist in database')
        qry <- paste0 (qry, " AND start_station_id = '", station, "'")
    }
    qry <- paste0 (qry, " GROUP BY STRFTIME('%Y-%m-%d', date);")
    ret <- dbGetQuery (db, qry)
    RSQLite::dbDisconnect (db)

    ret$date <- as.Date (ret$date)

    return (ret)
}
