#' convert hms to 'HH:MM:SS'
#'
#' @param x A numeric or character object to to be converted
#'
#' @return A string formatted to 'HH:MM:SS'
#'
#' @noRd
convert_hms <- function (x)
{
    if (is.numeric (x)) # presume it's HH
    {
        if (x < 0 | x > 24)
            stop ('hms values must be between 0 and 24')
        if (x < 24)
            res <- paste0 (sprintf ('%02d', x), ':00:00')
        else
            res <- paste0 (23, ':59:59')
    } else if (is.character (x))
    {
        # split at all non-numeric characters
        x <- sapply (strsplit (x, '[^0-9]') [[1]], as.numeric)
        if (length (x) == 0)
            stop ('Can not convert to hms without numeric values')
        if (length (x) == 1)
        {
            if (x < 24)
                res <- paste0 (sprintf ('%02d', x [1]), ':00:00')
            else
                res <- paste0 (23, ':59:59')
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

#' convert weekday vector to numbered weekdays
#'
#' @param wd Vector of numeric or character denoting weekdays
#'
#' @return Equivalent character vector of numbered weekdays
#'
#' @noRd
convert_weekday <- function (wd)
{
    if (!is.numeric (wd))
    {
        if (!is.character (wd))
            stop ("don't know how to convert weekdays of class ", class (wd))
        wdlist <- c ("sunday", "monday", "tuesday", "wednesday", 
                     "thursday", "friday", "saturday")
        wd <- sapply (tolower (wd), function (i)
                      {
                          res <- grep (paste0 ("\\<", i), wdlist)
                          if (length (res) != 1)
                              res <- NA
                          return (res)
                      })
        if (any (is.na (wd)))
            stop ('weekday specification is ambiguous')
    } else if (any (!wd %in% 1:7))
        stop ('weekdays must be between 1 and 7')
    return (paste (sort (wd) - 1)) # sql is 0-indexed
}

#' Extract date-time limits from trip database
#'
#' @param bikedb Path to the SQLite3 database 
#'
#' @return A vector of 2 elements giving the date-time of the first and last
#' trips
#'
#' @export
get_datelimits <- function (bikedb)
{
    # suppress R CMD check notes:
    start_time <- stop_time <- NULL
    db <- dplyr::src_sqlite (bikedb, create=F)
    trips <- dplyr::tbl (db, 'trips')
    first_trip <- dplyr::summarise (trips, first_trip=min (start_time)) %>%
        dplyr::collect () %>% as.character ()
    last_trip <- dplyr::summarise (trips, last_trip=max (stop_time)) %>%
        dplyr::collect () %>% as.character ()
    res <- c (first_trip, last_trip)
    names (res) <- c ('first', 'last')
    return (res)
}

#' Filter a trip matrix by date and/or time
#'
#' @param db The spatiallite database containing the trips (and established by a
#' \code{dplyr::src_sqlite} call)
#' @param ... Additional arguments including start_time, end_time, start_date,
#' end_date, and weekday
#'
#' @return A modified version of the \code{trips} table from \code{db}, filtered
#' by the specified times
#'
#' @noRd
filter_tripmat_by_datetime <- function (db, ...)
{
    # NOTE that this approach is much more efficient than the `dplyr::filter`
    # command, because that can only be applied to the entire datetime field:
    # dplyr::filter (trips, start_time > "2014-07-07 00:00:00", 
    #                       start_time < "2014-07-10 23:59:59")
    # ... and there is no direct way to filter based on distinct dates AND
    # times, nor can the SQL `date` and `time` functions be applied through
    # dplyr.
    x <- as.list (...)
    qryargs <- c()
    qry <- paste( "SELECT s1.id AS start_station_id, s2.id AS end_station_id, iq.numtrips",
                   "FROM stations s1, stations s2 LEFT OUTER JOIN",
                   "(SELECT start_station_id, end_station_id, COUNT(*) as numtrips FROM trips")
    
    qry_dt <- NULL
    if ('start_date' %in% names (x)) {
      qry_dt <- c (qry_dt, "stop_time >= ?")
      qryargs <- c (qryargs, paste(x$start_date, '00:00:00'))
    }
    if ('end_date' %in% names (x)) {
      qry_dt <- c (qry_dt, "start_time <= ?")
      qryargs <- c (qryargs, paste(x$end_date, '23:59:59'))
    }
    if ('start_time' %in% names (x)) {
      qry_dt <- c (qry_dt, "time(stop_time) >= ?")
      qryargs <- c (qryargs, x$start_time)
    }
    if ('end_time' %in% names (x)) {
      qry_dt <- c (qry_dt, "time(start_time) <= ?")
      qryargs <- c (qryargs, x$end_time)
    }

    qry_wd <- NULL
    if ('weekday' %in% names (x))
    {
        qry_wd <- "strftime('%w', start_time) IN "
        qry_wd <- paste0(qry_wd, " (", paste(rep("?", times = length(x$weekday)), collapse=", "), ")")
        qry_dt <- c (qry_dt, qry_wd)
        qryargs <- c (qryargs, x$weekday)
    }

    qry <- paste (qry, "WHERE", paste (qry_dt, collapse=" AND "))
    qry <- paste (qry, "GROUP BY start_station_id, end_station_id) iq",
                  "ON s1.id = iq.start_station_id AND s2.id = iq.end_station_id",
                  "ORDER BY s1.id, s2.id")

    qryres <- RSQLite::dbSendQuery(db, qry)
    RSQLite::dbBind(qryres, as.list(qryargs))
    trips <- RSQLite::dbFetch(qryres)
    RSQLite::dbClearResult(qryres)
    
    return(trips)
    
}

#' Extract station-to-station trip matrix from SQLite3 database
#'
#' @param bikedb Path to the SQLite3 database 
#' @param start_date If given (as year, month, day) , extract only those records
#' from and including this date
#' @param end_date If given (as year, month, day), extract only those records to
#' and including this date
#' @param start_time If given, extract only those records starting from and
#' including this time of each day
#' @param end_time If given, extract only those records ending at and including
#' this time of each day
#' @param weekday If given, extract only those records including the nominated
#' weekdays. This can be a vector of numeric, starting with Sunday=1, or
#' unambiguous characters, so "sa" and "tu" for Saturday and Tuesday.
#' @param long If FALSE, a square tripmat of (num-stations, num_stations) is
#' returns; if TRUE, a long-format matrix of (stn-from, stn-to, ntrips) is
#' returned.
#' @param quiet If FALSE, progress is displayed on screen
#'
#' @return Square matrix of numbers of trips between each station
#'
#' @note Both dates and times may be given either in numeric or character
#' format, with arbitrary (or no) delimiters between fields. Single numeric
#' times are interpreted as hours, with 24 interpreted as day's end at 23:59:59.
#'
#' @export
tripmat <- function (bikedb, start_date, end_date, start_time, end_time,
                     weekday, long=FALSE, quiet=FALSE)
{
    # suppress R CMD check notes:
    stop_time <- sttrip_id <- st <- et <- NULL
    db <- RSQLite::dbConnect(SQLite(), bikedb, create = FALSE)

    x <- NULL
    if (!missing (start_date))
        x <- c (x, 'start_date' = paste (lubridate::ymd (start_date)))
    if (!missing (end_date))
        x <- c (x, 'end_date' = paste (lubridate::ymd (end_date)))
    if (!missing (start_time))
        x <- c (x, 'start_time' = convert_hms (start_time))
    if (!missing (end_time))
        x <- c (x, 'end_time' = convert_hms (end_time))
    if (!missing (weekday))
        x <- c (x, 'weekday' = list (convert_weekday (weekday)))

    # create generic table, while is replaced if filtered by time
    if (length (x) > 0) {
        trips <- filter_tripmat_by_datetime (db, x)
    }
    else {
      trips <- dbGetQuery(db, 
                paste("SELECT s1.id AS start_station_id, s2.id AS end_station_id, iq.numtrips",
                "FROM stations s1, stations s2 LEFT OUTER JOIN",
                "(SELECT start_station_id, end_station_id, COUNT(*) as numtrips FROM trips",
                "GROUP BY start_station_id, end_station_id) iq",
                "ON s1.id = iq.start_station_id AND s2.id = iq.end_station_id ORDER BY s1.id, s2.id"))
    }
    
    RSQLite::dbDisconnect(db)

    # suppress R CMD check notes:
    start_station_id <- end_station_id <- n <- NULL
    if (long == FALSE) {
      ntrips <- reshape2::dcast(trips, start_station_id ~ end_station_id, 
                                value.var = "numtrips", fill = 0)
      row.names(ntrips) <- ntrips$start_station_id
      ntrips$start_station_id <- NULL
      ntrips <- as.matrix(ntrips)
    }
    else {
      trips$numtrips <- ifelse(is.na(trips$numtrips) == TRUE, 0, trips$numtrips)
      ntrips <- as.matrix(trips)
    }

    return (ntrips)

}

