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
#' each station, otherwise a long-form \pkg{tibble} with three columns of of
#' (start_station_id, end_station_id, distance)
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
    cols <- c ("longitude", "latitude", "stn_id")
    xy <- stns [, which (names (stns) %in% cols)] %>%
        remove_xy_outliers ()
    stn_id <- xy$stn_id # names for matrix
    xy <- xy [, which (names (xy) %in% cols [1:2])] # remove ID
    dmat <- dodgr::dodgr_dists (from = xy, to = xy, quiet = quiet)
    rownames (dmat) <- colnames (dmat) <- stn_id

    if (long)
    {
        dmat <- reshape2::melt (dmat,
                                id.vars = c (rownames (dmat), colnames (dmat)))
        colnames (dmat) <- c ("start_station_id", "end_station_id", "distance")
        dmat <- tibble::as_tibble (dmat)
    } else
    {
        attr (dmat, "variable") <- "distance" # used in match_matrices
    }

    return (dmat)
}

#' Some systems like Boston have outliers, presunably due to something like
#' humans mistyping a digit. These completely muck up distmat extraction, so are
#' removed here.
#' @noRd
remove_xy_outliers <- function (xy)
{
    xmn <- mean (xy$longitude)
    ymn <- mean (xy$latitude)
    d <- sqrt ( (xy$longitude - xmn) ^ 2 + (xy$latitude - ymn) ^ 2)
    dsd <- sd (c (-d, d))
    if (any (d > (10 * dsd)))
    {
        indx <- which (d < (10 * dsd))
        xy <- xy [indx, ]
    }
    return (xy)
}


#' Match rows and columns of distance and trip matrices
#'
#' @param mat1 A wide- or long-form trip or distance matrix returned from
#' \code{\link{bike_tripmat}} or \code{\link{bike_distmat}}.
#' @param mat2 The corresponding distance or trip matrix.
#'
#' @return A list of the same matrices with matching start and end stations, and
#' in the same order passed to the routine (that is, \code{mat1} then
#' \code{mat2}). Each kind of matrix will be identified and named accordingly as
#' either "trip" or "dist". Matrices are returned in same format (long or wide)
#' as submitted.
#'
#' @note Distance matrices returned from \code{bike_distamat} use all stations
#' listed for a given system, while trip matrices extracted with
#' \link{bike_tripmat} will often have fewer stations because operational
#' station numbers commonly vary over time. This function reconciles the two
#' matrices through matching all row and column names (or just station IDs for
#' long-form matrices), enabling then to be directly compared.
#'
#' @export
bike_match_matrices <- function (mat1, mat2)
{
    # convert both to wide form first
    long <- FALSE
    if (!nrow (mat1) == ncol (mat1))
    {
        mat1 <- long2wide (mat1)
        if (nrow (mat2) == ncol (mat2))
            message ("One matrix is long-form, the other is wide; ",
                     "will return both matrices in wide form")
        else
            long <- TRUE
    }
    if (!nrow (mat2) == ncol (mat2))
        mat2 <- long2wide (mat2)

    nms <- intersect (rownames (mat1), rownames (mat2))
    mat1 <- match_one_mat (mat1, nms, long = long)
    mat2 <- match_one_mat (mat2, nms, long = long)

    ret <- list (mat1, mat2)
    names (ret) <- c (is_trip_or_dist (mat1), is_trip_or_dist (mat2))
    return (ret)
}

#' match one trip or distance matrix to the \code{nms} common to both
#' The \code{long} param determines the return form, not the input form.
#' @noRd
match_one_mat <- function (mat, nms, long = FALSE)
{
    variable <- attr (mat, "variable")
    indx <- match (nms, rownames (mat))
    mat <- mat [indx, indx]
    if (!is.null (variable))
        attr (mat, "variable") <- variable

    if (long)
        mat <- bike_wide2long (mat) %>% tibble::as_tibble ()

    return (mat)
}

#' Determine whether matrix is trip or distance matrix
#' @noRd
is_trip_or_dist <- function (mat)
{
    variable <- "numtrips"
    if (nrow (mat) == ncol (mat))
    {
        variable <- attr (mat, "variable")
    } else
    {
        if ("distance" %in% names (mat))
            variable <- "distance"
    }
    if (variable == "distance")
        variable <- "dist"
    else
        variable <- "trip"

    return (variable)
}
