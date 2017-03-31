#' Download hire bicycle data
#'
#' @param city City for which to download bike data, or name of corresponding
#' bike system (see Details below).
#' @param data_dir Directory to which to download the files
#' @param dates Character vector of dates to download data with dates formated
#' as YYYYMM.
#'
#' @section Details:
#' This function produces zip-compressed data in R's temporary directory. City
#' names are not case sensitive, and must only be long enough to unambiguously
#' designate the desired city. Names of corresponding bike systems can also be
#' given.  Currently possible cities (with minimal designations in parentheses)
#' and names of bike hire systems are:
#' \tabular{lr}{
#'  New York City (ny)\tab Citibike\cr
#'  Washington, D.C. (dc)\tab Capital Bike Share\cr
#'  Chicago (ch)\tab Divvy Bikes\cr
#'  Los Angeles (la)\tab Metro Bike Share\cr
#'  Boston (bo)\tab Hubway\cr
#' }
#'
#' Ensure you have a fast internet connection and at least 100 Mb space
#'
#' @note Only files that don't already exist in \code{data_dir} will be
#' downloaded, and this function may thus be used to update a directory of files
#' by downloading more recent files.
#'
#' @export
dl_bikedata <- function(city = 'nyc', data_dir = tempdir(), dates)
{
    dl_files <- get_bike_files (city)
    files <- file.path (data_dir, basename (dl_files))

    if (missing (dates)) 
      indx <- which (!file.exists (files))
    else 
      indx <- which (!file.exists (files) & 
                     grepl (paste (dates, collapse = "|"), files))

    if (length (indx) > 0)
    {
        for (f in dl_files [indx])
        {
            destfile <- file.path (data_dir, basename(f))
            download.file (f, destfile)
        }
    } else
        message ('All data files already exist')
    invisible (list.files (data_dir, pattern = '.zip', full.names = TRUE))
}
