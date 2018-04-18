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
    # and some have lat-lons of zero, so remove these too
    indx <- which (abs (st$longitude) > 1e-6 &
                   abs (st$latitude) > 1e-6)
    st <- st [indx, ]

    return (st)
}

#' Get London station data from Transport for London (TfL)
#'
#' @param quiet If \code{FALSE}, just declare getting stations (coz it can take
#' a while).
#' @return \code{data.frame} of (id, name, lon, lat) of all stations in London's
#' Santander Cycles system
#'
#' @noRd
bike_get_london_stations <- function (quiet = TRUE)
{
    if (!quiet)
        message ("getting london stations ...", appendLF = FALSE)
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
        res <- data.frame (id = id, name = name, lon = lon, lat = lat,
                           stringsAsFactors = FALSE)
    }
    if (!quiet)
        message (" done")

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
    res <- data.frame (id = id, name = name, lon = lon, lat = lat,
                       stringsAsFactors = FALSE)
    res <- res [which (!duplicated (res)), ]

    return (res)
}

#' Get Boston station data
#'
#' @param flists List of files returned from bike_unzip_files, which includes
#' entries in \code{$flist_csv_stns}
#'
#' @return \code{data.frame} of (id, name, lon, lat) of all stations in Boston's
#' Hubway system
#'
#' @noRd
bike_get_bo_stations <- function (flists, data_dir)
{
    if (is.null (flists$flist_csv_stns))
    {
        # then download station data ...
        dl_files <- get_bike_files (city = 'bo')
        dl_files <- dl_files [which (grepl ('Stations', dl_files))]
        for (f in dl_files)
        {
            furl <- gsub (" ", "%20", f)
            f <- gsub (" ", "", f)
            destfile <- file.path (data_dir, basename(f))
            resp <- httr::GET (furl,
                               httr::write_disk (destfile, overwrite = TRUE))
            if (resp$status_code != 200)
            {
                count <- 0
                while (!file.exists (destfile) & count < 5)
                {
                    resp <- httr::GET (furl,
                                       httr::write_disk (destfile,
                                                         overwrite = TRUE))
                    count <- count + 1
                }
                if (!file.exists (destfile))
                    stop ('Download request failed')
            }
        }
        flists$flist_csv_stns <- file.path (data_dir, basename (dl_files))
    }

    id <- name <- lon <- lat <- NULL
    for (f in flists$flist_csv_stns)
    {
        fi <- read.csv (f, header = TRUE)
        id <- c (id, paste0 (fi$Station.ID))
        name <- c (name, paste0 (fi$Station))
        lon <- c (lon, paste0 (fi$Longitude))
        lat <- c (lat, paste0 (fi$Latitude))
    }
    # Remove apostrophes from names coz they muck up sqlite fields:
    name <- gsub ("\'", "", name)
    res <- data.frame (id = id, name = name, lon = lon, lat = lat,
                       stringsAsFactors = FALSE)
    res <- res [which (!duplicated (res)), ]

    return (res)
}

#' Get Minneapolis/Minnesota station data
#'
#' @return \code{data.frame} of (id, name, lon, lat) of all stations in Boston's
#' Hubway system
#'
#' @noRd
bike_get_mn_stations <- function (flists)
{
    if (is.null (flists$flist_csv_stns))
        stop ("Station files must be in nominated data_dir")

    id <- name <- lon <- lat <- NULL
    for (f in flists$flist_csv_stns)
    {
        fi <- read.csv (f, header = TRUE)
        idcol <- grep ("terminal|number", names (fi), ignore.case = TRUE)
        nmcol <- grep ("station|name", names (fi), ignore.case = TRUE)
        loncol <- grep ("lon", names (fi), ignore.case = TRUE)
        latcol <- grep ("lat", names (fi), ignore.case = TRUE)
        id <- c (id, paste0 (fi [[idcol]]))
        name <- c (name, paste0 (fi [[nmcol]]))
        lon <- c (lon, paste0 (fi [[loncol]]))
        lat <- c (lat, paste0 (fi [[latcol]]))
    }
    # Remove apostrophes from names coz they muck up sqlite fields:
    name <- gsub ("\'", "", name)
    res <- data.frame (id = id, name = name, lon = lon, lat = lat,
                       stringsAsFactors = FALSE)
    res <- res [which (!duplicated (res)), ]

    indx <- which (res$lon != "N/A" & res$lon != "NA" &
                   res$lat != "N/A" & res$lat != "NA")
    return (res [indx, ])
}

#' Get Montreal station data
#'
#' @return \code{data.frame} of (id, name, lon, lat) of all stations in
#' Montreal's Bixi system
#'
#' @noRd
bike_get_mo_stations <- function (flists)
{
    if (is.null (flists$flist_csv_stns))
        stop ("Station files must be in nominated data_dir")

    id <- name <- lon <- lat <- NULL
    for (f in flists$flist_csv_stns)
    {
        fi <- read.csv (f, header = TRUE)
        idcol <- grep ("code", names (fi), ignore.case = TRUE)
        nmcol <- grep ("name", names (fi), ignore.case = TRUE)
        loncol <- grep ("longitude", names (fi), ignore.case = TRUE)
        latcol <- grep ("latitude", names (fi), ignore.case = TRUE)
        id <- c (id, paste0 (fi [[idcol]]))
        name <- c (name, paste0 (fi [[nmcol]]))
        lon <- c (lon, paste0 (fi [[loncol]]))
        lat <- c (lat, paste0 (fi [[latcol]]))
    }
    # Remove apostrophes from names coz they muck up sqlite fields:
    name <- gsub ("\'", "", name)
    res <- data.frame (id = id, name = name, lon = lon, lat = lat,
                       stringsAsFactors = FALSE)
    res <- res [which (!duplicated (res)), ]

    indx <- which (res$lon != "N/A" & res$lon != "NA" &
                   res$lat != "N/A" & res$lat != "NA")
    return (res [indx, ])
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
    name <- noquote (gsub ("'", "", sysdata$stations_dc$name)) #nolint
    name <- trimws (name, which = 'right') # trim terminal white space
    res <- data.frame (id = sysdata$stations_dc$id,
                       name = name,
                       lon = sysdata$stations_dc$lon,
                       lat = sysdata$stations_dc$lat,
                       stringsAsFactors = FALSE)

    return (res)
}

#' Get Guadalajara stations
#'
#' @return \code{data.frame} of (id, name, lon, lat) of all stations in
#' Gaudalajara's mibici system
#'
#' @noRd
bike_get_gu_stations <- function ()
{
    link <- "https://www.mibici.net/site/assets/files/1118/nomenclatura_1.csv"
    suppressMessages (
                      dat <- httr::GET (link) %>%
                          httr::content (encoding = 'UTF-8')
                      )

    # Remove apostrophes from names coz they muck up sqlite fields:
    nm <- gsub ("\"", "", dat$name)
    nm <- gsub ("\'", "", nm)
    res <- data.frame (id = dat$id, name = nm,
                       lon = dat$longitude, lat = dat$latitude,
                       stringsAsFactors = FALSE)
    res <- res [which (!duplicated (res)), ]

    return (res)
}
