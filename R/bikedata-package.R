#' bikedata.
#'
#' @name bikedata
#' @docType package
#' @author Mark Padgham
#' @import RSQLite 
#' @importFrom dplyr %>% collect filter group_by src_sqlite summarise sql tbl
#' @importFrom httr content GET
#' @importFrom lubridate ymd
#' @importFrom methods as
#' @importFrom Rcpp evalCpp
#' @importFrom reshape2 dcast
#' @importFrom stats xtabs
#' @importFrom utils download.file menu read.csv unzip
#' @importFrom xml2 xml_children xml_find_all
#' @useDynLib bikedata
NULL
