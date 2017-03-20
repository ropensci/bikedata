#' get_awsbike_files
#'
#' Returns list of URLs for each trip data file from nominated system
#'
#' @param name Name of the AWS bucket in which data are stored
#'
#' @return List of URLs used to download data
#'
#' @noRd
get_aws_bike_files <- function (bucket)
{
    host <- "https://s3.amazonaws.com"
    aws_url <- sprintf ("https://%s.s3.amazonaws.com", bucket)

    doc <- httr::content (httr::GET (aws_url), encoding='UTF-8')
    nodes <- xml2::xml_children (doc)
    # NOTE: xml2::xml_find_all (doc, ".//Key") should work here but doesn't, so
    # this manually does what that would do
    files <- lapply (nodes, function (i)
                     if (grepl ('zip', i))
                         strsplit (strsplit (as.character (i), "<Key>") [[1]] [2], 
                                   "</Key>") [[1]] [1] )
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
    nodes <- httr::content (httr::GET (host), encoding='UTF-8') %>%
        xml2::xml_find_all (., ".//aside") %>%
        xml2::xml_find_all (., ".//a")
    xml2::xml_attr (nodes, "href")
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
get_bike_files <- function (city="nyc")
{
    city <- tolower (gsub ("[[:punct:]]", "", city))
    aws <- TRUE

    if (grepl ('ny', city) | grepl ('citi', city))
        name <- "tripdata"
    else if (grepl ('dc', city) | grepl ('wash', city) | grepl ('cap', city))
        name <- "capitalbikeshare-data"
    else if (grepl ('bos', city) | grepl ('hub', city))
        name <- "hubway-data"
    else
        aws <- FALSE

    files <- NULL
    if (aws)
        files <- get_aws_bike_files (name)
    else
    {
        if (grepl ('la', city) | grepl ('los', city))
            files <- get_la_bike_files ()
        if (grepl ('ch', city) | grepl ('di', city))
            files <- get_chicago_bike_files ()
    }

    return (files)
}


#' Download hire bicycle data
#'
#' @param city City for which to download bike data, or name of corresponding
#' bike system (see Details below).
#' @param data_dir Directory to which to download the files
#' @param dates Character vector of dates to download data with dates formated
#' as YYYYMM.
#'
#' @section Details:
#' This function produces zip-compressed data in R's temporary directory. City
#' names are not case sensitive, and must only be long enough to unambiguously
#' designate the desired city. Names of corresponding bike systems can also be
#' given.  Currently possible cities (with minimal designations in parentheses)
#' and names of bike hire systems are:
#' \tabular{lr}{
#'  New York City (ny)\tab Citibike\cr
#'  Washington, D.C. (dc)\tab Capital Bike Share\cr
#'  Chicago (ch)\tab Divvy Bikes\cr
#'  Los Angeles (la)\tab Metro Bike Share\cr
#'  Boston (bo)\tab Hubway\cr
#' }
#'
#' Ensure you have a fast internet connection and at least 100 Mb space
#'
#' @note Only files that don't already exist in \code{data_dir} will be
#' downloaded, and this function may thus be used to update a directory of files
#' by downloading more recent files.
#'
#' @export
dl_bikedata <- function(city='nyc', data_dir = tempdir(), dates = NULL)
{
    dl_files <- get_bike_files (city)
    files <- file.path (data_dir, basename (dl_files))

    if (is.null (dates)) 
      indx <- which (!file.exists (files))
    else 
      indx <- which (!file.exists (files) & 
                     grepl (paste (dates, collapse="|"), files))

    if (length (indx) > 0)
    {
        for (f in dl_files [indx])
        {
            destfile <- file.path (data_dir, basename(f))
            download.file (f, destfile)
        }
    } else
        message ('All data files already exist')
    invisible (list.files (data_dir, pattern='.zip', full.names=TRUE))
}

#' Store nyc-citibike data in spatialite database
#'
#' 
#' @param data_dir A character vector giving the directory containing the
#' \code{.zip} files of citibike data.
#' @param spdb A string containing the path to the spatialite database to 
#' use. It will be created automatically.
#' @param quiet If FALSE, progress is displayed on screen
#' @param create_index If TRUE, creates an index on the start and end station
#' IDs and start and stop times.
#'
#' @note This function can take quite a long time to execute (typically > 10
#' minutes), and generates a spatialite database file several gigabytes in size.
#' 
#' @export
store_bikedata <- function (data_dir, spdb, quiet=FALSE, create_index = TRUE)
{
    if (file.exists (spdb))
        stop ('File named ', spdb, ' already exists')

    # platform-independent unzipping is much easier in R
    flist <- file.path (data_dir, list.files (data_dir, pattern=".zip"))
    if (!quiet)
        message ('Extracting files ', appendLF=FALSE)
    for (f in flist)
    {
        fcsv <- unzip (f, list=TRUE)$Name
        if (!exists (fcsv))
            unzip (f, exdir=data_dir)
        if (!quiet)
            message ('.', appendLF=FALSE)
    }
    if (!quiet)
        message (' done')
    flist_csv <- file.path (data_dir, list.files (data_dir, pattern=".csv"))
    ntrips <- importDataToSpatialite (flist_csv, spdb, quiet)
    if (!quiet)
        message ('total trips read = ', 
                 format (ntrips, big.mark=',', scientific=FALSE))
    if (create_index == TRUE) {
      if (!quiet) {
        message ('Creating indexes')
      }
      create_db_indexes(spdb, 
                      tables = rep("trips", times=4),
                      cols = c("start_station_id", "end_station_id", "start_time", "stop_time"))
    }
    invisible (file.remove (flist_csv))
}
