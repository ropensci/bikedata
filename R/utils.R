#' List of cities currently included in bikedata
#'
#' @return A \code{data.frame} of cities, abbreviations, and names of bike
#' systems currently able to be accessed.
#'
#' @export
#'
#' @examples
#' bike_cities ()
bike_cities <- function ()
{
    dat <- bike_demographic_data ()
    dat$demographic_data <- NULL
    return (dat)
}


#' Convert city names to two-letter prefixes
#'
#' @param city Name of one or more cities or corresponding bicycle hire systems
#'
#' @return A two letter prefix matching (bo, ch, dc, la, lo, mn, ny, ph)
#'
#' @noRd
convert_city_names <- function (city)
{
    city <- gsub (' ', '', city)
    if (any (nchar (city) >= 4))
    {
        if (substring (tolower (city), 1, 4) == 'sant')
            city <- 'lo'
        else if (substring (tolower (city), 1, 4) == 'sanf')
            city <- 'sf'
    }
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
                     'ph', 'in', # philly indego
                     'mn', 'mi', # minneapolis/st.paul nice ride
                     'fo', 'go', 'sf', # ford gobike san fran
                     'mo', 'bi', # montreal bixi
                     'gu') # guadalajara mibici
    city_code <- c ('ny', 'ny', 'ny', 'bo', 'bo', 'ch', 'ch',
                    'dc', 'dc', 'dc', 'la', 'la', 'lo', 'lo', 'ph', 'ph',
                    'mn', 'mn', 'sf', 'sf', 'sf', 'mo', 'mo', 'gu')
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

# check whether data_dir exists and add option to create if not
# no code coverage coz it's interactive
check_data_dir <- function (x) # nocov start
{
    split_path <- function (x)
    {
        if (dirname(x)==x)
            x
        else
            c (basename (x), split_path (dirname (x)))
    }
    if (!file.exists (x))
    {
        message ("directory ", x, " does not exist")
        inp <- readline ("Should it be created (y/n)? ") %>%
                tolower ()
        if (substring (inp, 1, 1) == "y")
        {
            xsp <- rev (split_path (x)) [-1]
            for (i in seq_along (xsp))
            {
                fp <- do.call (file.path, as.list (xsp [1:i]))
                if (!file.exists (fp))
                    dir.create (fp)
            }
        } else
        {
            stop ("Okay, stopping now")
        }
    }
    invisible (x)
} # nocov end

# header files are parsed using sysdata.rda, which is written on load to the
# following file, subsequently read directly within the C++ routines
header_file_name <- function ()
{
    file.path (tempdir (), "field_names.csv")
}

data_has_stations <- function (city)
{
    cities <- bike_demographic_data ()$city
    ret <- rep (FALSE, length (cities))
    cities_with_station_data <- c ("ny", "la", "ph", "sf")
    ret [cities %in% cities_with_station_data] <- TRUE
    return (ret [which (cities == city)])
}
