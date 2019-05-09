#' Download hire bicycle data
#'
#' Download data for subsequent storage via \link{store_bikedata}.
#'
#' @param city City for which to download bike data, or name of corresponding
#' bike system (see Details below).
#' @param data_dir Directory to which to download the files
#' @param dates Character vector of dates to download data with dates formated
#' as YYYYMM.
#' @param quiet If FALSE, progress is displayed on screen
#'
#' @section Details:
#' This function produces (generally) zip-compressed data in R's temporary
#' directory. City names are not case sensitive, and must only be long enough to
#' unambiguously designate the desired city. Names of corresponding bike systems
#' can also be given.  Currently possible cities (with minimal designations in
#' parentheses) and names of bike hire systems are:
#' \tabular{lr}{
#'  Boston (bo)\tab Hubway\cr
#'  Chicago (ch)\tab Divvy Bikes\cr
#'  Washington, D.C. (dc)\tab Capital Bike Share\cr
#'  Los Angeles (la)\tab Metro Bike Share\cr
#'  London (lo)\tab Santander Cycles\cr
#'  Minnesota (mn)\tab NiceRide\cr
#'  New York City (ny)\tab Citibike\cr
#'  Philadelphia (ph)\tab Indego\cr
#'  San Francisco Bay Area (sf)\tab Ford GoBike\cr
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
    if (city == 'mn')
        warning ('Data for the Nice Ride MN system must be downloaded ',
                 'manually from\nhttps://www.niceridemn.com/system-data/, and ',
                 'loaded using store_bikedata')

    dl_files <- get_bike_files (city)
    data_dir <- expand_home (data_dir) %>%
        check_data_dir () # check for existence and create if non-existent
    files <- file.path (data_dir, basename (dl_files))

    dates_exist <- TRUE # set to F if requested dates do not exist
    if (is.null (dates))
        indx <- which (!file.exists (files))
    else
    {
        dates <- bike_convert_dates (dates) %>%
            expand_dates_to_range () %>%
            convert_dates_to_filenames (city = city) %>%
            sort ()
        dates <- unique (c (dates, tolower (dates)))
        indx <- which (grepl (paste (dates, collapse = "|"), files,
                              ignore.case = TRUE))
        if (length (indx) == 0)
            dates_exist <- FALSE
        else
            indx <- which (!file.exists (files) &
                           grepl (paste (dates, collapse = "|"), files,
                                  ignore.case = TRUE))
    }

    if (length (indx) > 0)
    {
        for (f in dl_files [indx])
        {
            # replace whitespace in URLs (see issue#53)
            furl <- gsub (" ", "%20", f)
            f <- gsub (" ", "", f)
            destfile <- file.path (data_dir, basename(f))
            if (!quiet)
                message ('Downloading ', basename (f))
            resp <- httr::GET (furl,
                               httr::write_disk (destfile, overwrite = TRUE))
            if (resp$status_code != 200)
            {
                count <- 0
                while (!file.exists (destfile) & count < 5)
                {
                    resp <- httr::GET (furl,
                                       httr::write_disk (destfile,
                                                         overwrite = TRUE))
                    count <- count + 1
                }
                if (!file.exists (destfile))
                    stop ('Download request failed')
                # some junk files are also listed on AWS but not downloadable
                if (resp$status_code != 200 & file.exists (destfile))
                    chk <- file.remove (destfile)
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
        csvs <- file.path (data_dir, list.files (data_dir, pattern = '.csv'))
        indx <- which (file.info (csvs)$size < 1000)
        invisible (tryCatch (file.remove (csvs [indx]),
                             warning = function (w) NULL,
                             error = function (e) NULL))
        # There is also now one .xlxs file (#49, March 2017), which is converted
        # to .csv here
        xls <- file.path (data_dir, list.files (data_dir, pattern = '.xlsx'))
        for (f in xls)
        {
            fcsv <- paste0 (tools::file_path_sans_ext (f), ".csv")
            readxl::read_xlsx (f) %>%
                write.csv (, file = fcsv, row.names = FALSE)
            chk <- file.remove (f)
        }
        ptn <- paste0 (ptn, '|.csv')
    } else if (city == 'ny')
    {
        # ny also has some junk .zip files, but temporarily disabled because all
        # seems okay again now? (Oct 17)
        #zips <- file.path (data_dir, list.files (data_dir, pattern = '.zip'))
        #indx <- which (file.info (zips)$size < 1000)
        #invisible (tryCatch (file.remove (zips [indx]),
        #                     warning = function (w) NULL,
        #                     error = function (e) NULL))
    }

    ret <- list.files (data_dir, pattern = ptn, full.names = TRUE)
    invisible (ret [!grepl ("field_names", ret)])
}

#' @rdname dl_bikedata
#' @export
download_bikedata <- function (city, data_dir = tempdir(), dates = NULL,
                               quiet = FALSE)
{
    dl_bikedata (city = city, data_dir = data_dir, dates = dates, quiet = quiet)
}
