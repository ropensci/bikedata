#' Store hire bicycle data in SQLite3 database
#'
#' @param city One or more cities for which to download and store bike data, or
#'          names of corresponding bike systems (see Details below).
#' @param data_dir A character vector giving the directory containing the
#'          data files downloaded with \code{dl_bikedata} for one or more
#'          cities. Only if this parameter is missing will data be downloaded.
#' @param bikedb A string containing the path to the SQLite3 database to 
#'          use. If it doesn't already exist, it will be created, otherwise data
#'          will be appended to existing database.  If no directory specified,
#'          it is presumed to be in \code{tempdir()}.
#' @param dates If specified and no \code{data_dir} is given, data are
#' downloaded and stored only for these dates specified as vector of YYYYMM
#' values.
#' @param quiet If FALSE, progress is displayed on screen
#'
#' @return Number of trips added to database
#'
#' @section Details:
#' City names are not case sensitive, and must only be long enough to
#' unambiguously designate the desired city. Names of corresponding bike systems
#' can also be given.  Currently possible cities (with minimal designations in
#' parentheses) and names of bike hire systems are:
#' \tabular{lr}{
#'  New York City (ny)\tab Citibike\cr
#'  Washington, D.C. (dc)\tab Capital Bike Share\cr
#'  Chicago (ch)\tab Divvy Bikes\cr
#'  Los Angeles (la)\tab Metro Bike Share\cr
#'  Boston (bo)\tab Hubway\cr
#'  Philadelphia (ph)\tab Indego\cr
#' }
#'
#' @note Data for different cities are all stored in the same database, with
#' city identifiers automatically established from the names of downloaded data
#' files. This function can take quite a long time to execute (typically > 10
#' minutes), and generates a SQLite3 database file several gigabytes in size.
#' Downloaded data files are removed after loading into the database; files may
#' be downloaded and stored permanently with \code{dl_bikedata}, and the
#' corresponding \code{data_dir} passed to this function.
#' 
#' @export
#' 
#' @examples
#' \dontrun{
#' data_dir <- tempdir ()
#' bike_write_test_data (data_dir = data_dir)
#' # or download some real data!
#' # dl_bikedata (city = 'la', data_dir = data_dir)
#' bikedb <- file.path (data_dir, 'testdb')
#' store_bikedata (data_dir = data_dir, bikedb = bikedb)
#' # create database indexes for quicker access:
#' index_bikedata_db (bikedb = bikedb)
#'
#' trips <- bike_tripmat (bikedb = bikedb, city = 'LA') # trip matrix
#' stations <- bike_stations (bikedb = bikedb) # station data
#' 
#' bike_rm_test_data (data_dir = data_dir)
#' bike_rm_db (bikedb)
#' # don't forget to remove real data!
#' # file.remove (list.files (data_dir, pattern = '.zip'))
#' }
store_bikedata <- function (bikedb, city, data_dir, dates = NULL, quiet = FALSE)
{
    if (missing (city) & missing (data_dir))
    {
        mt <- paste0 ("Calling this function without specifying a city or ",
                     "data_dir will download\n*ALL* avaialable data ",
                     "for all cities and store it in a *HUGE* database.\n",
                     "This will likely take quite a long time. ",
                     "Is this really what you want to do?")
        val <- menu (c ("yes", "no"), graphics = FALSE, title = mt)
        if (val != 1)
            stop ('Yeah, probably better not to do that')
        city <- bike_demographic_data ()$city
    }

    if (missing (data_dir))
    {
        if (!quiet)
            message ('Checking data for ', city)
        for (ci in city)
            dl_bikedata (city = ci, dates = dates, quiet = quiet)
        data_dir <- tempdir ()
    } else if (missing (city))
    {
        if (length (list.files (data_dir)) == 0)
            stop ('data_dir contains no files')
        city <- get_bike_cities (expand_home (data_dir))
    }
    data_dir <- expand_home (data_dir)

    if (missing (bikedb))
        stop ("Can't store bikedata if bikedb isn't provided")
    if (!(grepl ('/', bikedb) | grepl ('*//*', bikedb)))
        bikedb <- file.path (tempdir (), bikedb)

    bikedb <- expand_home (bikedb)

    city <- convert_city_names (city)

    er_idx <- file.exists (bikedb) + 1 # = (1, 2) if (!exists, exists)
    if (!quiet)
        message (c ('Creating', 'Adding data to') [er_idx], ' sqlite3 database')
    (!file.exists (bikedb))
    {
        chk <- rcpp_create_sqlite3_db (bikedb)
        if (chk != 0)
            stop ('Unable to create SQLite3 database')
    }

    ntrips <- 0
    for (ci in city)
    {
        if (!quiet)
        {
            if (length (city) == 1 & ci != 'lo')
                message ('Unzipping raw data files ...')
            else if (ci != 'lo') # mostly csv files that don't need unzipping
                message ('Unzipping raw data files for ', ci, ' ...')
        }
        if (ci == 'ch')
            flists <- bike_unzip_files_chicago (data_dir, bikedb, dates)
        else
            flists <- bike_unzip_files (data_dir, bikedb, ci, dates)

        if (!quiet & length (city) > 1)
            message ('Reading files for ', ci, ' ...')

        if (length (flists$flist_csv) > 0)
        {
            # Import file names to datafile table
            nf <- num_datafiles_in_db (bikedb)
            if (length (flists$flist_zip) > 0)
                nf <- rcpp_import_to_file_table (bikedb,
                                                 basename (flists$flist_zip),
                                                 ci, nf)
            if (ci == 'lo' & length (flists$flist_csv) > 0)
                nf <- rcpp_import_to_file_table (bikedb,
                                                 basename (flists$flist_csv),
                                                 ci, nf)

            # import stations to stations table
            if (ci == 'ch')
            {
                ch_stns <- bike_get_chicago_stations (flists)
                if (nrow (ch_stns) > 0)
                    nstations <- rcpp_import_stn_df (bikedb, ch_stns, 'ch')
            } else if (ci == 'lo')
            {
                lo_stns <- bike_get_london_stations ()
                nstations <- rcpp_import_stn_df (bikedb, lo_stns, 'lo')
            } else if (ci == 'dc')
            {
                dc_stns <- bike_get_dc_stations ()
                nstations <- rcpp_import_stn_df (bikedb, dc_stns, 'dc')
            }

            # main step: Import trips
            ntrips_city <- rcpp_import_to_trip_table (bikedb, flists$flist_csv,
                                                      ci, quiet)

            if (length (flists$flist_rm) > 0)
               invisible (tryCatch (file.remove (flists$flist_rm),
                                warning = function (w) NULL,
                                error = function (e) NULL))
            if (!quiet & length (city) > 1)
                message ('Trips read for ', ci, ' = ',
                         format (ntrips_city, big.mark = ',',
                                 scientific = FALSE), '\n')
            ntrips <- ntrips + ntrips_city
        }
    }

    if (!quiet)
        if (ntrips > 0)
        {
            message ('Total trips ', c ('read', 'added') [er_idx], ' = ',
                     format (ntrips, big.mark = ',', scientific = FALSE))
            if (er_idx == 2)
                message ("database '", basename (bikedb), "' now has ",
                         format (bike_db_totals (bikedb), big.mark = ',',
                                 scientific = FALSE), ' trips')
        } else
            message ('All data already in database; no new data added')

    return (ntrips)
}

#' Add indexes to database created with store_bikedata
#'
#' @param bikedb The SQLite3 database containing the bikedata.
#'
#' @export
#' 
#' @examples
#' \dontrun{
#' data_dir <- tempdir ()
#' bike_write_test_data (data_dir = data_dir)
#' # or download some real data!
#' # dl_bikedata (city = 'la', data_dir = data_dir)
#' bikedb <- file.path (data_dir, 'testdb')
#' store_bikedata (data_dir = data_dir, bikedb = bikedb)
#' # create database indexes for quicker access:
#' index_bikedata_db (bikedb = bikedb)
#'
#' trips <- bike_tripmat (bikedb = bikedb, city = 'LA') # trip matrix
#' stations <- bike_stations (bikedb = bikedb) # station data
#' 
#' bike_rm_test_data (data_dir = data_dir)
#' bike_rm_db (bikedb)
#' # don't forget to remove real data!
#' # file.remove (list.files (data_dir, pattern = '.zip'))
#' }
index_bikedata_db <- function (bikedb)
{
    if (missing (bikedb))
        stop ("bikedb must be provided in order to create indexes")

    bikedb <- check_db_arg (bikedb)

    db <- DBI::dbConnect (RSQLite::SQLite(), bikedb, create = FALSE)
    idx_list <- DBI::dbGetQuery (db, "PRAGMA index_list (trips)")
    DBI::dbDisconnect (db)

    reindex <- 'idx_trips_city' %in% idx_list$name
    chk <- rcpp_create_city_index (bikedb, reindex) # nolint

    reindex <- (nrow (idx_list) > 2)
    chk <- rcpp_create_db_indexes (bikedb,
                                   tables = rep("trips", times = 4),
                                   cols = c("start_station_id",
                                            "end_station_id",
                                            "start_time", "stop_time"),
                                   reindex) # nolint
}

#' Remove SQLite3 database generated with 'store_bikedat()'
#'
#' If no directory is specified the \code{bikedb} argument passed to
#' \code{store_bikedata}, the database is created in \code{tempdir()}. This
#' function provides a convenient way to remove the database in such cases by
#' simply passing the name.
#'
#' @param bikedb The SQLite3 database containing the bikedata.
#'
#' @return TRUE if \code{bikedb} successfully removed; otherwise FALSE
#'
#' @export
#'
#' @examples
#' \dontrun{
#' data_dir <- tempdir ()
#' bike_write_test_data (data_dir = data_dir)
#' # or download some real data!
#' # dl_bikedata (city = 'la', data_dir = data_dir)
#' bikedb <- file.path (data_dir, 'testdb')
#' store_bikedata (data_dir = data_dir, bikedb = bikedb)
#' 
#' bike_rm_test_data (data_dir = data_dir)
#' bike_rm_db (bikedb)
#' # don't forget to remove real data!
#' # file.remove (list.files (data_dir, pattern = '.zip'))
#' }
bike_rm_db <- function (bikedb)
{
    if (missing (bikedb))
        stop ("Can'remove database if bikedb isn't provided")

    bikedb <- check_db_arg (bikedb)

    ret <- tryCatch (file.remove (bikedb),
                     warning = function (w) NULL,
                     error = function (e) NULL)

    return (ret)
}

#' Get list of cities from files in specified data directory
#'
#' @param data_dir A character vector giving the directory containing the
#'          \code{.zip} files of citibike data.
#'
#' @noRd
get_bike_cities <- function (data_dir)
{
    ptn <- '.zip'
    flist <- list.files (data_dir)
    if (any (grepl ('cyclehireusagestats', flist, ignore.case = TRUE) |
             grepl ('JourneyDataExtract', flist, ignore.case = TRUE)))
        ptn <- paste0 (ptn, '|.csv') # London has raw csv files too
    flist <- list.files (data_dir, pattern = ptn)

    n <- nrow (bike_demographic_data ())
    cities <- as.list (rep (FALSE, n))
    names (cities) <- bike_demographic_data ()$city

    if (any (grepl ('citibike', flist, ignore.case = TRUE)))
        cities$ny <- TRUE
    if (any (grepl ('divvy', flist, ignore.case = TRUE)))
        cities$ch <- TRUE
    if (any (grepl ('hubway', flist, ignore.case = TRUE)))
        cities$bo <- TRUE
    if (any (grepl ('cabi', flist, ignore.case = TRUE)))
        cities$dc <- TRUE
    if (any (grepl ('cyclehireusagestats', flist, ignore.case = TRUE) |
             grepl ('JourneyDataExtract', flist, ignore.case = TRUE)))
        cities$lo <- TRUE
    if (any (grepl ('metro', flist, ignore.case = TRUE)))
        cities$la <- TRUE
    if (any (grepl ('indego', flist, ignore.case = TRUE)))
        cities$ph <- TRUE

    cities <- which (unlist (cities))
    names (cities)
}

#' Get list of data files for a particular city in specified directory and not
#' in database
#'
#' @param data_dir Directory containing data files
#' @param bikedb name of bikedata database
#' @param city One of (nyc, boston, chicago, dc, la, philly)
#'
#' @return Only those members of flist corresponding to nominated city
#'
#' @noRd
get_flist_city <- function (data_dir, bikedb, city)
{
    city <- convert_city_names (city)

    flist <- list.files (data_dir, pattern = '.zip')

    index <- NULL
    if (any (city == 'ny'))
        index <- which (grepl ('citibike', flist, ignore.case = TRUE))
    else if (any (city == 'ch'))
        index <- which (grepl ('divvy', flist, ignore.case = TRUE))
    else if (any (city == 'bo'))
        index <- which (grepl ('hubway', flist, ignore.case = TRUE))
    else if (any (city == 'dc'))
        index <- which (grepl ('cabi', flist, ignore.case = TRUE))
    else if (any (city == 'la'))
        index <- which (grepl ('metro', flist, ignore.case = TRUE))
    else if (any (city == 'lo'))
        index <- which (grepl ('Journey', flist, ignore.case = TRUE) |
                        grepl ('cyclehireusage', flist, ignore.case = TRUE))
    else if (any (city == 'ph'))
        index <- which (grepl ('indego', flist, ignore.case = TRUE))

    ret <- NULL
    if (length (index) > 0)
        ret <- file.path (data_dir, flist [index])

    db <- DBI::dbConnect (RSQLite::SQLite(), bikedb, create = FALSE)
    db_files <- DBI::dbGetQuery (db, "SELECT * FROM datafiles")
    DBI::dbDisconnect (db)

    db_files <- db_files$name [db_files$city == city]
    db_files <- file.path (data_dir, db_files)

    return (ret [!ret %in% db_files])
}

#' Get list of files to be unzipped and added to database
#'
#' @param data_dir Directory containing data files
#' @param bikedb A string containing the path to the SQLite3 database.
#' If no directory specified, it is presumed to be in \code{tempdir()}.
#' @param city City for which files are to be added to database
#' @param dates If specified, data will be unzipped only for these dates
#'
#' @return List of three vectors of file names:
#' \itemize{
#' \item @code{flist_zip} contains names of all zip archives to be added to
#' database;
#' \item \code{flist_csv} contains all corresponding \code{.csv} files;
#' \item \code{flist_rm} contains files to be deleted after having been added,
#' }
#'
#' @note This function is to be applied only for those cities publishing zip
#' archives containing a single file: NYC, Boston. Other cities which publish
#' multi-file zip archives (Chicago) have their own equivalent routines.
#'
#' @noRd
bike_unzip_files <- function (data_dir, bikedb, city, dates)
{
    flist_zip <- get_flist_city (data_dir = data_dir, bikedb = bikedb,
                                 city = city)
    # These are only those files in data_dir but **not** in bikedb. Remove any
    # not in dates:
    if (!is.null (dates))
    {
        dates <- bike_convert_dates (dates) %>%
            expand_dates_to_range %>%
            convert_dates_to_filenames (city = city)
        indx <- which (grepl (paste (dates, collapse = "|"), flist_zip))
        flist_zip <- flist_zip [indx]
    }
    existing_csv_files <- list.files (data_dir, pattern = '\\.csv$')
    flist_csv <- flist_rm <- NULL

    # Recent London data files are not compressed
    if (city == 'lo' && length (existing_csv_files) > 0)
    {
        flist_csv <- get_new_datafiles (bikedb, existing_csv_files)
        if (length (flist_csv) > 0)
            flist_csv <- file.path (data_dir, basename (flist_csv))
    }

    if (length (flist_zip) > 0)
    {
        flist_zip <- get_new_datafiles (bikedb, flist_zip)
        for (f in flist_zip)
        {
            fi <- unzip (f, list = TRUE)$Name
            flist_csv <- c (flist_csv, basename (fi))
            if (!all (fi %in% existing_csv_files))
            {
                unzip (f, exdir = data_dir, junkpaths = TRUE)
                flist_rm <- c (flist_rm, fi)
            }
        }
        if (length (flist_csv) > 0)
            flist_csv <- file.path (data_dir, basename (flist_csv))
        if (length (flist_rm) > 0)
            flist_rm <- file.path (data_dir, basename (flist_rm))
    }

    return (list (flist_zip = flist_zip,
                  flist_csv = flist_csv,
                  flist_rm = flist_rm))
}

#' Get list of Chicago files to be unzipped and added to database
#'
#' @param data_dir Directory containing data files
#' @param bikedb A string containing the path to the SQLite3 database 
#' If no directory specified, it is presumed to be in \code{tempdir()}.
#' @param dates If specified, data will be unzipped only for these dates
#'
#' @return List of three vectors of file names:
#' \itemize{
#' \item @code{flist_zip} contains names of all zip archives to be added to
#' database;
#' \item \code{flist_csv_trip} contains all corresponding \code{.csv} files of
#' trip data;
#' \item \code{flist_csv_stn} contains all corresponding \code{.csv} files of
#' station data;
#' \item \code{flist_rm} contains files to be deleted after having been added,
#' }
#'
#' @note File 'Divvy_Stations_Trips_2014_Q1Q2.zip' has a 'Divvy_Stations' file
#' in \code{.xlsx} format. The stations are identical to those from 2013, so
#' this is *NOT* extracted here. That's the only archive with an \code{.xlxs}
#' file.
#'
#' @noRd
bike_unzip_files_chicago <- function (data_dir, bikedb, dates)
{
    flist_zip <- get_flist_city (data_dir = data_dir, bikedb = bikedb,
                                 city = 'ch')
    if (!is.null (dates))
    {
        dates <- bike_convert_dates (dates) %>%
            expand_dates_to_range %>%
            convert_dates_to_filenames (city = "ch")
        indx <- which (grepl (paste (dates, collapse = "|"), flist_zip))
        flist_zip <- flist_zip [indx]
    }

    flist_zip <- get_new_datafiles (bikedb, flist_zip)
    existing_csv_files <- list.files (data_dir, pattern = "Divvy.*\\.csv")
    if (length (existing_csv_files) == 0)
        existing_csv_files <- NULL
    flist_csv_trips <- flist_csv_stns <- flist_rm <- NULL
    if (length (flist_zip) > 0)
    {
        for (f in flist_zip)
        {
            fi <- unzip (f, list = TRUE)$Name
            fi_trips <- fi [which (grepl ('Trips.*\\.csv', basename (fi)))]
            fi_stns <- fi [which (grepl ('Stations', basename (fi)) &
                                            grepl ('.csv', basename (fi)))]
            flist_csv_trips <- c (flist_csv_trips, basename (fi_trips))
            flist_csv_stns <- c (flist_csv_stns, basename (fi_stns))
            if (!all (basename (fi_trips) %in% existing_csv_files))
            {
                unzip (f, files = fi_trips, exdir = data_dir, junkpaths = TRUE)
                flist_rm <- c (flist_rm, basename (fi_trips))
            }
            if (length (fi_stns) > 0) # always except 2014_Q1Q2 with .xlsx
                if (!all (basename (fi_stns) %in% existing_csv_files))
                {
                    unzip (f, files = fi_stns, exdir = data_dir,
                           junkpaths = TRUE)
                    flist_rm <- c (flist_rm, basename (fi_stns))
                }
        }
        flist_csv_trips <- file.path (data_dir, basename (flist_csv_trips))
        flist_csv_trips <- file.path (data_dir, basename (flist_csv_trips))
        flist_csv_stns <- file.path (data_dir, basename (flist_csv_stns))
        if (length (flist_rm) > 0)
            flist_rm <- file.path (data_dir, basename (flist_rm))
    }
    return (list (flist_zip = flist_zip,
                  flist_csv = flist_csv_trips,
                  flist_csv_stns = flist_csv_stns,
                  flist_rm = flist_rm))
}
