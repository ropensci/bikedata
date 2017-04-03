# miscellaneous functions for retrieving summary stats from database

#' Check whether indexes have been created for database
#'
#' @param bikedb A string containing the path to the SQLite3 database to 
#' use. 
#'
#' @noRd
indexes_exist <- function (bikedb)
{
    db <- RSQLite::dbConnect (RSQLite::SQLite(), bikedb, create = FALSE)
    idx_list <- dbGetQuery(db, "PRAGMA index_list (trips)")
    RSQLite::dbDisconnect(db)
    nrow (idx_list) > 2 # 2 because city index is automatically created
}

#' Count number of trips in sqlite3 database
#'
#' @param bikedb A string containing the path to the SQLite3 database to 
#' use. 
#'
#' @export
bike_total_trips <- function (bikedb)
{
    db <- RSQLite::dbConnect (RSQLite::SQLite(), bikedb, create = FALSE)
    ntrips <- dbGetQuery(db, "SELECT Count(*) FROM trips")
    RSQLite::dbDisconnect(db)
    return (as.numeric (ntrips))
}

#' Count number of datafiles in sqlite3 database
#'
#' @param bikedb A string containing the path to the SQLite3 database to 
#' use. 
#'
#' @noRd
num_datafiles_in_db <- function (bikedb)
{
    db <- RSQLite::dbConnect (RSQLite::SQLite(), bikedb, create = FALSE)
    ntrips <- dbGetQuery(db, "SELECT Count(*) FROM datafiles")
    RSQLite::dbDisconnect(db)
    return (as.numeric (ntrips))
}

#' List the cities with data containined in SQLite3 database
#'
#' @param bikedb A string containing the path to the SQLite3 database to 
#' use. 
#'
#' @noRd
bike_cities_in_db <- function (bikedb)
{
    db <- RSQLite::dbConnect (RSQLite::SQLite(), bikedb, create = FALSE)
    cities <- dbGetQuery(db, "SELECT city FROM stations")
    RSQLite::dbDisconnect(db)
    cities <- unique (cities)
    rownames (table (cities)) # TODO: Find a better way to do that
}

#' Return names of files in nominated directory that are **not** already in
#' database
#'
#' @param bikedb A string containing the path to the SQLite3 database to 
#'          use. 
#' @param flist_zip A character vector listing the names of \code{.zip} files
#'          for a particular city as returned from \code{get_flist_city}
#'
#' @return Vector with members of \code{flist_zip} that have not already been
#'          read into the database
#'
#' @noRd
get_new_datafiles <- function (bikedb, flist_zip)
{
    db <- dplyr::src_sqlite (bikedb, create=F)
    old_files <- dplyr::collect (dplyr::tbl (db, 'datafiles'))$name
    flist_zip [which (!basename (flist_zip) %in% old_files)]
}

#' Extract date-time limits from trip database
#'
#' @param bikedb Path to the SQLite3 database 
#' @param city If given, date limits are calculated only for trips in 
#'          that city.
#'
#' @return A vector of 2 elements giving the date-time of the first and last
#' trips
#'
#' @export
bike_datelimits <- function (bikedb, city)
{
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
