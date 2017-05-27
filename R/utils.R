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
                     'la', 'me',
                     'lo', 'sa' ) # london santander
    city_code <- c ('ny', 'ny', 'ny', 'bo', 'bo', 'ch', 'ch',
                    'dc', 'dc', 'dc', 'la', 'la', 'lo', 'lo')
    city_code <- city_code [pmatch (city, city_names)]

    if (length (indx_lo) > 0)
    {
        city <- rep (NA, min (1, length (city)))
        city [indx_lo] <- city_lo
        city [indx] <- city_code
    }

    return (city)
}
