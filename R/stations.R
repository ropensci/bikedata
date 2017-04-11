#' Extract station matrix from SQLite3 database
#'
#' @param bikedb Path to the SQLite3 database 
#'
#' @return Matrix containing data for each station
#'
#' @export
bike_stations <- function (bikedb)
{
    db <- dplyr::src_sqlite (bikedb, create = F)
    dplyr::collect (dplyr::tbl (db, 'stations'))
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
                                gsub ("'", "", i$commonName)))
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
        name <- c (name, fi$name)
        lon <- c (lon, fi$longitude)
        lat <- c (lat, fi$latitude)
    }
    res <- data.frame (id = id, name = name, lon = lon, lat = lat)
    res <- res [which (!duplicated (res)), ]

    return (res)
}
