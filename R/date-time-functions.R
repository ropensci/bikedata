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
        x <- sapply (strsplit (x, '[^0-9]') [[1]], as.numeric)
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
        wd <- sapply (tolower (wd), function (i)
                      {
                          res <- grep (paste0 ("\\<", i), wdlist)
                          if (length (res) != 1)
                              res <- NA
                          return (res)
                      })
        if (any (is.na (wd)))
            stop ('weekday specification is ambiguous')
    } else if (any (!wd %in% 1:7))
        stop ('weekdays must be between 1 and 7')
    return (paste (sort (wd) - 1)) # sql is 0-indexed
}
