#' Extract station-to-station distance matrix
#'
#' @param bikedb A string containing the path to the SQLite3 database.
#' If no directory specified, it is presumed to be in \code{tempdir()}.
#' @param city City for which tripmat is to be aggregated
#' @param expand Distances are calculated by routing through the OpenStreetMap
#' street network surrounding the bike stations, with the street network
#' expanded by this amount to ensure all stations can be connected.
#' @param long If FALSE, a square distance matrix of (num-stations,
#' num_stations) is returned; if TRUE, a long-format matrix of (stn-from,
#' stn-to, distance) is returned.
#' @param quiet If FALSE, progress is displayed on screen
#'
#' @return If \code{long = FALSE}, a square matrix of numbers of trips between
#' each station, otherwise a long-form data.frame with three columns of of
#' (start_station, end_station, num_trips)
#'
#' @note Distance matrices returned from \code{bike_distamat} use all stations
#' listed for a given system, while trip matrices extracted with
#' \link{bike_tripmat} will often have fewer stations because operational
#' station numbers commonly vary over time. The two matrices may be reconciled
#' with the \code{match_trips2dists} function, enabling then to be directly
#' compared.
#'
#' @export
bike_distmat <- function (bikedb, city, expand = 0.5,
                          long = FALSE, quiet = TRUE)
{
    if (missing (bikedb))
        stop ("Can't get trip matrix if bikedb isn't provided")

    bikedb <- check_db_arg (bikedb = bikedb)
    city <- check_city_arg (bikedb = bikedb, city = city)
    stns <- bike_stations (bikedb = bikedb, city = city)
    xy <- stns [, which (names (stns) %in% c ("longitude", "latitude"))]
    dmat <- dodgr_dists (from = xy, to = xy, quiet = quiet)
    rownames (dmat) <- colnames (dmat) <- 
        stns [, which (names (stns) == "stn_id")] [[1]]

    if (long)
    {
        dmat <- reshape2::melt (dmat,
                                id.vars = c (rownames (dmat), colnames (dmat)))
        colnames (dmat) <- c ("from", "to", "distance")
    }

    return (dmat)
}
