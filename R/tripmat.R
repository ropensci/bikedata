#' Filter a trip matrix by date and/or time
#'
#' @param bikedb A string containing the path to the SQLite3 database.
#' If no directory specified, it is presumed to be in \code{tempdir()}.
#' @param ... Additional arguments including start_time, end_time, start_date,
#' end_date, and weekday
#'
#' @return A modified version of the \code{trips} table from \code{bikedb},
#' filtered by the specified times
#'
#' @noRd
filter_bike_tripmat <- function (bikedb, ...)
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
    # and NOTE here that the DISTINCT is necessary for Boston, which has a
    # station table with 300 entries for 193 stations, because lots change
    # names. All must nevertheless be stored so names in the trip data can be
    # mapped to IDs.
    qry <- paste("SELECT DISTINCT s1.stn_id AS start_station_id,",
                 "s2.stn_id AS end_station_id, iq.numtrips",
                 "FROM stations s1, stations s2 LEFT OUTER JOIN",
                 "(SELECT start_station_id, end_station_id,",
                 "COUNT(*) as numtrips FROM trips")

    qry_dt <- NULL
    if ('start_date' %in% names (x))
    {
      qry_dt <- c (qry_dt, "stop_time >= ?")
      qryargs <- c (qryargs, paste(x$start_date, '00:00:00'))
    }
    if ('end_date' %in% names (x))
    {
      qry_dt <- c (qry_dt, "start_time <= ?")
      qryargs <- c (qryargs, paste(x$end_date, '23:59:59'))
    }
    if ('start_time' %in% names (x))
    {
      qry_dt <- c (qry_dt, "time(stop_time) >= ?")
      qryargs <- c (qryargs, x$start_time)
    }
    if ('end_time' %in% names (x))
    {
      qry_dt <- c (qry_dt, "time(start_time) <= ?")
      qryargs <- c (qryargs, x$end_time)
    }

    qry_wd <- NULL
    if ('weekday' %in% names (x))
    {
        qry_wd <- "strftime('%w', start_time) IN "
        qry_wd <- paste0(qry_wd, " (",
                         paste (rep("?", times = length(x$weekday)),
                                collapse = ", "), ")")
        qry_dt <- c (qry_dt, qry_wd)
        qryargs <- c (qryargs, x$weekday)
    }

    qry_demog <- NULL
    if ('member' %in% names (x))
    {
        qry_demog <- c (qry_demog, "user_type = ?")
        qryargs <- c (qryargs, x$member)
    }
    if ('birth_year' %in% names (x))
    {
        qtmp <- add_birth_year_to_qry (qry_demog, qryargs, x$birth_year)
        qry_demog <- qtmp$qry
        qryargs <- qtmp$qryargs
    }
    if ('gender' %in% names (x))
    {
        qry_demog <- c (qry_demog, "gender = ?")
        qryargs <- c (qryargs, x$gender)
    }
    qry_dt <- c (qry_dt, qry_demog)

    qry <- paste (qry, "WHERE", paste (qry_dt, collapse = " AND "))
    qry <- paste (qry, "GROUP BY start_station_id, end_station_id) iq",
                  "ON s1.stn_id = iq.start_station_id AND",
                  "s2.stn_id = iq.end_station_id")

    if ('city' %in% names (x))
    {
        qry <- paste (qry, "WHERE s1.city = ? AND s2.city = ?")
        qryargs <- c (qryargs, rep (x$city, 2))
    }

    qry <- paste (qry, "ORDER BY s1.stn_id, s2.stn_id")

    db <- DBI::dbConnect (RSQLite::SQLite(), bikedb, create = FALSE)
    qryres <- DBI::dbSendQuery (db, qry)
    DBI::dbBind(qryres, as.list(qryargs))
    trips <- DBI::dbFetch(qryres)
    DBI::dbClearResult(qryres)
    DBI::dbDisconnect (db)

    return(trips)
}

#' add birth year specification to query
#'
#' @param qry The query character vector for demographic characteristics
#' @param qryargs The arguments associated with the query vector
#' @param birth_year Number vector of desired birth year(s)
#'
#' @return List containing modified versions of qry and qryargs
#'
#' @noRd
add_birth_year_to_qry <- function (qry, qryargs, birth_year)
{
        if (length (birth_year) == 1)
        {
            qry <- c (qry, "birth_year = ?")
            qryargs <- c (qryargs, birth_year)
        } else
        {
            qry <- c (qry, "birth_year >= ?", "birth_year <= ?")
            qryargs <- c (qryargs, min (birth_year), max (birth_year))
        }
        return (list (qry = qry, qryargs = qryargs))
}

#' Calculation station weights for standardising trip matrix by operating
#' durations of stations
#'
#' @param bikedb A string containing the path to the SQLite3 database.
#' If no directory specified, it is presumed to be in \code{tempdir()}.
#' @param city City for which standarisation is desired
#'
#' @return A vector of weights for each station of designated city. Weights are
#' standardised to sum to nstations, so overall numbers of trips remain the
#' same.
#'
#' @noRd
bike_tripmat_standardisation <- function (bikedb, city)
{
    dates <- bike_station_dates (bikedb, city = city)
    wt <- 1 / dates$ndays # shorter durations are weighted higher
    wt <- wt * nrow (dates) / sum (wt)
    names (wt) <- dates$station

    return (wt)
}

#' Transform specified membership to standard 0/1 form
#'
#' @param member Value as entered
#'
#' @return Standardised 0/1 equivalent specification
#'
#' @noRd
bike_transform_member <- function (member)
{
    if (!(is.logical (member) | member %in% 0:1))
        stop ('member must be TRUE/FALSE or 1/0')
    if (!member)
        member <- 0
    else if (member)
        member <- 1

    return (member)
}


#' Transform specified gender to standard numeric form
#'
#' @param gender Value as entered
#'
#' @return Standardised 0/1/2 equivalent specification
#'
#' @noRd
bike_transform_gender <- function (gender)
{
    if (!(is.numeric (gender) | is.character (gender)))
        stop ('gender must be numeric or character')
    if (is.numeric (gender) & (gender < 0 | gender > 2))
    {
        message ('gender only filtered for values of 0, 1, or 2')
        gender <- NULL
    } else if (is.character (gender))
    {
        gender <- tolower (substring (gender, 1, 1))
        if (gender == 'f')
            gender <- 2
        else if (gender == 'm')
            gender <- 1
        else
            gender <- 0
    }
    return (gender)
}

#' Extract station-to-station trip matrix or data.frame from SQLite3 database
#'
#' @param bikedb A string containing the path to the SQLite3 database.
#' If no directory specified, it is presumed to be in \code{tempdir()}.
#' @param city City for which tripmat is to be aggregated
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
#' @param member If given, extract only trips by registered members
#' (\code{member = 1} or \code{TRUE}) or not (\code{member = 0} or \code{FALSE}).
#' @param birth_year If given, extract only trips by registered members whose
#' declared birth years equal or lie within the specified value or values.
#' @param gender If given, extract only records for trips by registered
#' users declaring the specified genders (\code{f/m/.} or \code{2/1/0}).
#' @param standardise If TRUE, numbers of trips are standardised to the
#' operating durations of each stations, so trip numbers are increased for
#' stations that have only operated a short time, and vice versa.
#' @param long If FALSE, a square tripmat of (num-stations, num_stations) is
#' returned; if TRUE, a long-format matrix of (stn-from, stn-to, ntrips) is
#' returned.
#' @param quiet If FALSE, progress is displayed on screen
#'
#' @return If \code{long = FALSE}, a square matrix of numbers of trips between
#' each station, otherwise a long-form \pkg{tibble} with three columns of of
#' (\code{start_station_id, end_station_id, numtrips}).
#'
#' @note The \code{city} parameter should be given for databases containing data
#' from multiple cities, otherwise most of the resultant trip matrix is likely
#' to be empty.  Both dates and times may be given either in numeric or
#' character format, with arbitrary (or no) delimiters between fields. Single
#' numeric times are interpreted as hours, with 24 interpreted as day's end at
#' 23:59:59.
#'
#' @note If \code{standardise = TRUE}, the trip matrix will have the same number
#' of trips, but they will be re-distributed as described, with more recent
#' stations having more trips than older stations. Trip number are also
#' non-integer in this case, whereas they are always integer-valued for
#' \code{standardise = FALSE}.
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
#' 
#' tm <- bike_tripmat (bikedb = bikedb, city = 'ny') # full trip matrix
#' tm <- bike_tripmat (bikedb = bikedb, city = 'ny',
#'                     start_date = 20161201, end_date = 20161201)
#' tm <- bike_tripmat (bikedb = bikedb, city = 'ny', start_time = 1)
#' tm <- bike_tripmat (bikedb = bikedb, city = 'ny', start_time = "01:00")
#' tm <- bike_tripmat (bikedb = bikedb, city = 'ny', end_time = "01:00")
#' tm <- bike_tripmat (bikedb = bikedb, city = 'ny', 
#'                     start_date = 20161201, start_time = 1)
#' tm <- bike_tripmat (bikedb = bikedb, city = 'ny', start_date = 20161201,
#'                     end_date = 20161201, start_time = 1, end_time = 2)
#' tm <- bike_tripmat (bikedb = bikedb, city = 'ny', weekday = 5)
#' tm <- bike_tripmat (bikedb = bikedb, city = 'ny', weekday = c('f', 'sa', 'th'))
#' tm <- bike_tripmat (bikedb = bikedb, city = 'ny', weekday = c('f', 'th', 'sa'))
#' tm <- bike_tripmat (bikedb = bikedb, city = 'ny', member = 1)
#' tm <- bike_tripmat (bikedb = bikedb, city = 'ny', birth_year = 1976)
#' tm <- bike_tripmat (bikedb = bikedb, city = 'ny', birth_year = 1976:1990)
#' tm <- bike_tripmat (bikedb = bikedb, city = 'ny', gender = 'f')
#' tm <- bike_tripmat (bikedb = bikedb, city = 'ny',
#'                     gender = 'm', birth_year = 1976:1990)
#' 
#' bike_rm_test_data (data_dir = data_dir)
#' bike_rm_db (bikedb)
#' # don't forget to remove real data!
#' # file.remove (list.files (data_dir, pattern = '.zip'))
#' }
bike_tripmat <- function (bikedb, city, start_date, end_date,
                          start_time, end_time, weekday,
                          member, birth_year, gender,
                          standardise = FALSE,
                          long = FALSE, quiet = FALSE)
{
    if (missing (bikedb))
        stop ("Can't get trip matrix if bikedb isn't provided")

    bikedb <- check_db_arg (bikedb)
    city <- check_city_arg (bikedb, city)
    dl <- vapply (bike_datelimits (bikedb), function (i)
                  strsplit (i, " ") [[1]] [1], "character") %>%
                as.character ()

    x <- c (NULL, 'city' = city)
    if (!missing (start_date))
    {
        dl [1] <- convert_ymd (start_date)
        x <- c (x, 'start_date' = dl [1])
    }
    if (!missing (end_date))
    {
        dl [2] <- convert_ymd (end_date)
        x <- c (x, 'end_date' = dl [2])
    }
    if (!missing (start_time))
        x <- c (x, 'start_time' = convert_hms (start_time))
    if (!missing (end_time))
        x <- c (x, 'end_time' = convert_hms (end_time))
    if (!missing (weekday))
        x <- c (x, 'weekday' = list (convert_weekday (weekday)))

    if ( (!missing (birth_year) | !missing (gender)) &
        !city %in% (c ('bo', 'ch', 'ny')))
        stop ('Only Boston, Chicago, and New York provide demographic data')
    if ( !missing (member) & !city %in% c ('bo', 'ch', 'ny', 'la', 'ph'))
        stop (paste0 ('Only Boston, Chicago, New York, LA, and ',
                      'Philly provide member/non-member data'))
    if (!missing (member))
        x <- c (x, 'member' = bike_transform_member (member))
    if (!missing (birth_year))
    {
        if (!is.numeric (birth_year))
            stop ('birth_year must be numeric')
        x <- c (x, 'birth_year' = list (birth_year))
    }
    if (!missing (gender))
        if (!is.null (bike_transform_gender (gender)))
            x <- c (x, 'gender' = bike_transform_gender (gender))

    if ( (missing (city) & length (x) > 0) |
        (!missing (city) & length (x) > 1) )
    {
        trips <- filter_bike_tripmat (bikedb, x)
    } else
    {
        # NOTE that the DISTINCT is necessary for Boston, which has a station
        # table with 300 entries for 193 stations, because lots change names.
        # All must nevertheless be stored so names in the trip data can be
        # mapped to IDs.
        qry <- paste("SELECT DISTINCT s1.stn_id AS start_station_id,",
                     "s2.stn_id AS end_station_id, iq.numtrips",
                     "FROM stations s1, stations s2 LEFT OUTER JOIN",
                     "(SELECT start_station_id, end_station_id,",
                     "COUNT(*) as numtrips FROM trips",
                     "GROUP BY start_station_id, end_station_id) iq",
                     "ON s1.stn_id = iq.start_station_id AND",
                     "s2.stn_id = iq.end_station_id")
        if (!missing (city))
            qry <- paste0 (qry, " WHERE s1.city = '", city,
                           "' AND s2.city = '", city, "'")
        qry <- paste (qry, "ORDER BY s1.stn_id, s2.stn_id")

        db <- DBI::dbConnect (RSQLite::SQLite(), bikedb, create = FALSE)
        trips <- DBI::dbGetQuery (db, qry)
        DBI::dbDisconnect(db)
    }


    if (standardise)
    {
        wts <- bike_tripmat_standardisation (bikedb, city)
        wts_start <- wts [match (trips$start_station_id, names (wts))]
        wts_end <- wts [match (trips$end_station_id, names (wts))]
        trips$numtrips <- trips$numtrips *
            do.call (pmin, data.frame (wts_start, wts_end) [-1])
        # Then round to 3 places
        trips$numtrips <- round (trips$numtrips, digits = 3)
    }

    if (!long)
    {
        trips <- long2wide (trips)
        trips [is.na (trips)] <- 0
    } else
    {
        trips$numtrips <- ifelse (is.na (trips$numtrips) == TRUE, 0,
                                  trips$numtrips)
        trips <- tibble::as_tibble (trips)
    }
    attr (trips, "variable") <- "numtrips" # used in bike_match_matrices
    attr (trips, "bikedata_version") <- packageVersion ("bikedata")
    attr (trips, "start_date") <- dl [1]
    attr (trips, "end_date") <- dl [2]

    return (trips)
}

#' convert long-form trip or distance tibble to square matrix
#'
#' @param mat Long-form trip or distance matrix
#' @return Equivalent square matrix
#'
#' @noRd
long2wide <- function (mat)
{
    variable <- "numtrips"
    if ("numtrips" %in% names (mat))
        mat <- reshape2::dcast (mat, start_station_id ~ end_station_id,
                                value.var = "numtrips", fill = 0,
                                fun.aggregate = sum)
    else
    {
        mat <- reshape2::dcast (mat, start_station_id ~ end_station_id,
                                value.var = "distance")
        variable <- "distance"
    }

    row.names (mat) <- mat$start_station_id
    mat$start_station_id <- NULL
    mat <- as.matrix (mat)
    attr (mat, "variable") <- variable

    return (mat)
}

#' convert wide-form trip or distance matrix to long form
#'
#' @param mat Wide-form trip or distance matrix
#' @return Equivalent long-form tibble
#'
#' @note This is only used in \code{match_dmat_tmat}
#'
#' @noRd
bike_wide2long <- function (mat)
{
    zvar <- attr (mat, "variable") # "numtrips" or "distance"

    mat <- reshape2::melt (mat,
                           id.vars = c (rownames (mat), colnames (mat)))
    names (mat) <- c ("start_station_id", "end_station_id", zvar)

    return (mat)
}
