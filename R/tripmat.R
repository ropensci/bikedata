#' Render station-to-station matrix square by inserting extra rows or cols
#'
#' @param mat Station-to-station matrix of trip numbers
#'
#' @return Re-shaped version of mat that has equal numbers of rows and columns
reshape_tripmat <- function (mat)
{
    if (nrow (mat) > ncol (mat))
    {
        indx <- sort (which (!rownames (mat) %in% colnames (mat)))
        for (i in indx)
            mat <- cbind (mat [,1:(i-1)], rep (0, nrow (mat)), 
                          mat [,i:ncol (mat)])
        colnames (mat) <- rownames (mat)
    } else if (ncol (mat) > nrow (mat))
    {
        indx <- sort (which (!colnames (mat) %in% rownames (mat)))
        for (i in indx)
            mat <- rbind (mat [1:(i-1),], rep (0, ncol (mat)), 
                          mat [i:nrow (mat),])
        rownames (mat) <- colnames (mat)
    }
    return (mat)
}

#' Extract station-to-station trip matrix from spatialite database
#'
#' @param spdb Path to the spatialite database 
#' @param quiet If FALSE, progress is displayed on screen
#'
#' @return Square matrix of numbers of trips between each station
#'
#' @export
tripmat <- function (spdb, quiet=FALSE)
{
    db <- dplyr::src_sqlite ('junk', create=F)
    trips <- dplyr::tbl (db, 'trips')
    # suppress R CMD check notes:
    start_station_id <- end_station_id <- n <- NULL
    byid <- dplyr::group_by (trips, start_station_id, end_station_id)
    tripdb <- dplyr::summarise (byid, count=n())
    if (!quiet)
        message ('Counting numbers of trips from spatialite database ... ',
                 appendLF=FALSE)
    ntrips <- xtabs (count ~ start_station_id + end_station_id, data=tripdb)
    if (!quiet)
        message ('done')

    if (nrow (tripmat) != ncol (tripmat))
        tripmat <- reshape_tripmat (tripmat)

    return (as (ntrips, 'matrix'))
}

