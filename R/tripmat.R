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
    db <- dplyr::src_sqlite ("junk", create=F)
    trips <- dplyr::tbl (db, "trips")
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

    # Insert extra rows or columns in cases where xtabs does not yield a square
    # matrix. 
    if (nrow (ntrips) > ncol (ntrips))
    {
        indx <- sort (which (!rownames (ntrips) %in% colnames (ntrips)))
        for (i in indx)
            ntrips <- cbind (ntrips [,1:(i-1)], rep (0, nrow (ntrips)), 
                             ntrips [,i:ncol (ntrips)])
        colnames (ntrips) <- rownames (ntrips)
    } else if (ncol (ntrips) > nrow (ntrips))
    {
        indx <- sort (which (!colnames (ntrips) %in% rownames (ntrips)))
        for (i in indx)
            ntrips <- rbind (ntrips [1:(i-1),], rep (0, ncol (ntrips)), 
                             ntrips [i:nrow (ntrips),])
        rownames (ntrips) <- colnames (ntrips)
    }
    return (as (ntrips, 'matrix'))
}
