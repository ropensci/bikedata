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
    db <- DBI::dbConnect (RSQLite::SQLite(), bikedb, create = FALSE)
    idx_list <- DBI::dbGetQuery (db, "PRAGMA index_list (trips)")
    DBI::dbDisconnect (db)
    nrow (idx_list) > 2 # 2 because city index is automatically created
}

#' Count number of datafiles in sqlite3 database
#'
#' @param bikedb A string containing the path to the SQLite3 database.
#'
#' @noRd
num_datafiles_in_db <- function (bikedb)
{
    db <- DBI::dbConnect (RSQLite::SQLite(), bikedb, create = FALSE)
    numtrips <- DBI::dbGetQuery (db, "SELECT Count(*) FROM datafiles")
    DBI::dbDisconnect (db)
    return (as.numeric (numtrips))
}

#' List the cities with data containined in SQLite3 database
#'
#' @param bikedb A string containing the path to the SQLite3 database.
#'
#' @noRd
bike_cities_in_db <- function (bikedb)
{
    db <- DBI::dbConnect (RSQLite::SQLite(), bikedb, create = FALSE)
    cities <- DBI::dbGetQuery (db, "SELECT city FROM stations")
    DBI::dbDisconnect (db)
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
    if (!is.null (flist_zip))
    {
        db <- DBI::dbConnect (RSQLite::SQLite(), bikedb, create = FALSE)
        old_files <- DBI::dbReadTable (db, 'datafiles')$name
        DBI::dbDisconnect (db)
        flist_zip <- flist_zip [which (!basename (flist_zip) %in% old_files)]
    }

    return (flist_zip)
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
    db <- DBI::dbConnect (RSQLite::SQLite(), bikedb, create = FALSE)
    qry <- paste0 ("SELECT MIN (STRFTIME('%Y-%m-%d', start_time)) AS 'first',",
                   "MAX (STRFTIME('%Y-%m-%d', start_time)) AS 'last',",
                   "start_station_id AS 'station' FROM trips WHERE city = '",
                   city, "' GROUP BY start_station_id")
    dates <- DBI::dbGetQuery (db, qry)
    DBI::dbDisconnect (db)
    # re-order stations to numeric order
    stn <- as.numeric (substr (dates$station, 3, 10)) # 10 = arbitrarily length
    dates <- dates [order (stn), c (3, 1, 2)] # station ID in 1st column
    # Then add numbers of days in operation
    ndays <- lubridate::interval (dates$first, dates$last) /
        lubridate::ddays (1)
    dates <- data.frame (first = dates$first,
                         last = dates$last,
                         ndays = ndays + 1, # start = end -> 1 day
                         station = dates$station)
    rownames (dates) <- NULL

    dates$first <- as.Date (dates$first)
    dates$last <- as.Date (dates$last)

    return (dates)
}

# *****************************************************
# *****************************************************
# ***                                               ***
# ***              EXPORTED FUNCTIONS               ***
# ***                                               ***
# *****************************************************
# *****************************************************


#' Get names of files read into database
#'
#' @param bikedb A string containing the path to the SQLite3 database.
#' @param city Optional city for which filenames are to be obtained
#'
#' @export
#'
#' @examples
#' data_dir <- tempdir ()
#' bike_write_test_data (data_dir = data_dir)
#' bikedb <- file.path (data_dir, 'testdb')
#' store_bikedata (data_dir = data_dir, bikedb = bikedb)
#' files <- bike_stored_files (bikedb = bikedb)
#' # returns a tibble with names of all stored files
#'
#' bike_rm_test_data (data_dir = data_dir)
#' bike_rm_db (bikedb)
#' # don't forget to remove real data!
#' # file.remove (list.files ('.', pattern = '.zip'))
bike_stored_files <- function (bikedb, city)
{
    if (missing (bikedb))
        stop ("Can't get daily trips if bikedb isn't provided")

    bikedb <- check_db_arg (bikedb)

    db <- DBI::dbConnect (RSQLite::SQLite(), bikedb, create = FALSE)
    qry <- "SELECT * FROM datafiles"
    if (!missing (city))
        qry <- paste0 (qry, " WHERE city = '", city, "'")
    files <- DBI::dbGetQuery (db, qry)
    DBI::dbDisconnect (db)
    return (files)
}

#' Count number of entries in sqlite3 database tables
#'
#' @param bikedb A string containing the path to the SQLite3 database.
#' @param trips If true, numbers of trips are counted; otherwise numbers of
#' stations
#' @param city Optional city for which numbers of trips are to be counted
#'
#' @export
#'
#' @examples
#' data_dir <- tempdir ()
#' bike_write_test_data (data_dir = data_dir)
#' bikedb <- file.path (data_dir, 'testdb')
#' store_bikedata (data_dir = data_dir, bikedb = bikedb)
#' # create database indexes for quicker access:
#' index_bikedata_db (bikedb = bikedb)
#'
#' bike_db_totals (bikedb = bikedb, trips = TRUE) # total trips
#' bike_db_totals (bikedb = bikedb, trips = TRUE, city = 'ch')
#' bike_db_totals (bikedb = bikedb, trips = TRUE, city = 'ny')
#' bike_db_totals (bikedb = bikedb, trips = FALSE) # total stations
#' bike_db_totals (bikedb = bikedb, trips = FALSE, city = 'ch')
#' bike_db_totals (bikedb = bikedb, trips = FALSE, city = 'ny')
#' # numbers of stations can also be extracted with
#' nrow (bike_stations (bikedb = bikedb))
#' nrow (bike_stations (bikedb = bikedb, city = 'ch'))
#' 
#' bike_rm_test_data (data_dir = data_dir)
#' bike_rm_db (bikedb)
#' # don't forget to remove real data!
#' # file.remove (list.files ('.', pattern = '.zip'))
bike_db_totals <- function (bikedb, trips = TRUE, city)
{
    if (missing (bikedb))
        stop ("Can't get daily trips if bikedb isn't provided")

    bikedb <- check_db_arg (bikedb)

    db <- DBI::dbConnect (RSQLite::SQLite(), bikedb, create = FALSE)
    if (trips)
        qry <- "SELECT Count(*) FROM trips"
    else
        qry <- "SELECT Count(*) FROM stations"
    if (!missing (city))
        qry <- paste0 (qry, " WHERE city = '", city, "'")
    numtrips <- DBI::dbGetQuery (db, qry)
    DBI::dbDisconnect (db)
    return (as.numeric (numtrips))
}


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
#' data_dir <- tempdir ()
#' bike_write_test_data (data_dir = data_dir)
#' # or download some real data!
#' # dl_bikedata (city = 'la', data_dir = data_dir)
#' # Remove one London file that triggers an API call which may fail tests:
#' file.remove (file.path (tempdir(), "01aJourneyDataExtract10Jan16-23Jan16.csv"))
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
    if (missing (bikedb))
        stop ("Can't get latest files if bikedb isn't provided")

    bikedb <- check_db_arg (bikedb)

    db <- DBI::dbConnect (RSQLite::SQLite(), bikedb, create = FALSE)
    files <- DBI::dbGetQuery (db, "SELECT * FROM datafiles")
    cities <- unique (files$city)
    DBI::dbDisconnect (db)

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
#' data_dir <- tempdir ()
#' bike_write_test_data (data_dir = data_dir)
#' # dl_bikedata (city = 'la', data_dir = data_dir) # or download some real data!
#' # Remove one London file that triggers an API call which may fail tests:
#' file.remove (file.path (tempdir(), "01aJourneyDataExtract10Jan16-23Jan16.csv"))
#' bikedb <- file.path (data_dir, 'testdb')
#' store_bikedata (data_dir = data_dir, bikedb = bikedb)
#' # create database indexes for quicker access:
#' index_bikedata_db (bikedb = bikedb)
#'
#' bike_datelimits ('testdb') # overall limits for all cities
#' bike_datelimits ('testdb', city = 'NYC') 
#' bike_datelimits ('testdb', city = 'los angeles') 
#' 
#' bike_rm_test_data (data_dir = data_dir)
#' bike_rm_db (bikedb)
#' # don't forget to remove real data!
#' # file.remove (list.files ('.', pattern = '.zip'))
bike_datelimits <- function (bikedb, city)
{
    if (missing (bikedb))
        stop ("Can't get date limits if bikedb isn't provided")

    bikedb <- check_db_arg (bikedb)

    qry_min <- "SELECT MIN(start_time) FROM trips"
    qry_max <- "SELECT MAX(start_time) FROM trips"
    if (!missing (city))
    {
        city <- convert_city_names (city)
        qry_min <- paste0 (qry_min, " WHERE city = '", city, "'")
        qry_max <- paste0 (qry_max, " WHERE city = '", city, "'")
    }

    db <- DBI::dbConnect (RSQLite::SQLite(), bikedb, create = FALSE)
    first_trip <- DBI::dbGetQuery (db, qry_min) [1, 1]
    last_trip <- DBI::dbGetQuery (db, qry_max) [1, 1]
    DBI::dbDisconnect(db)

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
#' \dontrun{
#' data_dir <- tempdir ()
#' bike_write_test_data (data_dir = data_dir)
#' # dl_bikedata (city = 'la', data_dir = data_dir) # or download some real data!
#' # Remove one London file that triggers an API call which may fail tests:
#' file.remove (file.path (tempdir(), "01aJourneyDataExtract10Jan16-23Jan16.csv"))
#' bikedb <- file.path (data_dir, 'testdb')
#' store_bikedata (data_dir = data_dir, bikedb = bikedb)
#' # create database indexes for quicker access:
#' index_bikedata_db (bikedb = bikedb)
#'
#' bike_summary_stats ('testdb')
#' 
#' bike_rm_test_data (data_dir = data_dir)
#' bike_rm_db (bikedb)
#' # don't forget to remove real data!
#' # file.remove (list.files ('.', pattern = '.zip'))
#' }
bike_summary_stats <- function (bikedb)
{
    if (missing (bikedb))
        stop ("Can't get summary statistics if bikedb isn't provided")

    bikedb <- check_db_arg (bikedb)

    cities <- bike_cities_in_db (bikedb)

    num_trips <- bike_db_totals (bikedb, TRUE)
    num_stations <- bike_db_totals (bikedb, FALSE)
    dates <- rbind (c (NULL, NULL), bike_datelimits (bikedb)) # so [,1] works

    latest_files <- bike_latest_files (bikedb)
    latest <- NULL # latest_files aren't necessarily in db order
    for (ci in cities)
    {
        num_trips <- c (num_trips, bike_db_totals (bikedb, TRUE, city = ci))
        num_stations <- c (num_stations, bike_db_totals (bikedb, FALSE,
                                                         city = ci))
        dates <- rbind (dates, bike_datelimits (bikedb, city = ci))
        latest <- c (latest,
                     latest_files [which (names (latest_files) == ci)])
    }
    latest_files <- c (NA, as.logical (latest))

    res <- data.frame (city = as.character (c ('total', cities)),
                       num_trips = num_trips, num_stations = num_stations,
                       first_trip = dates [, 1], last_trip = dates [, 2],
                       latest_files = latest_files,
                       stringsAsFactors = FALSE)
    return (tibble::as_tibble (res))
}

#' Extract daily trip counts for all stations
#'
#' @param bikedb A string containing the path to the SQLite3 database.
#' If no directory specified, it is presumed to be in \code{tempdir()}.
#' @param city City for which trips are to be counted - mandatory if database
#' contains data for more than one city
#' @param station Optional argument specifying bike station for which trips are
#' to be counted
#' @param member If given, extract only trips by registered members
#' (\code{member = 1} or \code{TRUE}) or not (\code{member = 0} or \code{FALSE}).
#' @param birth_year If given, extract only trips by registered members whose
#' declared birth years equal or lie within the specified value or values.
#' @param gender If given, extract only records for trips by registered
#' users declaring the specified genders (\code{f/m/.} or \code{2/1/0}).
#' @param standardise If TRUE, daily trip counts are standardised to the
#' relative numbers of bike stations in operation for each day, so daily trip
#' counts are increased during (generally early) periods with relatively fewer
#' stations, and decreased during (generally later) periods with more stations.
#'
#' @return A \code{data.frame} containing daily dates and total numbers of trips
#'
#' @export
#'
#' @examples
#' \dontrun{
#' bike_write_test_data () # by default in tempdir ()
#' # dl_bikedata (city = 'la', data_dir = data_dir) # or download some real data!
#' store_bikedata (data_dir = tempdir (), bikedb = 'testdb')
#' # create database indexes for quicker access:
#' index_bikedata_db (bikedb = 'testdb')
#'
#' bike_daily_trips (bikedb = 'testdb', city = 'ny')
#' bike_daily_trips (bikedb = 'testdb', city = 'ny', member = TRUE)
#' bike_daily_trips (bikedb = 'testdb', city = 'ny', gender = 'f')
#' bike_daily_trips (bikedb = 'testdb', city = 'ny', station = '173',
#'                   gender = 1)
#' 
#' bike_rm_test_data ()
#' bike_rm_db ('testdb')
#' # don't forget to remove real data!
#' # file.remove (list.files ('.', pattern = '.zip'))
#' }
bike_daily_trips <- function (bikedb, city, station, member, birth_year, gender,
                              standardise = FALSE)
{
    if (missing (bikedb))
        stop ("Can't get daily trips if bikedb isn't provided")

    bikedb <- check_db_arg (bikedb)
    city <- check_city_arg (bikedb, city)

    db <- DBI::dbConnect (RSQLite::SQLite(), bikedb, create = FALSE)
    qry <- paste0 ("SELECT STRFTIME('%Y-%m-%d', start_time) AS 'date', ",
                   "COUNT() AS 'numtrips' FROM trips ")

    qry_where <- "city = ?"
    qryargs <- city
    if (!missing (member))
    {
        qry_where <- c (qry_where, "user_type = ?")
        qryargs <- c (qryargs, bike_transform_member (member))
    }
    if (!missing (birth_year))
    {
        qtmp <- add_birth_year_to_qry (qry_where, qryargs, birth_year)
        qry_where <- qtmp$qry
        qryargs <- qtmp$qryargs
    }
    if (!missing (gender))
    {
        qry_where <- c (qry_where, "gender = ?")
        qryargs <- c (qryargs, bike_transform_gender (gender))
    }

    if (!missing (station))
    {
        if (substring (station, 1, 2) != city)
            station <- paste0 (city, station)
        # Then just check that station is in stations table
        stns <- DBI::dbGetQuery (db, "SELECT stn_id FROM stations")$stn_id
        if (!station %in% stns)
            stop ('Station ', station, ' does not exist in database')
        qry_where <- c (qry_where, "start_station_id = ?")
        qryargs <- c (qryargs, station)
    }
    qry <- paste (qry, "WHERE", paste (qry_where, collapse = " AND "))
    qry <- paste0 (qry, " GROUP BY STRFTIME('%Y-%m-%d', date);")

    qryres <- DBI::dbSendQuery (db, qry)
    DBI::dbBind(qryres, as.list (qryargs))
    trips <- DBI::dbFetch (qryres)
    DBI::dbClearResult (qryres)
    DBI::dbDisconnect (db)

    trips$date <- as.Date (trips$date)

    if (standardise)
    {
        dates <- bike_station_dates (bikedb = bikedb, city = city)
        all_days <- seq (min (dates$first), max (dates$last), by = 'days')
        daily_stns <- rep (0, length (all_days))
        for (i in seq (daily_stns))
            daily_stns [i] <- length (which (dates$first <= all_days [i]))
        daily_stns <- 1 / daily_stns # convert to multiplicative factor

        # Entire systems can also have gaps, so dates have to be matched
        if (!all (trips$date %in% all_days))
            stop ('sequence of dates generated dates not present in db')
        daily_stns <- daily_stns [which (trips$date %in% all_days)]
        daily_stns <- daily_stns / mean (daily_stns)
        trips$numtrips <- trips$numtrips * daily_stns

        # Then round to 3 places
        trips$numtrips <- round (trips$numtrips * daily_stns, digits = 3)
    }

    return (tibble::as_tibble (trips))
}



#' Static summary of which systems provide demographic data
#'
#' @return A \code{data.frame} detailing the kinds of demographic data provided
#' by the different systems
#'
#' @export
#'
#' @examples
#' bike_demographic_data ()
#' # Examples of filtering data by demographic parameters:
#' \dontrun{
#' data_dir <- tempdir ()
#' bike_write_test_data (data_dir = data_dir)
#' bikedb <- file.path (data_dir, 'testdb')
#' store_bikedata (data_dir = data_dir, bikedb = bikedb)
#' # create database indexes for quicker access:
#' index_bikedata_db (bikedb = bikedb)
#'
#' sum (bike_tripmat (bikedb = bikedb, city = 'bo')) # 200 trips
#' sum (bike_tripmat (bikedb = bikedb, city = 'bo', birth_year = 1990)) # 9
#' sum (bike_tripmat (bikedb = bikedb, city = 'bo', gender = 'f')) # 22
#' sum (bike_tripmat (bikedb = bikedb, city = 'bo', gender = 2)) # 22
#' sum (bike_tripmat (bikedb = bikedb, city = 'bo', gender = 1)) # = m; 68
#' sum (bike_tripmat (bikedb = bikedb, city = 'bo', gender = 0)) # = n; 9
#' # Sum of gender-filtered trips is less than total because \code{gender = 0}
#' # extracts all registered users with unspecified genders, while without gender
#' # filtering extracts all trips for registered and non-registered users.
#' 
#' # The following generates an error because Washinton DC's DivvyBike system does
#' # not provide demographic data
#' sum (bike_tripmat (bikedb = bikedb, city = 'dc', birth_year = 1990))
#' bike_rm_test_data (data_dir = data_dir)
#' bike_rm_db (bikedb)
#' }
bike_demographic_data <- function ()
{
    dat <- data.frame (city = c ('bo', 'ch', 'dc', 'gu', 'la', 'lo',
                                 'mo', 'mn', 'ny', 'ph', 'sf'),
                       city_name = c ('Boston', 'Chicago', 'Washington DC',
                                      'Guadalajara', 'Los Angeles', 'London',
                                      'Montreal', 'Minneapolis', 'New York',
                                      'Philadelphia', 'Bay Area'),
                       bike_system = c ('Hubway', 'Divvy', 'CapitalBikeShare',
                                        'mibici', 'Metro', 'Santander', 'Bixi',
                                        'NiceRide', 'Citibike', 'Indego',
                                        'FordGoBike'),
                       demographic_data = c (TRUE, TRUE, FALSE, TRUE, FALSE,
                                             FALSE, FALSE, TRUE, TRUE, FALSE,
                                             TRUE),
                       stringsAsFactors = FALSE)

    return (dat)
}
