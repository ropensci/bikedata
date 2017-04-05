#' get_awsbike_files
#'
#' Returns list of URLs for each trip data file from nominated system
#'
#' @param name Name of the AWS bucket in which data are stored
#'
#' @return List of URLs used to download data
#'
#' @note bukets which work straight from AWS are
#' c ('tripdata', 'capitalbikeshare-data', 'hubway-data') for the cities
#' c ('ny', 'dc', 'bo'), respectively.
#'
#' @noRd
get_aws_bike_files <- function (bucket)
{
    host <- "https://s3.amazonaws.com"
    aws_url <- sprintf ("https://%s.s3.amazonaws.com", bucket)

    doc <- httr::content (httr::GET (aws_url), encoding  =  'UTF-8')
    nodes <- xml2::xml_children (doc)
    # NOTE: xml2::xml_find_all (doc, ".//Key") should work here but doesn't, so
    # this manually does what that would do
    files <- lapply (nodes, function (i)
                     if (grepl ('zip', i))
                         strsplit (strsplit (as.character (i),
                                 "<Key>") [[1]] [2], "</Key>") [[1]] [1] )
    # nyc citibike data has a redundamt file as first item
    files <- unlist (files)
    if (bucket == 'tripdata')
        files <- files [2:length (files)]
    paste0 (host, "/", bucket, "/", files)
}

#' get_la_bike_files
#'
#' Returns list of URLs for each trip data file from LA's Metro system
#'
#' @return List of URLs used to download data
#'
#' @note This system is brand new, and the data URLs will likely resolve to some
#' more systematic form very soon. This is just a temporary function providing
#' explicit URLs until that time.
#'
#' @noRd
get_la_bike_files <- function ()
{
    host <- paste0 ("https://11ka1d3b35pv1aah0c3m9ced-wpengine.netdna-ssl.com/",
                    "wp-content/uploads/")
    files <- c ("2016/10/MetroBikeShare_2016_Q3_trips.zip",
                "2017/01/Metro_trips_Q4_2016.zip")
    paste0 (host, files)
}

#' get_boston_bike_files
#'
#' Returns list of URLs for each trip data file from Boston's Hubway system
#'
#' @return List of URLs used to download data
#'
#' @note This is also an AWS, but it requires an OAuth key for direct access.
#' The AWS URLs are nevertheless listed in the main \code{html} file. 
#'
#' @noRd
get_chicago_bike_files <- function ()
{
    host <- "https://www.divvybikes.com/system-data"
    . <- NULL # suppress R CMD check note
    nodes <- httr::content (httr::GET (host), encoding = 'UTF-8') %>%
        xml2::xml_find_all (., ".//aside") %>%
        xml2::xml_find_all (., ".//a")
    unique (xml2::xml_attr (nodes, "href"))
}



#' get_bike_files
#'
#' Returns list of URLs for each trip data file from nominated system
#'
#' @param city The city for which data are to be obtained
#'
#' @return List of URLs used to download data
#'
#' @noRd
get_bike_files <- function (city)
{
    city <- convert_city_names (city)
    aws_cities <- c ('ny', 'dc', 'bo')
    buckets <- c ('tripdata', 'capitalbikeshare-data', 'hubway-data')

    if (city %in% aws_cities)
    {
        bucket <- buckets [match (city, aws_cities)]
        files <- get_aws_bike_files (bucket)
    } else if (city == 'la')
        files <- get_la_bike_files ()
    else if (city == 'ch')
        files <- get_chicago_bike_files ()

    return (files)
}
