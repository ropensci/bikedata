#' bikedata.
#'
#' @name bikedata
#' @docType package
#' @author Mark Padgham
#' @importFrom dplyr %>% collect src_sqlite tbl
#' @importFrom httr content GET
#' @importFrom lubridate ddays interval ymd
#' @importFrom methods as
#' @importFrom Rcpp evalCpp
#' @importFrom RSQLite dbBind dbClearResult dbConnect dbDisconnect
#' @importFrom RSQLite dbGetQuery dbSendQuery SQLite
#' @importFrom reshape2 dcast
#' @importFrom utils data menu read.csv tail type.convert unzip write.csv zip
#' @importFrom xml2 xml_children xml_find_all
#' @useDynLib bikedata
NULL
