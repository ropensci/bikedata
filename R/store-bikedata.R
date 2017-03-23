#' Store nyc-citibike data in spatialite database
#'
#' 
#' @param data_dir A character vector giving the directory containing the
#' \code{.zip} files of citibike data.
#' @param spdb A string containing the path to the spatialite database to 
#' use. It will be created automatically.
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
    if (file.exists (spdb))
        stop ('File named ', spdb, ' already exists')

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
        message ('total trips read = ', 
                 format (ntrips, big.mark=',', scientific=FALSE))
    if (create_index == TRUE) {
      if (!quiet) {
        message ('Creating indexes')
      }
      create_db_indexes(spdb, 
                      tables = rep("trips", times=4),
                      cols = c("start_station_id", "end_station_id", "start_time", "stop_time"))
    }
    invisible (file.remove (flist_csv))
}
