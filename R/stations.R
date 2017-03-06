#' Extract station matrix from spatialite database
#'
#' @param spdb Path to the spatialite database 
#'
#' @return Matrix containing data for each station
#'
#' @export
bike_stations <- function (spdb)
{
    db <- dplyr::src_sqlite (spdb, create=F)
    dplyr::tbl (db, 'stations') 
}

