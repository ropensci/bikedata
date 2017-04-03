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
