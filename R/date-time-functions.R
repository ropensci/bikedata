#' convert hms to 'HH:MM:SS'
#'
#' @param x A numeric or character object to to be converted
#'
#' @return A string formatted to 'HH:MM:SS'
#'
#' @noRd
convert_hms <- function (x)
{
    if (is.numeric (x))
    {
        if (nchar (x) <= 2) # presume it's HH
        {
            if (x < 0 | x > 24)
                stop ('hms values must be between 0 and 24')
            if (x < 24)
                res <- paste0 (sprintf ('%02d', x), ':00:00')
            else
                res <- paste0 (23, ':59:59')
        } else if (nchar (x) == 4)
        {
            res <- paste0 (substring (x, 1, 2), ':', substring (x, 3, 4),
                           ':00')
        } else if (nchar (x) == 6)
        {
            res <- paste0 (substring (x, 1, 2), ':', substring (x, 3, 4),
                           ':', substring (x, 5, 6))
        } else
            stop ('Unable to convert time value')
    } else if (is.character (x))
    {
        # split at all non-numeric characters
        x <- vapply (strsplit (x, '[^0-9]') [[1]], as.numeric, numeric (1))
        if (length (x) == 0)
            stop ('Can not convert to hms without numeric values')
        if (length (x) == 1)
        {
            if (x < 24)
                res <- paste0 (sprintf ('%02d', x [1]), ':00:00')
            else
                res <- paste0 (23, ':59:59')
        }
        else if (length (x) == 2)
            res <- paste0 (sprintf ('%02d', x [1]), ':',
                           sprintf ('%02d', x [2]), ':00')
        else if (length (x) == 3)
            res <- paste0 (sprintf ('%02d', x [1]), ':',
                           sprintf ('%02d', x [2]), ':',
                           sprintf ('%02d', x [2]))
        else
            warning ('only first 3 numeric components used to convert to hms')
    } else
        stop ('hms values must be either numeric or character')

    return (res)
}

#' convert ymd to 'YYYY-MM-DD'
#'
#' @param x A numeric or character object to to be converted
#'
#' @return A string formatted to 'YYYY-MM-DD'
#'
#' lubridate::ymd requires a day to be specified. This function just appends
#' days (and months where necessary) where they don't exist.
#'
#' @noRd
convert_ymd <- function (x)
{
    if (is.numeric (x)) # presume it's HH
    {
        if (nchar (x) == 2) # can only be YY
            x <- as.numeric (paste0 ('20', x, '0101'))
        else if (nchar (x) == 4) # Either YYYY or YYMM
        {
            if (substring (x, 1, 2) == '20')
                x <- as.numeric (paste0 (x, '0101'))
            else
                x <- as.numeric (paste0 ('20', x, '01'))
        } else if (nchar (x) == 6 & substring (x, 1, 2) == '20')
            x <- as.numeric (paste0 (x, '01'))
    } else
    {
        xsp <- strsplit (x, "[[:space:]]|[[:punct:]]") [[1]]
        if (length (xsp) == 1)
            x <- paste (c (xsp, '01', '01'), collapse = ' ')
        if (length (xsp) == 2)
            x <- paste (c (xsp, '01'), collapse = ' ')
    }

    paste0 (lubridate::ymd (x))
}

#' convert weekday vector to numbered weekdays
#'
#' @param wd Vector of numeric or character denoting weekdays
#'
#' @return Equivalent character vector of numbered weekdays
#'
#' @noRd
convert_weekday <- function (wd)
{
    if (!is.numeric (wd))
    {
        if (!is.character (wd))
            stop ("don't know how to convert weekdays of class ", class (wd))
        wdlist <- c ("sunday", "monday", "tuesday", "wednesday",
                     "thursday", "friday", "saturday")
        wd <- vapply (tolower (wd), function (i)
                      {
                          res <- grep (paste0 ("\\<", i), wdlist)
                          if (length (res) != 1)
                              res <- NA
                          return (res)
                      },
                      numeric (1))
        if (any (is.na (wd)))
            stop ('weekday specification is ambiguous')
    } else if (any (!wd %in% 1:7))
        stop ('weekdays must be between 1 and 7')
    return (paste (sort (wd) - 1)) # sql is 0-indexed
}

# ------ functions for converting "dates" arg of dl_bikedata

#' Paste "20" onto start of any 2-digit years
#'
#' @noRd
prepend_year <- function (x)
{
    if (any (nchar (x) == 2))
        x [which (nchar (x) == 2)] <- paste0 ('20', x [which (nchar (x) == 2)])
    return (x)
}

#' Paste Jan and Dec respectively on to first and last value of year vector
#'
#' @noRd
add_month_range <- function (x)
{
    x [1] <- paste0 (x [1], '01')
    x [2] <- paste0 (x [2], '12')
    return (x)
}

#' Convert arbitrary character or numeric month to standard two-digit format
#'
#' @noRd
convert_month <- function (x)
{
    if (is.numeric (x))
        x <- paste0 (x)
    if (!is.numeric (type.convert (x)))
    {
        x <- substring (tolower (x), 1, 3)
        x <- pmatch (x, tolower (month.abb))
    }
    if (any (nchar (x) == 1))
        x [which (nchar (x) == 1)] <- paste0 ('0', x [which (nchar (x) == 1)])

    return (x)
}

#' Expand start and end dates given as YYYYMM to sequential range
#'
#' @param x Vector of one or two values giving start and potential end dates as
#' YYYYMM
#'
#' @return Vector all all sequential months between start and end dates of x
#'
#' @noRd
expand_dates_to_range <- function (x)
{
    if (length (x) == 2)
    {
        if (identical (substring (x [1], 1, 4), substring (x [2], 1, 4)))
            x <- x [1]:x [2]
        else
        {
            yy <- unique (substring (x, 1, 4))
            yy <- yy [1]:yy [2]
            xstart <- paste0 (yy [1], substring (x [1], 5, 6))
            xstart_12 <- paste0 (yy [1], '12')
            xstart <- paste0 (as.numeric (xstart):as.numeric (xstart_12))
            xend_1 <- paste0 (tail (yy, 1), '01')
            xend <- paste0 (tail (yy, 1), substring (x [2], 5, 6))
            xend <- paste0 (as.numeric (xend_1):as.numeric (xend))
            xmid <- NULL
            if (length (yy) > 2)
            {
                ymid <- yy [2:(length (yy) - 1)]
                mm <- c (paste0 ('0', 1:9), paste0 (10:12))
                xmid <- vapply (ymid, function (i)
                                paste0 (i, mm), FUN.VALUE = character (12))
            }
            x <- c (xstart, xmid, xend)
        }
    }

    return (unique (x))
}


#' Convert vector of dates returned by \code{expand_dates_to_range} to
#' appropriate character format matching file names for designed city
#'
#' Different cities use different date formats for their data files. While
#' NY and Boston use simple "YYYYMM" formats, other cities (DC, LA, Chicago,
#' Philly) disseminate data quarterly or with corresponding file names. London
#' is it's own unique case.
#'
#' @param x Vector of dates in YYYYMM format
#' @param city City for which dates to be matched
#'
#' @return Vector of YYYY_Q1-style date specifications to be matched against
#' file names for designated city
#'
#' @noRd
convert_dates_to_filenames <- function (x, city = 'ny')
{
    yy <- substring (x, 1, 4)
    if (city == 'ch')
    {
        # Chicago has 2013 bundled as single file, after which
        # YYYY_Q1Q2 or YYYY_Q3Q4
        indx13 <- which (grepl ('2013', paste0 (x)))
        indx <- which (!seq (x) %in% indx13)
        x <- x [indx]
        hh <- ceiling (as.numeric (substring (x, 5, 6)) / 6)
        hh [hh == 1] <- 'Q1Q2'
        hh [hh == 2] <- 'Q3Q4'
        x <- unique (paste0 (yy [indx], '_', hh))
        if (length (indx13) > 0)
            x <- c ('2013', x)
    } else if (city == 'lo')
    {
        mm <- month.abb [as.numeric (substring (x, 5, 6))]
        x <- paste0 (mm, yy)
    } else if (city %in% c ('dc', 'la', 'ph'))
    {
        # LA uses both "YYYY_QX" and "QX_YYYY"
        qq <- paste0 ('Q', ceiling (as.numeric (substring (x, 5, 6)) / 3))
        if (city == 'dc')
            x <- unique (paste0 (yy, '_', qq))
        else
            x <- unique (c (paste0 (yy, '_', qq), paste0 (qq, '_', yy)))
    } else
        x <- paste0 (x)

    return (x)
}


#' Convert dates argument for dl_bikedata to single start and end values in
#' YYYYMM format.
#'
#' @param dates Specified range of dates in almost any format
#'
#' @return Vector of one or two YYYYMM values
#'
#' @noRd
bike_convert_dates <- function (dates)
{
    if (is.numeric (dates))
    {
        if (length (dates) > 2)
            dates <- c (dates [1], tail (dates, 1))
        if (length (unique (nchar (dates))) > 1)
            stop ('Ambiguous dates format')
        if (all (nchar (dates) == 2))
            dates <- 200000 + 100 * dates + c (1, 12)
        else if (all (nchar (dates) == 4))
            dates <- 100 * dates + c (1, 12)
    } else
    {
        dates <- strsplit (dates, "[[:space:]]|[[:punct:]]") [[1]]
        if (length (dates) > 4)
            stop ('Cannot determine date range')
        if (length (dates) == 1)
        {
            if (nchar (dates) < 6)
                dates <- add_month_range (rep (prepend_year (dates), 2))
        } else if (length (dates) == 2)
        {
            # either range of years or year + month
            if (all (nchar (dates) == 2))
            {
                if (as.numeric (dates [2]) > 12) # try year-year
                    dates <- add_month_range (prepend_year (dates))
                else # try single year-month
                    dates <- paste0 (prepend_year (dates [1]),
                                     convert_month (dates [2]))
            } else if (all (nchar (dates) == 4)) # presume year-year
                dates <- add_month_range (dates)
            else if (!all (nchar (dates) == 6)) # presume year + month
                dates <- paste0 (prepend_year (dates [1]),
                                 convert_month (dates [2]))
        } else if (length (dates) == 3)
        {
            # presume year + month-month
            dates [1] <- prepend_year (dates [1])
            dates <- c (paste0 (dates [1], convert_month (dates [2])),
                        paste0 (dates [1], convert_month (dates [3])))
        } else
        {
            # length  == 4: year-month year-month
            dates [c (1, 3)] <- prepend_year (dates [c (1, 3)])
            dates [c (2, 4)] <- convert_month (dates [c (2, 4)])
            dates <- c (paste0 (dates [1], dates [2]),
                        paste0 (dates [3], dates [4]))
        }
    }

    return (as.numeric (dates))
}
