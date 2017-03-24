#' Store nyc-citibike data in spatialite database
#'
#' 
#' @param data_dir A character vector giving the directory containing the
#' \code{.zip} files of citibike data.
#' @param spdb A string containing the path to the spatialite database to 
#' use. It will be created automatically if it doesn't already exist.
#' @param quiet If FALSE, progress is displayed on screen
#' @param create_index If TRUE, creates an index on the start and end station
#' IDs and start and stop times.
#'
#' @note This function can take quite a long time to execute (typically > 10
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
    ntrips <- importDataToSqlite3 (flist_csv, spdb, quiet)
    if (!quiet)
    {
        message ('Total trips ', c ('read', 'added') [er_idx], ' = ',
                 format (ntrips, big.mark=',', scientific=FALSE))
        if (er_idx == 2)
            message ("database '", basename (spdb), "' now has ", 
                     format (num_trips_in_db (spdb), big.mark=',',
                             scientific=FALSE), ' trips')
    }
    if (create_index == TRUE) 
    {
        if (!quiet) 
            message (c ('Creating', 'Re-creating') [er_idx], ' indexes')
        create_db_indexes(spdb, 
                          tables = rep("trips", times=4),
                          cols = c("start_station_id", "end_station_id", 
                                   "start_time", "stop_time"),
                          indexes_exist (spdb))
    }
    invisible (file.remove (flist_csv))
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
    nrow (idx_list) > 1
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
