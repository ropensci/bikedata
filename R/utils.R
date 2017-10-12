#' Convert city names to two-letter prefixes
#'
#' @param city Name of one or more cities or corresponding bicycle hire systems
#'
#' @return A two letter prefix matching (ny, bo, ch, dc, la)
#'
#' @noRd
convert_city_names <- function (city)
{
    city <- gsub (' ', '', city)
    city <- substring (gsub ('[[:punct:]]', '', tolower (city)), 1, 3)
    indx_lo <- which (city %in% c ('lon', 'los'))
    indx <- which (!seq (city) %in% indx_lo)
    if (length (indx_lo) > 0)
    {
        city_lo <- city [indx_lo]
        city <- city [indx]
        city_lo [city_lo == 'lon'] <- 'lo'
        city_lo [city_lo == 'los'] <- 'la'
    }
    city <- substring (city, 1, 2)

    city_names <- c ('ny', 'ne', 'ci', # nyc citibike
                     'bo', 'hu', # boston hubway
                     'ch', 'di', # chicago divvy bike
                     'wa', 'dc', 'ca', # washington dc capital bike share
                     'la', 'me', # LA metro
                     'lo', 'sa', # london santander
                     'ph', 'in') # philly indego
    city_code <- c ('ny', 'ny', 'ny', 'bo', 'bo', 'ch', 'ch',
                    'dc', 'dc', 'dc', 'la', 'la', 'lo', 'lo', 'ph', 'ph')
    city_code <- city_code [pmatch (city, city_names)]

    if (length (indx_lo) > 0)
    {
        city <- rep (NA, min (1, length (city)))
        city [indx_lo] <- city_lo
        city [indx] <- city_code
    } else
        city <- city_code

    if (any (is.na (city)))
        stop ("city not recognised")

    return (city)
}

#' check city arg
#'
#' @param bikedb Name of database holding bike trip data
#' @param city Name of city as passed to functions such as \code{bike_tripmat},
#' \code{bike_stations}, or \code{bike_distmat}
#' @return Standardised version of \code{city} parameter
#' @noRd
check_city_arg <- function (bikedb, city)
{
    db_cities <- bike_cities_in_db (bikedb)
    if (missing (city))
    {
        if (length (db_cities) > 1)
        {
            stop ('bikedb contains multiple cities; please specify one.',
                  'cities in current database are [',
                  paste (db_cities, collapse = ' '), ']')
        } else
            city <- db_cities [1]
    } else if (!missing (city))
    {
        city <- convert_city_names (city)
        if (is.na (city))
            stop ('city not recognised')
        if (!city %in% bike_cities_in_db (bikedb))
            stop ('city ', city, ' not represented in database')
    }
    return (city)
}

#' Perform checks for name, existance, and structure of bikedb
#'
#' @param bikedb A string containing the path to the SQLite3 database.
#' If no directory specified, it is presumed to be in \code{tempdir()}.
#'
#' @return Potentially modified string containing full path
#'
#' @noRd
check_db_arg <- function (bikedb)
{
    if (exists (bikedb, envir = parent.frame ()))
        bikedb <- get (bikedb, envir = parent.frame ())

    bikedb <- expand_home (bikedb)

    # Note that dirname (bikedb) == '.' can not be used because that prevents
    # bikedb = "./bikedb", so grepl must be used instead.
    if (!grepl ('/', bikedb) | !grepl ('*//*', bikedb))
        bikedb <- file.path (tempdir (), bikedb)

    if (!file.exists (bikedb))
        stop ('file ', basename (bikedb), ' does not exist')

    db <- DBI::dbConnect(RSQLite::SQLite(), bikedb, create = FALSE)
    qry <- 'SELECT name FROM sqlite_master WHERE type = "table"'
    tbls <- DBI::dbGetQuery(db, qry) [, 1]
    DBI::dbDisconnect(db)
    if (!identical (tbls, c ('trips', 'stations', 'datafiles')))
        stop ('bikedb does not appear to be a bikedata database')

    return (bikedb)
}

# expand unix-style tidle for home directory
expand_home <- function (x)
{
    if (grepl ("~", x))
        x <- gsub ("~", Sys.getenv ("HOME"), x)
    return (x)
}
