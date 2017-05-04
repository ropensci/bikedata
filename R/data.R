#' Test data for all 6 cities
#'
#' A data set containing for each of the six cities a \code{data.frame} object
#' of 200 trips.
#'
#' @docType data
#'
#' @format A list of one data frame for each of the five cities of (bo, dc, la,
#' lo, ny), plus two more for chicago stations and trips (ch_st, ch_tr). Each of
#' these (except 'ch_st') contains 200 representative trips.
#'
#' @note These data are only used to convert to \code{.zip}-compressed files
#' using \code{bike_write_test_data()}. These \code{.zip} files can be
#' subsequently read into an SQLite3 database using \code{store_bikedata}.
"bike_test_data"
