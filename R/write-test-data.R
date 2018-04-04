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
#' \dontrun{
#' bike_write_test_data (data_dir = '.')
#' list.files ()
#' bike_rm_test_data (data_dir = '.')
#' }
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
    fst <- file.path (data_dir, 'Divvy_Trips_sample', 'Divvy_Stations.csv')
    ftr <- file.path (data_dir, 'Divvy_Trips_sample', 'Divvy_Trips_sample.csv')
    write.csv (env$bike_test_data$ch_st, file = fst,
               quote = FALSE, row.names = FALSE, na = '')
    # quotes need manual tweaking here:
    ch_tr <- env$bike_test_data$ch_tr
    cols <- c ("trip_id", "bikeid", "tripduration", "from_station_id",
               "to_station_id", "birthyear", "gender")
    for (i in cols)
        ch_tr [[i]] <- as.character (ch_tr [[i]])
    ch_tr$birthyear [which (is.na (ch_tr$birthyear))] <- ""
     
    write.csv (ch_tr, file = ftr, quote = TRUE, row.names = FALSE, na = '')
    zip (file.path (data_dir, 'sample-divvy-trips.zip'), fdir)
    invisible (tryCatch (file.remove (fst, ftr, fdir),
                         warning = function (w) NULL,
                         error = function (e) NULL))
    # so does boston
    f12 <- file.path (data_dir, "hubway_Trips_2012.csv")
    write.csv (env$bike_test_data$bo12, row.names = FALSE, file = f12,
               quote = FALSE, na = '') # not zipped
    f17 <- file.path (data_dir, "201701-hubway-tripdata.csv")
    write.csv (env$bike_test_data$bo17, row.names = FALSE, file = f17,
               quote = FALSE, na = '')
    zip (file.path (data_dir, "201701_hubway_tripdata.zip"), f17)
    f18 <- file.path (data_dir, "201801_hubway_tripdata.csv")
    write.csv (env$bike_test_data$bo18, row.names = FALSE, file = f18,
               quote = TRUE, na = '')
    zip (file.path (data_dir, "201801_hubway_tripdata.csv.zip"), f18)
    st1 <- file.path (data_dir, "Hubway_Stations_2011_2016.csv")
    write.csv (env$bike_test_data$bo_st1, row.names = FALSE, file = st1,
               quote = FALSE, na = '')
    st2 <- file.path (data_dir, "Hubway_Stations_as_of_July_2017.csv")
    write.csv (env$bike_test_data$bo_st2, row.names = FALSE, file = st2,
               quote = FALSE, na = '')
    invisible (tryCatch (file.remove (f17, f18),
                         warning = function (w) NULL,
                         error = function (e) NULL))

    # and MN
    fdir <- file.path (data_dir, 'Nice_Ride_data_2012_season')
    dir.create (fdir)
    fst <- file.path (data_dir, "Nice_Ride_data_2012_season",
                      "Nice_Ride_2012-station_locations .csv") # original typo
    ftr <- file.path (data_dir, "Nice_Ride_data_2012_season",
                      "Nice_Ride_trip_history_2012_season.csv")
    write.csv (env$bike_test_data$mn_st, file = fst,
               quote = FALSE, row.names = FALSE, na = '')
    write.csv (env$bike_test_data$mn_tr, file = ftr,
               quote = FALSE, row.names = FALSE, na = '')
    zip (file.path (data_dir, 'Nice_Ride_data_2012_season.zip'), fdir)
    invisible (tryCatch (file.remove (fst, ftr, fdir),
                         warning = function (w) NULL,
                         error = function (e) NULL))

    # London is just a plain csv
    csv <- file.path (data_dir, '01aJourneyDataExtract10Jan16-23Jan16.csv')
    lo <- env$bike_test_data$lo
    lo$EndStation.Name <- paste0 ("\"", lo$EndStation.Name, "\"")
    lo$StartStation.Name <- paste0 ("\"", lo$StartStation.Name, "\"")
    write.csv (lo, file = csv, quote = FALSE, row.names = FALSE, na = '')

    # dc has to have numeric fields quoted
    cols <- c ("Duration", "Start.station.number", "End.station.number")
    for (i in cols)
        env$bike_test_data$dc [[i]] <- as.character (env$bike_test_data$dc [[i]])

    # Then the remaining files
    cities <- c ('dc', 'la', 'ny')
    zips <- c ('sample-cabi-dc-trips-history-data.zip',
               'sample-la-metro.zip',
               'sample-citibike-tripdata.zip')
    csvs <- c ('2017Q1-capitalbikeshare-tripdata-temp.csv',
               'la_metro_gbfs_trips_Q1_2017.csv',
               '201612-citibike-tripdata.csv')
    quotes <- c (TRUE, FALSE, FALSE, FALSE)

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
#' \dontrun{
#' bike_write_test_data (data_dir = getwd ())
#' list.files ()
#' bike_rm_test_data (data_dir = getwd ())
#' }
bike_rm_test_data <- function (data_dir = tempdir ())
{
    files <- c ("01aJourneyDataExtract10Jan16-23Jan16.csv",
            "201701_hubway_tripdata.zip",
            "201801_hubway_tripdata.csv.zip",
            "Hubway_Stations_2011_2016.csv",
            "Hubway_Stations_as_of_July_2017.csv",
            "hubway_Trips_2012.csv",
            "Nice_Ride_2012-station_locations .csv",
            "Nice_Ride_data_2012_season.zip",
            "Nice_Ride_trip_history_2012_season.csv",
            "sample-cabi-dc-trips-history-data.zip",
            "sample-citibike-tripdata.zip",
            "sample-divvy-trips.zip",
            "sample-la-metro.zip")
    files <- file.path (data_dir, files)
    res <- NULL
    rm1 <- function (f) {
        tryCatch (file.remove (f),
                  warning = function (w) NULL,
                  error = function (e) NULL)
    }
    for (f in files)
        if (file.exists (f))
            res <- c (res, rm1 (f))

    return (length (res))
}
