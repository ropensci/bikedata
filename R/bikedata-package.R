#' bikedata.
#'
#' @name bikedata
#' @docType package
#' @author Mark Padgham
#' @import RSQLite
#' @importFrom Rcpp evalCpp
#' @importFrom dplyr %>% collect filter group_by src_sqlite summarise sql tbl
#' @importFrom lubridate ymd
#' @importFrom methods as
#' @importFrom stats xtabs
#' @importFrom utils download.file unzip
#' @useDynLib bikedata
NULL
