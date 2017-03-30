#' Convert city names to two-letter prefixes
#'
#' @param city Name of one or more cities or corresponding bicycle hire systems
#'
#' @return A two letter prefix matching (ny, bo, ch, dc, la)
#'
#' @noRd
convert_city_names <- function (city)
{
    city_names <- c ('ny', 'ne', 'ci', # nyc citibike
                     'bo', 'hu', # boston hubway
                     'ch', 'di', # chicago divvy bike
                     'wa', 'dc', 'ca', # washington dc capital bike share
                     'la', 'lo', 'me') # los angeles metro
    city_code <- c ('ny', 'ny', 'ny', 'bo', 'bo', 'ch', 'ch', 
                    'dc', 'dc', 'dc', 'la', 'la', 'la') 
    city <- substring (gsub ('[[:punct:]]', '', tolower (city)), 1, 2)
    city_code [pmatch (city, city_names)]
}
