#' Download and aggregate data from public bicycle hire systems
#'
#' Download data from all public bicycle hire systems which provide open data,
#' currently including 
#' \itemize{
#' \item Santander Cycles London, U.K.
#' \item citibike New York City NY, U.S.A.
#' \item Divvy Chicago IL, U.S.A.
#' \item Capital BikeShare Washingon DC, U.S.A.
#' \item Hubway Boston MA, U.S.A.
#' \item Metro Los Angeles CA, U.S.A.
#' }
#'
#' @section Download and store data:
#' \itemize{
#' \item \code{dl_bikedata} Download data for particular cities and dates
#' \item \code{store_bikedata} Store data in \code{SQLite3} database
#' }
#'
#' @section Sample data for testing package:
#' \itemize{
#' \item \code{bike_test_data} Description of test data included with package
#' \item \code{bike_write_test_data} Write test data to disk in form precisely
#' reflecting data provided by all systems
#' \item \code{bike_rm_test_data} Remove data written to disk with
#' \code{bike_write_test_data}
#' }
#'
#' @section Functions to aggregate trip data:
#' \itemize{
#' \item \code{bike_daily_trips} Aggregate daily time series of total trips
#' \item \code{bike_stations} Extract table detailing locations and names of
#' bicycle docking stations
#' \item \code{bike_tripmat} Extract aggregate counts of trips between all pairs
#' of stations within a given city
#' }
#'
#' @section Summary Statistics:
#' \itemize{
#' \item \code{bike_summary_stats} Overall quantitative summary of database
#' contents.  All of the following functions provide individual aspects of this
#' summary.
#' \item \code{bike_db_totals} Count total numbers of trips or stations, either
#' for entire database or a specified city.
#' \item \code{bike_datelimits} Return dates of first and last trips, either for
#' entire database or a specified city.
#' \item \code{bike_demographic_data} Simple table indicating which cities
#' include demographic parameters with their data
#' \item \code{bike_latest_files} Check whether files contained in database are
#' latest published versions
#' }
#'
#' @name bikedata
#' @docType package
#' @author Mark Padgham
#' @importFrom magrittr %>%
#' @importFrom Rcpp evalCpp
#' @useDynLib bikedata, .registration = TRUE
NULL
