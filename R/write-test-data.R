#' Writes test data bundled with package to zip files
#'
#' Writes very small test files to disk that can be used to test the package.
#' The entire package works by reading zip-compressed data files provided by the
#' various hire bicycle systems. This function generates some equivalent data
#' that can be read into an \code{SQLite} database by the
#' \code{store_bikedata()} function, so that all other package functionality can
#' then be tested from the resultant database. This function is also used in the
#' examples of all other functions.
#'
#' @param data_dir Directory in which data are to be extracted. Defaults to
#' \code{tempdir()}. If any other directory is specified, files ought to be
#' removed with \code{bike_rm_test_data()}.
#'
#' @export
#'
#' @examples
#' bike_write_test_data ()
#' list.files (tempdir ())
#' bike_rm_test_data ()
#'
#' bike_write_test_data (data_dir = '.')
#' list.files ()
#' bike_rm_test_data (data_dir = '.')
bike_write_test_data <- function (data_dir = tempdir ())
{
    # http://tolstoy.newcastle.edu.au/R/e17/devel/12/04/0876.html
    # This works but brings bike_test_data into the global env; 2nd option
    # doesn't
    #bike_test_data <- get ("bike_test_data", envir = .GlobalEnv)
    env <- new.env ()
    data ('bike_test_data', envir = env)

    # chicago has to be done separately
    fdir <- file.path (data_dir, 'Divvy_Trips_sample')
    dir.create (fdir)
    fst <- file.path (data_dir, 'Divvy_Trips_sample/Divvy_Stations.csv')
    ftr <- file.path (data_dir, 'Divvy_Trips_sample/Divvy_Trips_sample.csv')
    write.csv (env$bike_test_data$ch_st, file = fst,
               quote = FALSE, row.names = FALSE, na = '')
    write.csv (env$bike_test_data$ch_tr, file = ftr,
               quote = TRUE, row.names = FALSE, na = '')
    zip (file.path (data_dir, 'sample-divvy-trips.zip'), fdir)
    invisible (tryCatch (file.remove (fst, ftr, fdir),
                         warning = function (w) NULL,
                         error = function (e) NULL))
    # Then the remaining files
    cities <- c ('bo', 'dc', 'la', 'lo', 'ny')
    zips <- c ('sample-hubway-trip-data.zip',
               'sample-cabi-dc-trips-history-data.zip',
               'sample-la-metro.zip',
               'sample-JourneyDataExtract-london.csv.zip',
               'sample-citibike-tripdata.zip')
    csvs <- c ('201604-hubway-tripdata.csv',
               '2017-Q1-Trips-History-Data.csv',
               'la_metro_gbfs_trips_Q1_2017.csv',
               '01aJourneyDataExtract10Jan16-23Jan16.csv',
               '201612-citibike-tripdata.csv')
    quotes <- c (TRUE, FALSE, FALSE, FALSE, FALSE)

    zips <- file.path (data_dir, zips)
    csvs <- file.path (data_dir, csvs)

    for (i in seq (csvs))
    {
        dati <- env$bike_test_data [[cities [i]]]
        write.csv (dati, file = csvs [i], quote = quotes [i], na = '',
                   row.names = FALSE)
        # Note: Output of zip can't be suppressed because it's a 'system2()'
        # command
        zip (zips [i], csvs [i])
    }
    invisible (tryCatch (file.remove (csvs),
                         warning = function (w) NULL,
                         error = function (e) NULL))
}

#' Removes test data written with 'bike_write_test_data()'
#'
#' The function \code{bike_write_test_data()} writes several small
#' zip-compressed files to disk. The default location is \code{tempdir()}, in
#' which case these files will be automatically removed on termination of
#' current R session. If, however, any other value for \code{data_dir} is passed
#' to \code{bike_write_test_data()}, then the resultant files ought be deleted
#' by calling this function.
#'
#' @param data_dir Directory in which data were extracted.
#'
#' @return Number of files successfully removed, which should equal six.
#'
#' @export
#'
#' @examples
#' bike_write_test_data ()
#' list.files (tempdir ())
#' bike_rm_test_data ()
#'
#' bike_write_test_data (data_dir = getwd ())
#' list.files ()
#' bike_rm_test_data (data_dir = getwd ())
bike_rm_test_data <- function (data_dir = tempdir ())
{
    zips <- c ('sample-hubway-trip-data.zip',
               'sample-divvy-trips.zip',
               'sample-cabi-dc-trips-history-data.zip',
               'sample-la-metro.zip',
               'sample-JourneyDataExtract-london.csv.zip',
               'sample-citibike-tripdata.zip')
    zips <- file.path (data_dir, zips)
    res <- NULL
    rm1 <- function (f) {
        tryCatch (file.remove (f),
                  warning = function (w) NULL,
                  error = function (e) NULL)
    }
    for (z in zips)
        if (file.exists (z))
            res <- c (res, rm1 (z))

    return (length (res))
}
