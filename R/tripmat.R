#' Render station-to-station matrix square by inserting extra rows or cols
#'
#' @param mat Station-to-station matrix of trip numbers
#'
#' @return Re-shaped version of mat that has equal numbers of rows and columns
reshape_tripmat <- function (mat)
{
    rnames <- as.numeric (rownames (mat))
    cnames <- as.numeric (colnames (mat))
    if (!identical (sort (cnames), cnames))
        mat <- mat [,order (cnames)]
    if (!identical (sort (rnames), rnames))
        mat <- mat [order (rnames),]

    allnames <- sort (unique (c (cnames, rnames)))
    if (length (allnames) > max (c (nrow (mat), ncol (mat))))
    {
        cindx <- which (allnames %in% cnames)
        rindx <- which (allnames %in% rnames)
        mat_ex <- array (0, dim=rep (length (allnames), 2))
        mat_ex [rindx, cindx] <- mat
        colnames (mat_ex) <- rownames (mat_ex) <- paste (allnames)
        mat <- mat_ex
    }

    return (mat)
}

#' convert hms to 'HH:MM:SS'
#'
#' @param x A numeric or character object to to be converted
#'
#' @return A string formatted to 'HH:MM:SS'
convert_hms <- function (x)
{
    if (is.numeric (x)) # presume it's HH
    {
        if (x < 0 | x > 23)
            stop ('hms values must be between 0 and 23')
        if (x < 23)
            res <- paste0 (sprintf ('%02d', x), ':00:00')
        else
            res <- paste0 (sprintf ('%02d', x), ':59:59')
    } else if (is.character (x))
    {
        # split at all non-numeric characters
        x <- sapply (strsplit (x, '[^0-9]') [[1]], as.numeric)
        if (length (x) == 0)
            stop ('Can not convert to hms without numeric values')
        if (length (x) == 1)
        {
            if (x < 23)
                res <- paste0 (sprintf ('%02d', x [1]), ':00:00')
            else
                res <- paste0 (sprintf ('%02d', x [1]), ':59:59')
        }
        else if (length (x) == 2)
            res <- paste0 (sprintf ('%02d', x [1]), ':',
                           sprintf ('%02d', x [2]), ':00')
        else if (length (x) == 3)
            res <- paste0 (sprintf ('%02d', x [1]), ':',
                           sprintf ('%02d', x [2]), ':',
                           sprintf ('%02d', x [2]))
        else
            warning ('only first 3 numeric components used to convert to hms')
    } else
        stop ('hms values must be either numeric or character')
    
    return (res)
}

#' Extract date-time limits from trip database
#'
#' @param spdb Path to the spatialite database 
#'
#' @return A vector of 2 elements giving the date-time of the first and last
#' trips
#'
#' @export
get_datelimits <- function (spdb)
{
    # suppress R CMD check notes:
    start_time <- stop_time <- NULL
    db <- dplyr::src_sqlite (spdb, create=F)
    trips <- dplyr::tbl (db, 'trips')
    first_trip <- dplyr::summarise (trips, first_trip=min (start_time)) %>%
        dplyr::collect () %>% as.character ()
    last_trip <- dplyr::summarise (trips, last_trip=max (stop_time)) %>%
        dplyr::collect () %>% as.character ()
    res <- c (first_trip, last_trip)
    names (res) <- c ('first', 'last')
    return (res)
}

#' Filter a trip matrix by time
#'
#' @param db The spatiallite database containing the trips (and established by a
#' \code{dplyr::src_sqlite} call)
#' @param start_time Extract only those records starting from and including this
#' time of each day
#' @param end_time Extract only those records ending at and including this time
#' of each day
#'
#' @return A modified version of the \code{trips} table from \code{db}, filtered
#' by the specified times
filter_tripmat_by_time <- function (db, start_time=NULL, end_time=NULL)
{
    trips <- dplyr::tbl (db, 'trips') # avoid CMD check message
    if (!is.null (start_time) & !is.null (end_time))
    {
        # TODO: This qry yields a tbl with variables named "sttrip_id" and "et"
        # instead of the desired "st" and "et" - fix!
        qry <- paste0 ("WITH t AS (SELECT trip_id, start_station_id, ",
                       "end_station_id, start_time, stop_time FROM trips)",
                       "SELECT trip_id, start_station_id, end_station_id, ",
                       "start_time, stop_time, time(start_time) AS st",
                       "trip_id, time(stop_time) AS et FROM t")
        trips <- dplyr::tbl (db, dplyr::sql (qry))
        trips <- dplyr::filter (trips, sttrip_id >= start_time & et <= end_time)
        # Alternative is left_join like this:
        #qry <- paste0 ("WITH t AS (SELECT trip_id, start_time FROM trips)",
        #               "SELECT trip_id, time(start_time) AS st FROM t")
        #starts <- dplyr::tbl (db, dplyr::sql (qry))
        #starts <- dplyr::filter (trips, starts >= start_time)
        #qry <- paste0 ("WITH t AS (SELECT trip_id, stop_time FROM trips)",
        #               "SELECT trip_id, time(stop_time) AS et FROM t")
        #ends <- dplyr::tbl (db, dplyr::sql (qry))
        #ends <- dplyr::filter (trips, ends <= end_time)
        #trips <- dplyr::left_join (starts, ends)
    } else if (!is.null (start_time))
    {
        qry <- paste0 ("WITH t AS (SELECT trip_id, start_station_id, ",
                       "end_station_id, start_time, stop_time FROM trips)",
                       "SELECT trip_id, start_station_id, end_station_id, ",
                       "start_time, stop_time, time(start_time) AS st FROM t")
        trips <- dplyr::tbl (db, dplyr::sql (qry))
        trips <- dplyr::filter (trips, st >= start_time)
    } else if (!is.null (end_time))
    {
        qry <- paste0 ("WITH t AS (SELECT trip_id, start_station_id, ",
                       "end_station_id, start_time, stop_time FROM trips)",
                       "SELECT trip_id, start_station_id, end_station_id, ",
                       "start_time, stop_time, time(stop_time) AS et FROM t")
        trips <- dplyr::tbl (db, dplyr::sql (qry))
        trips <- dplyr::filter (trips, et <= end_time)
    }
    return (trips)
}

#' Extract station-to-station trip matrix from spatialite database
#'
#' @param spdb Path to the spatialite database 
#' @param start_date If given (as year, month, day) , extract only those records
#' from and including this date
#' @param end_date If given (as year, month, day), extract only those records to
#' and including this date
#' @param start_time If given, extract only those records starting from and
#' including this time of each day
#' @param end_time If given, extract only those records ending at and including
#' this time of each day
#' @param quiet If FALSE, progress is displayed on screen
#'
#' @return Square matrix of numbers of trips between each station
#'
#' @note Both dates and times may be given either in numeric or character
#' format, with arbitrary (or no) delimiters between fields. Single numeric
#' times are interpreted as hours, with 23 interpreted as day's end at 23:59:59.
#'
#' @export
tripmat <- function (spdb, start_date, end_date, start_time, end_time,
                     quiet=FALSE)
{
    # suppress R CMD check notes:
    stop_time <- sttrip_id <- st <- et <- NULL
    db <- dplyr::src_sqlite (spdb, create=F)

    if (!missing (start_date))
        start_date <- paste (lubridate::ymd (start_date), '00:00:00')
    else
        start_date <- NULL
    if (!missing (end_date))
        end_date <- paste (lubridate::ymd (end_date), '23:59:59')
    else
        end_date <- NULL
    if (!missing (start_time))
        start_time <- convert_hms (start_time)
    else
        start_time <- NULL
    if (!missing (end_time))
        end_time <- convert_hms (end_time)
    else
        end_time <- NULL

    # create generic table, while is replaced if filtered by time
    trips <- dplyr::tbl (db, 'trips') 
    # Filter by time - this requires making a new table with time colunms
    if (!is.null (start_time) | !is.null (end_time))
        trips <- filter_tripmat_by_time (db, start_time, end_time)
    # Filter by date
    if (!is.null (start_date) & !is.null (end_date))
        trips <- dplyr::filter (trips, start_time >= start_date &
                                stop_time <= end_date)
    else if (!is.null (start_date))
        trips <- dplyr::filter (trips, start_time >= start_date)
    else if (!is.null (end_date))
        trips <- dplyr::filter (trips, stop_time <= end_date)

    # suppress R CMD check notes:
    start_station_id <- end_station_id <- n <- NULL
    byid <- dplyr::group_by (trips, start_station_id, end_station_id)
    tripdb <- dplyr::summarise (byid, count=n())
    if (!quiet)
        message ('Counting numbers of trips from spatialite database ... ',
                 appendLF=FALSE)
    ntrips <- xtabs (count ~ start_station_id + end_station_id, data=tripdb)
    if (!quiet)
        message ('done')

    ntrips <- reshape_tripmat (ntrips)

    return (as (ntrips, 'matrix'))
}

