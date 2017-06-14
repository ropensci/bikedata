#' get_awsbike_files
#'
#' Returns list of URLs for each trip data file from nominated Amazon Web
#' Services system
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

    doc <- httr::content (httr::GET (aws_url), encoding = 'UTF-8')
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


#' get_chicago_bike_files
#'
#' Returns list of URLs for each trip data file from Chicago's Divvy system
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


#' get_london_bike_files
#'
#' Returns list of URLs for each trip data file from London's Santander Cycles
#' system
#'
#' @return List of URLs used to download data
#'
#' @noRd
get_london_bike_files <- function ()
{
    # First get list of base file names from AWS:
    aws_url <- "https://s3-eu-west-1.amazonaws.com/cycling.data.tfl.gov.uk/"
    doc <- httr::content (httr::GET (aws_url), encoding  =  'UTF-8')
    nodes <- xml2::xml_children (doc)
    getflist <- function (nodes, type = 'zip')
    {
        f <- lapply (nodes, function (i) if (grepl (type, i))
                     strsplit (strsplit (as.character (i), "<Key>") [[1]] [2],
                               "</Key>") [[1]] [1])
        basename (unlist (f))
    }
    flist_zip <- getflist (nodes, type = 'zip')
    flist_zip <- flist_zip [which (grepl ('usage', flist_zip))]
    flist_csv <- getflist (nodes, type = 'csv')

    # Then convert to tfl.gov.uk filenames
    addr_base <- "http://cycling.data.tfl.gov.uk/usage-stats/"
    paste0 (addr_base, c (flist_zip, flist_csv))
}



#' get_nabsa_files
#'
#' Get list of URL for trip data from North American Bike Share Association
#' systems (currently LA and Philly).
#'
#' @noRd
get_nabsa_files <- function (city)
{
    if (city == 'ph')
        the_url <- "https://www.rideindego.com/about/data/"
    else if (city == 'la')
        the_url <- "https://bikeshare.metro.net/about/data/"
    else
        stop ('nabsa cities must be ph or la')

    doc <- httr::content (httr::GET (the_url), encoding = 'UTF-8',
                          as = 'parsed')
    hrefs <- xml2::xml_attr (xml2::xml_find_all (doc, ".//a"), "href")
    hrefs [which (grepl ("\\.zip", hrefs) & !grepl ("[Ss]tation", hrefs))]
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
    aws_cities <- c ('ny', 'dc', 'bo')
    buckets <- c ('tripdata', 'capitalbikeshare-data', 'hubway-data')
    nabsa_cities <- c ('la', 'ph')

    if (city %in% aws_cities)
    {
        bucket <- buckets [match (city, aws_cities)]
        files <- get_aws_bike_files (bucket)
    } else if (city %in% nabsa_cities)
        files <- get_nabsa_files (city = city)
    else if (city == 'ch')
        files <- get_chicago_bike_files ()
    else if (city == 'lo')
        files <- get_london_bike_files ()

    return (files)
}
