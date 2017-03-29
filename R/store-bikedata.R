#' Store hire bicycle data in SQLite3 database
#'
#' @param data_dir A character vector giving the directory containing the
#'          data files downloaded with \code{dl_bikedata} for one or more
#'          cities.
#' @param bikedb A string containing the path to the SQLite3 database to 
#'          use. If it doesn't already exist, it will be created, otherwise data
#'          will be appended to existing database.
#' @param station_files Character vector giving location 
#' @param quiet If FALSE, progress is displayed on screen
#' @param create_index If TRUE, creates an index on the start and end station
#'          IDs and start and stop times.
#'
#' @return Number of trips added to database
#'
#' @note Data for different cities are all stored in the same database, with
#' city identifiers automatically established from the names of downloaded data
#' files. This function can take quite a long time to execute (typically > 10
#' minutes), and generates a SQLite3 database file several gigabytes in size.
#' 
#' @export
store_bikedata <- function (data_dir, bikedb, station_files,
                            quiet=FALSE, create_index = TRUE)
{
    er_idx <- file.exists (bikedb) + 1 # = (1, 2) if (!exists, exists)
    if (!quiet)
        message (c ('Creating', 'Adding data to') [er_idx], ' sqlite3 database')
    if (!file.exists (bikedb))
        chk <- rcpp_create_sqlite3_db (bikedb)

    cities <- get_bike_cities (data_dir)

    ntrips <- 0
    for (city in cities)
    {
        flist_zip <- get_flist_city (data_dir, city)
        flist_zip <- get_new_datafiles (bikedb, data_dir)
        csv_files <- list.files (data_dir, pattern = '.csv')
        if (length (flist_zip) > 0)
        {
            flist_zip <- paste0 (data_dir, '/', flist_zip)
            flist_csv <- NULL
            for (f in flist_zip)
            {
                fi <- unzip (f, list = TRUE)$Name
                if (!fi %in% csv_files)
                {
                    flist_csv <- c (flist_csv, fi)
                    unzip (f, exdir = data_dir)
                }
            }
            flist_csv <- paste0 (data_dir, '/', flist_csv)
            nf <- num_datafiles_in_db (bikedb)
            nf <- rcpp_import_to_datafile_table (bikedb, basename (flist_zip),
                                                 substring (city, 1, 2), nf)
            ntrips <- ntrips + rcpp_import_to_trip_table (bikedb, flist_csv,
                                                    substring (city, 1, 2), quiet)
            invisible (file.remove (flist_csv))
        }
    }
    if (!quiet)
        if (ntrips > 0)
        {
            message ('Total trips ', c ('read', 'added') [er_idx], ' = ',
                     format (ntrips, big.mark=',', scientific=FALSE))
            if (er_idx == 2)
                message ("database '", basename (bikedb), "' now has ", 
                         format (num_trips_in_db (bikedb), big.mark=',',
                                 scientific=FALSE), ' trips')
        } else
            message ('All data already in database; no new data added')

    rcpp_create_city_index (bikedb, er_idx - 1)

    if (ntrips > 0 & create_index) # additional indexes for stations and times
    {
        if (!quiet) 
            message (c ('Creating', 'Re-creating') [er_idx], ' indexes')
        rcpp_create_db_indexes (bikedb, 
                           tables = rep("trips", times=4),
                           cols = c("start_station_id", "end_station_id", 
                                    "start_time", "stop_time"),
                           indexes_exist (bikedb))
    }
    
    return (ntrips)
}

#' Get list of cities from files in specified data directory
#'
#' @param data_dir A character vector giving the directory containing the
#'          \code{.zip} files of citibike data.
#'
#' @noRd
get_bike_cities <- function (data_dir)
{
    flist <- list.files (data_dir, pattern=".zip")
    cities <- list ('nyc' = FALSE, 
                    'boston' = FALSE, 
                    'chicago' = FALSE, 
                    'dc' = FALSE, 
                    'la' = FALSE)

    if (any (grepl ('citibike', flist, ignore.case=TRUE)))
        cities$nyc <- TRUE
    if (any (grepl ('divvy', flist, ignore.case=TRUE)))
        cities$chicago <- TRUE
    if (any (grepl ('hubway', flist, ignore.case=TRUE)))
        cities$boston <- TRUE
    if (any (grepl ('cabi', flist, ignore.case=TRUE)))
        cities$dc <- TRUE
    if (any (grepl ('metro', flist, ignore.case=TRUE)))
        cities$la <- TRUE

    cities <- which (unlist (cities))
    names (cities)
}

#' Get list of data files for a particular city in specified directory 
#'
#' @param data_dir Directory containing data files
#' @param city One of (nyc, boston, chicago, dc, la)
#'
#' @return Only those members of flist corresponding to nominated city
#'
#' @noRd
get_flist_city <- function (data_dir, city)
{
    city <- substring (tolower (city), 1, 2)

    flist <- list.files (data_dir, pattern='.zip')

    index <- NULL
    if (city %in% c ('ny', 'ne'))
        index <- which (grepl ('citibike', flist, ignore.case=TRUE))
    else if (city == 'ch')
        index <- which (grepl ('divvy', flist, ignore.case=TRUE))
    else if (city == 'bo')
        index <- which (grepl ('hubway', flist, ignore.case=TRUE))
    else if (city %in% c ('dc', 'wa'))
        index <- which (grepl ('cabi', flist, ignore.case=TRUE))
    else if (city %in% c ('la', 'lo'))
        index <- which (grepl ('metro', flist, ignore.case=TRUE))

    paste0 (data_dir, '/', flist [index])
}

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
