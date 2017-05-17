#' convert hms to 'HH:MM:SS'
#'
#' @param x A numeric or character object to to be converted
#'
#' @return A string formatted to 'HH:MM:SS'
#'
#' @noRd
convert_hms <- function (x)
{
    if (is.numeric (x)) # presume it's HH
    {
        if (x < 0 | x > 24)
            stop ('hms values must be between 0 and 24')
        if (x < 24)
            res <- paste0 (sprintf ('%02d', x), ':00:00')
        else
            res <- paste0 (23, ':59:59')
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
                      }, numeric (1))
        if (any (is.na (wd)))
            stop ('weekday specification is ambiguous')
    } else if (any (!wd %in% 1:7))
        stop ('weekdays must be between 1 and 7')
    return (paste (sort (wd) - 1)) # sql is 0-indexed
}

#' convert date ranges to explicit vector
#'
#' @param dates
#'
#' @return Vector of converted data
#'
#' @note Trip data files are always named with year and month, yet different
#' systems do this differently. This function returns an appropriate vector of
#' dates for the nominated city.
#'
#' @noRd
convert_range_of_dates <- function (dates, city)
{
    dates <- as.character (dates)
    lens <- vapply (dates, nchar, FUN.VALUE = numeric (1))
    if (any (!lens %in% c (2, 4, 6, 7)))
        stop ('Cannot convert those kind of dates')
    if (length (unique (lens)) > 1)
        stop ('Dates should all be in the same format')

    # first get all lens in 7-character format "yyyy.mm"
    len <- unique (lens)
    if (len == 2) # years given as 2 digits only
        dates <- paste0 ('20', dates)
    if (len == 2 | len == 4)
    {
        dates <- unlist (lapply (dates, function (i)
                                 paste0 (i, ".", sprintf ("%02d", 1:12))))
    } else if (len == 6)
    {
        dates <- vapply (dates, function (i)
                         paste0 (substr (i, 1, 4), ".", substr (i, 5, 6)),
                         FUN.VALUE = character (1))
    }
}
