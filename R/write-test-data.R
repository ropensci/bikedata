#' Writes test data bundled with package to zip files
#'
#' @param data_dir Directory in which data are to be extracted. Defaults to
#' \code{tempdir()}. If any other directory is specified, files ought to be
#' removed with \code{bike_rm_test_data()}.
#'
#' @note The entire package works by reading zip-compressed data files provided
#' by the various hire bicycle systems. Sample data are included with the
#' package, but these need to be written to zip-compressed archives in order for
#' the package to be able to use them. This function enables the package to be
#' tested, both for general use, and for the \code{testhtat} routines.
#'
#' @export
#'
#' @examples
#' \dontrun{
#' bike_write_test_data ()
#' list.files (tempdir ())
#' bike_rm_test_data ()
#' }
bike_write_test_data <- function (data_dir = tempdir ())
{
    #data ("bike_dat", package = 'bikedata', envir = parent.env (environment ()))
    #data (bike_dat, envir = parent.env (environment ()))
    bike_dat <- bike_dat 
    #bike_dat <- get (bike_dat, envir = parent.env (environment ()))
    # chicago has to be done separately
    fdir <- file.path (data_dir, 'Divvy_Trips_sample')
    dir.create (fdir)
    fst <- file.path (data_dir, "Divvy_Trips_sample/Divvy_Stations.csv")
    ftr <- file.path (data_dir, "Divvy_Trips_sample/Divvy_Trips_sample.csv")
    write.csv (bike_dat$ch_st, file = fst,
               quote = FALSE, row.names = FALSE, na = '')
    write.csv (bike_dat$ch_tr, file = ftr,
               quote = TRUE, row.names = FALSE, na = '')
    zip (file.path (data_dir, 'sample-divvy-trips.zip'), fdir)
    invisible (file.remove (fst, ftr, fdir))
    # Then the remaining files
    cities <- c ('bo', 'dc', 'la', 'lo', 'ny')
    zips <- c ("sample-hubway-trip-data.zip",
               "sample-cabi-dc-trips-history-data.zip",
               "sample-la-metro.zip",
               "sample-JourneyDataExtract-london.csv.zip",
               "sample-citibike-tripdata.zip")
    csvs <- c ("201604-hubway-tripdata.csv",
               "2017-Q1-Trips-History-Data.csv",
               "la_metro_gbfs_trips_Q1_2017.csv",
               "01aJourneyDataExtract10Jan16-23Jan16.csv",
               "201612-citibike-tripdata.csv")
    quotes <- c (TRUE, FALSE, FALSE, FALSE, FALSE)

    zips <- file.path (data_dir, zips)
    csvs <- file.path (data_dir, csvs)

    for (i in seq (csvs))
    {
        dati <- bike_dat [[cities [i]]]
        write.csv (dati, file = csvs [i], quote = quotes [i], na = '',
                   row.names = FALSE)
        # Note: Output of zip can't be suppressed because it's a 'system2()'
        # command
        zip (zips [i], csvs [i])
    }
    invisible (file.remove (csvs))
}

#' Removes test data written with 'bike_write_test_data()'
#'
#' @param data_dir Directory in which data were extracted.
#'
#' @export
#'
#' @examples
#' \dontrun{
#' data (bike_dat)
#' bike_write_test_data ()
#' list.files (tempdir ())
#' bike_rm_test_data ()
#' }
bike_rm_test_data <- function (data_dir = tempdir ())
{
    zips <- c ("sample-hubway-trip-data.zip",
               "sample-cabi-dc-trips-history-data.zip",
               "sample-la-metro.zip",
               "sample-JourneyDataExtract-london.csv.zip",
               "sample-citibike-tripdata.zip")
    zips <- file.path (data_dir, zips)
    for (z in zips)
        if (file.exists (z))
            invisible (file.remove (z))
}
