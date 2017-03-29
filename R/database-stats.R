# miscellaneous functions for retrieving summary stats from database

#' Check whether indexes have been created for database
#'
#' @param bikedb A string containing the path to the SQLite3 database to 
#' use. 
#'
#' @noRd
indexes_exist <- function (bikedb)
{
    db <- RSQLite::dbConnect(SQLite(), bikedb, create = FALSE)
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
num_trips_in_db <- function (bikedb)
{
    db <- RSQLite::dbConnect(SQLite(), bikedb, create = FALSE)
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
    db <- RSQLite::dbConnect(SQLite(), bikedb, create = FALSE)
    ntrips <- dbGetQuery(db, "SELECT Count(*) FROM datafiles")
    RSQLite::dbDisconnect(db)
    return (as.numeric (ntrips))
}

#' Return names of files in nominated directory that are **not** already in
#' database
#'
#' @param bikedb A string containing the path to the SQLite3 database to 
#'          use. 
#' @param data_dir A character vector giving the directory containing the
#'          data files downloaded with \code{dl_bikedata} for one or more
#'          cities.
#'
#' @return Vector with names of datafiles to be added to database
#'
#' @noRd
get_new_datafiles <- function (bikedb, data_dir)
{
    db <- dplyr::src_sqlite (bikedb, create=F)
    old_files <- dplyr::collect (dplyr::tbl (db, 'datafiles'))$name
    files <- list.files (data_dir, pattern = '.zip')
    files [which (!files %in% old_files)]
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
get_datelimits <- function (bikedb, city)
{
    qry_min <- "SELECT MIN(start_time) FROM trips"
    qry_max <- "SELECT MAX(start_time) FROM trips"
    if (!missing (city))
    {
        qry_min <- paste0 (qry_min, " WHERE city = '", 
                          substring (tolower (city), 1, 2), "'")
        qry_max <- paste0 (qry_max, " WHERE city = '", 
                          substring (tolower (city), 1, 2), "'")
    }

    db <- RSQLite::dbConnect(SQLite(), bikedb, create = FALSE)
    first_trip <- RSQLite::dbGetQuery (db, qry_min) [1, 1]
    last_trip <- RSQLite::dbGetQuery (db, qry_max) [1, 1]
    RSQLite::dbDisconnect(db)

    res <- c (first_trip, last_trip)
    names (res) <- c ('first', 'last')
    return (res)
}
