#' Store hire bicycle data in spatialite database
#'
#' @param data_dir A character vector giving the directory containing the
#'          data files downloaded with \code{dl_bikedata} for one or more
#'          cities.
#' @param spdb A string containing the path to the spatialite database to 
#'          use. If it doesn't already exist, it will be created, otherwise data
#'          will be appended to existing database.
#' @param quiet If FALSE, progress is displayed on screen
#' @param create_index If TRUE, creates an index on the start and end station
#'          IDs and start and stop times.
#'
#' @note Data for different cities are all stored in the same database, with
#' city identifiers automatically established from the names of downloaded data
#' files. This function can take quite a long time to execute (typically > 10
#' minutes), and generates a spatialite database file several gigabytes in size.
#' 
#' @export
store_bikedata <- function (data_dir, spdb, quiet=FALSE, create_index = TRUE)
{
    er_idx <- file.exists (spdb) + 1 # = (1, 2) if (!exists, exists)
    if (!quiet)
        message (c ('Creating', 'Adding data to') [er_idx], ' sqlite3 database')
    if (!file.exists (spdb))
        chk <- create_sqlite3_db (spdb)

    # platform-independent unzipping is much easier in R
    flist <- file.path (data_dir, list.files (data_dir, pattern=".zip"))
    if (!quiet)
        message ('Extracting files ', appendLF=FALSE)
    for (f in flist)
    {
        fcsv <- unzip (f, list=TRUE)$Name
        if (!exists (fcsv))
            unzip (f, exdir=data_dir)
        if (!quiet)
            message ('.', appendLF=FALSE)
    }
    if (!quiet)
        message (' done')
    flist_csv <- file.path (data_dir, list.files (data_dir, pattern=".csv"))

    cities <- get_bike_cities (data_dir)

    ntrips <- 0
    for (city in cities)
    {
        flist_city <- get_flist_city (flist_csv, city)
        ntrips <- ntrips + importDataToSqlite3 (flist_city, spdb, 
                                                substring (city, 1, 2), quiet)
    }
    if (!quiet)
    {
        message ('Total trips ', c ('read', 'added') [er_idx], ' = ',
                 format (ntrips, big.mark=',', scientific=FALSE))
        if (er_idx == 2)
            message ("database '", basename (spdb), "' now has ", 
                     format (num_trips_in_db (spdb), big.mark=',',
                             scientific=FALSE), ' trips')
    }

    create_city_index (spdb, er_idx - 1)

    if (create_index == TRUE) # additional indexes for stations and times
    {
        if (!quiet) 
            message (c ('Creating', 'Re-creating') [er_idx], ' indexes')
        create_db_indexes (spdb, 
                           tables = rep("trips", times=4),
                           cols = c("start_station_id", "end_station_id", 
                                    "start_time", "stop_time"),
                           indexes_exist (spdb))
    }

    invisible (file.remove (flist_csv))
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
    if (any (grepl ('trips-history', flist, ignore.case=TRUE)) |
        any (grepl ('quarter', flist, ignore.case=TRUE)))
        cities$dc <- TRUE
    if (any (grepl ('metro', flist, ignore.case=TRUE)))
        cities$la <- TRUE

    cities <- which (unlist (cities))
    names (cities)
}

#' Get list of unzipped .csv files for a particular city
#'
#' @param flist List of all unzipped (\code{.csv}) data files for one or more
#'              cities.
#' @param city One of (nyc, boston, chicago, dc, la)
#'
#' @return Only those members of flist corresponding to nominated city
#'
#' @noRd
get_flist_city <- function (flist, city)
{
    index <- NULL
    if (city == 'nyc')
        index <- which (grepl ('citibike', flist, ignore.case=TRUE))
    else if (city == 'chicago')
        index <- which (grepl ('divvy', flist, ignore.case=TRUE))
    else if (city == 'boston')
        index <- which (grepl ('hubway', flist, ignore.case=TRUE))
    else if (city == 'dc')
        index <- which (grepl ('trips-history', flist, ignore.case=TRUE) |
                        grepl ('quarter', flist, ignore.case=TRUE))
    else if (city == 'la')
        index <- which (grepl ('metro', flist, ignore.case=TRUE))

    flist [index]
}

#' Check whether indexes have been created for database
#'
#' @param spdb A string containing the path to the spatialite database to 
#' use. 
#'
#' @noRd
indexes_exist <- function (spdb)
{
    db <- RSQLite::dbConnect(SQLite(), spdb, create = FALSE)
    idx_list <- dbGetQuery(db, "PRAGMA index_list (trips)")
    RSQLite::dbDisconnect(db)
    nrow (idx_list) > 2 # 2 because city index is automatically created
}

#' Count number of trips in sqlite3 database
#'
#' @param spdb A string containing the path to the spatialite database to 
#' use. 
#'
#' @export
num_trips_in_db <- function (spdb)
{
    db <- RSQLite::dbConnect(SQLite(), spdb, create = FALSE)
    ntrips <- dbGetQuery(db, "SELECT Count(*) FROM trips")
    RSQLite::dbDisconnect(db)
    return (as.numeric (ntrips))
}
