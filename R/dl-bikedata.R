#' Download hire bicycle data
#'
#' @param city City for which to download bike data, or name of corresponding
#' bike system (see Details below).
#' @param data_dir Directory to which to download the files
#' @param dates Character vector of dates to download data with dates formated
#' as YYYYMM.
#' @param quiet If FALSE, progress is displayed on screen
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
#'  London (lo)\tab Santander Cycles\cr
#'  Philadelphia (ph)\tab Indego\cr
#' }
#'
#' Ensure you have a fast internet connection and at least 100 Mb space
#'
#' @note Only files that don't already exist in \code{data_dir} will be
#' downloaded, and this function may thus be used to update a directory of files
#' by downloading more recent files. If a particular file request fails,
#' downloading will continue regardless. To ensure all files are downloaded,
#' this function may need to be run several times until a message appears
#' declaring that 'All data files already exist'
#'
#' @export
#'
#' @examples
#' \dontrun{
#' dl_bikedata (city = 'New York City USA', dates = 201601:201613)
#' }
dl_bikedata <- function (city, data_dir = tempdir(), dates = NULL,
                         quiet = FALSE)
{
    if (missing (city))
        stop ('city must be specified for dl_bikedata()')

    city <- convert_city_names (city)

    dl_files <- get_bike_files (city)
    files <- file.path (data_dir, basename (dl_files))

    dates_exist <- TRUE # set to F is requested dates do not exist
    if (is.null (dates))
        indx <- which (!file.exists (files))
    else
    {
        dates <- bike_convert_dates (dates) %>%
            expand_dates_to_range %>%
            convert_dates_to_filenames (city = city)
        indx <- which (grepl (paste (dates, collapse = "|"), files))
        if (length (indx) == 0)
            dates_exist <- FALSE
        else
            indx <- which (!file.exists (files) &
                           grepl (paste (dates, collapse = "|"), files))
    }

    if (length (indx) > 0)
    {
        for (f in dl_files [indx])
        {
            destfile <- file.path (data_dir, basename(f))
            if (!quiet)
                message ('Downloading ', basename (f))
            resp <- httr::GET (f, httr::write_disk (destfile, overwrite = TRUE))
            if (resp$status_code != 200)
            {
                count <- 0
                while (!file.exists (destfile) & count < 5)
                {
                    resp <- httr::GET (f, httr::write_disk (destfile,
                                                            overwrite = TRUE))
                    count <- count + 1
                }
                if (!file.exists (destfile))
                    stop ('Download request failed')
            }
        }
    } else
    {
        if (!dates_exist)
            message ('There are no ', city, ' files for those dates')
        else
            message ('All data files already exist')
    }

    ptn <- '.zip'
    if (city == 'lo')
    {
        # London has raw csv files too, but sometimes the server delivers junk
        # data with default files that are very small. The following suffices to
        # remove these:
        csvs <- paste0 (data_dir, '/',
                        list.files (data_dir, pattern = '.csv'))
        indx <- which (file.info (csvs)$size < 1000)
        invisible (tryCatch (file.remove (csvs [indx]),
                             warning = function (w) NULL,
                             error = function (e) NULL))
        ptn <- paste0 (ptn, '|.csv')
    }

    invisible (list.files (data_dir, pattern = ptn, full.names = TRUE))
}
