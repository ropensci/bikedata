#' Extract station matrix from SQLite3 database
#'
#' @param bikedb A string containing the path to the SQLite3 database.
#' If no directory specified, it is presumed to be in \code{tempdir()}.
#' @param city Optional city (or vector of cities) for which stations are to be
#' extracted
#'
#' @return Matrix containing data for each station
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
#' stations <- bike_stations (bikedb)
#' head (stations)
#' 
#' bike_rm_test_data (data_dir = data_dir)
#' bike_rm_db (bikedb)
#' # don't forget to remove real data!
#' # file.remove (list.files (data_dir, pattern = '.zip'))
#' }
bike_stations <- function (bikedb, city)
{
    if (missing (bikedb))
        stop ("Can't get station data if bikedb isn't provided")

    bikedb <- check_db_arg (bikedb)

    db <- DBI::dbConnect (RSQLite::SQLite(), bikedb, create = FALSE)
    st <- tibble::as.tibble (DBI::dbReadTable (db, 'stations'))
    DBI::dbDisconnect (db)

    if (!missing (city))
        st <- st [which (st$city %in% convert_city_names (city)), ]

    st$longitude <- as.numeric (st$longitude)
    st$latitude <- as.numeric (st$latitude)
    # some token/test stns don't have lat-lons, and these become NAs
    indx <- which (!is.na (st$longitude) & !is.na (st$latitude))
    st <- st [indx, ]

    return (st)
}

#' Get London station data from Transport for London (TfL)
#'
#' @return \code{data.frame} of (id, name, lon, lat) of all stations in London's
#' Santander Cycles system
#'
#' @noRd
bike_get_london_stations <- function ()
{
    tfl_url <- "https://api.tfl.gov.uk/BikePoint"
    resp <- httr::GET (tfl_url)
    res <- NULL
    if (resp$status_code == 200)
    {
        doc <- httr::content (resp, encoding  =  'UTF-8')
        id <- unlist (lapply (doc, function (i)
                              strsplit (i$id, "BikePoints_") [[1]] [2]))
        name <- unlist (lapply (doc, function (i)
                                gsub ("'", "", i$commonName))) #nolint
        lon <- unlist (lapply (doc, function (i) i$lon))
        lat <- unlist (lapply (doc, function (i) i$lat))
        res <- data.frame (id = id, name = name, lon = lon, lat = lat)
    }
    return (res)
}

#' Get Chicago station data
#'
#' @param flists List of files returned from bike_unzip_files_chicago 
#'
#' @return \code{data.frame} of (id, name, lon, lat) of all stations in Chicago's
#' Divvybikes system
#'
#' @noRd
bike_get_chicago_stations <- function (flists)
{

    id <- name <- lon <- lat <- NULL
    for (f in flists$flist_csv_stns)
    {
        fi <- read.csv (f, header = TRUE)
        id <- c (id, paste0 (fi$id))
        name <- c (name, paste0 (fi$name))
        lon <- c (lon, paste0 (fi$longitude))
        lat <- c (lat, paste0 (fi$latitude))
    }
    res <- data.frame (id = id, name = name, lon = lon, lat = lat)
    res <- res [which (!duplicated (res)), ]

    return (res)
}

#' Get Washington DC station data
#'
#' @return \code{data.frame} of (id, name, lon, lat) of all stations in
#' Washington DC's Capital Bike Share system
#'
#' @note This data is available online from 
#' http://opendata.dc.gov/datasets/capital-bike-share-locations/
#' but this is a wrapper around an opendata.argis.com server that is not
#' reliable because it very commonly returns errors and fails to retrieve the
#' data. The relevant data were therefore downloaded and stored in
#' R/sysdata.rda. These data will need updating as the system expands in the
#' future.
#'
#' @noRd
bike_get_dc_stations <- function ()
{
    # rm apostrophes from names (only "L'Enfant Plaza"):
    # stations_dc is lazy loaded from R/sysdata.rda
    name <- noquote (gsub ("'", "", stations_dc$address)) #nolint
    name <- trimws (name, which = 'right') # trim terminal white space
    res <- data.frame (id = stations_dc$terminal_number,
                       name = name,
                       lon = stations_dc$longitude,
                       lat = stations_dc$latitude)

    return (res)
}
