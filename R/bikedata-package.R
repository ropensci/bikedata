#' bikedata.
#'
#' @name bikedata
#' @docType package
#' @author Mark Padgham
#' @import RSQLite
#' @importFrom dplyr %>% collect src_sqlite tbl
#' @importFrom httr content GET
#' @importFrom lubridate ddays interval ymd
#' @importFrom methods as
#' @importFrom Rcpp evalCpp
#' @importFrom reshape2 dcast
#' @importFrom utils data menu read.csv unzip write.csv zip
#' @importFrom xml2 xml_children xml_find_all
#' @useDynLib bikedata
NULL
