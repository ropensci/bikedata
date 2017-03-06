#' List of NYC citibike data files
#'
#' Ideally the list of file would be automatically extracted, but the files are
#' stored in an Amazon S3 bucket while requires special tools to read, all of
#' which require an authentification key to be set up. Present work-around is to
#' list them here manually, meaning they'll have to be updated on a monthly
#' basis.
citibike_files <- function(){
    c ('https://s3.amazonaws.com/tripdata/201307-citibike-tripdata.zip', 
       'https://s3.amazonaws.com/tripdata/201308-citibike-tripdata.zip', 
       'https://s3.amazonaws.com/tripdata/201309-citibike-tripdata.zip', 
       'https://s3.amazonaws.com/tripdata/201310-citibike-tripdata.zip', 
       'https://s3.amazonaws.com/tripdata/201311-citibike-tripdata.zip', 
       'https://s3.amazonaws.com/tripdata/201312-citibike-tripdata.zip', 
       'https://s3.amazonaws.com/tripdata/201401-citibike-tripdata.zip', 
       'https://s3.amazonaws.com/tripdata/201402-citibike-tripdata.zip', 
       'https://s3.amazonaws.com/tripdata/201403-citibike-tripdata.zip', 
       'https://s3.amazonaws.com/tripdata/201404-citibike-tripdata.zip', 
       'https://s3.amazonaws.com/tripdata/201405-citibike-tripdata.zip', 
       'https://s3.amazonaws.com/tripdata/201406-citibike-tripdata.zip', 
       'https://s3.amazonaws.com/tripdata/201407-citibike-tripdata.zip', 
       'https://s3.amazonaws.com/tripdata/201408-citibike-tripdata.zip', 
       'https://s3.amazonaws.com/tripdata/201409-citibike-tripdata.zip', 
       'https://s3.amazonaws.com/tripdata/201410-citibike-tripdata.zip', 
       'https://s3.amazonaws.com/tripdata/201411-citibike-tripdata.zip', 
       'https://s3.amazonaws.com/tripdata/201412-citibike-tripdata.zip', 
       'https://s3.amazonaws.com/tripdata/201501-citibike-tripdata.zip', 
       'https://s3.amazonaws.com/tripdata/201502-citibike-tripdata.zip', 
       'https://s3.amazonaws.com/tripdata/201503-citibike-tripdata.zip', 
       'https://s3.amazonaws.com/tripdata/201504-citibike-tripdata.zip', 
       'https://s3.amazonaws.com/tripdata/201505-citibike-tripdata.zip', 
       'https://s3.amazonaws.com/tripdata/201506-citibike-tripdata.zip', 
       'https://s3.amazonaws.com/tripdata/201507-citibike-tripdata.zip', 
       'https://s3.amazonaws.com/tripdata/201508-citibike-tripdata.zip', 
       'https://s3.amazonaws.com/tripdata/201509-citibike-tripdata.zip', 
       'https://s3.amazonaws.com/tripdata/201510-citibike-tripdata.zip', 
       'https://s3.amazonaws.com/tripdata/201511-citibike-tripdata.zip', 
       'https://s3.amazonaws.com/tripdata/201512-citibike-tripdata.zip', 
       'https://s3.amazonaws.com/tripdata/201601-citibike-tripdata.zip', 
       'https://s3.amazonaws.com/tripdata/201602-citibike-tripdata.zip', 
       'https://s3.amazonaws.com/tripdata/201603-citibike-tripdata.zip', 
       'https://s3.amazonaws.com/tripdata/201604-citibike-tripdata.zip', 
       'https://s3.amazonaws.com/tripdata/201605-citibike-tripdata.zip', 
       'https://s3.amazonaws.com/tripdata/201606-citibike-tripdata.zip', 
       'https://s3.amazonaws.com/tripdata/201607-citibike-tripdata.zip', 
       'https://s3.amazonaws.com/tripdata/201608-citibike-tripdata.zip', 
       'https://s3.amazonaws.com/tripdata/201609-citibike-tripdata.zip',
       'https://s3.amazonaws.com/tripdata/201610-citibike-tripdata.zip',
       'https://s3.amazonaws.com/tripdata/201611-citibike-tripdata.zip',
       'https://s3.amazonaws.com/tripdata/201612-citibike-tripdata.zip')
}


#' Download hire bicycle data
#'
#' @section Details:
#' This convenience function downloads hire bicycle data from the nominated
#' city.  It produces zip-compressed data in R's temporary directory.
#' Possible cities at present are:
#' \itemize{
#'  nyc = New York
#' }
#'
#' Ensure you have a fast internet connection and at least 100 Mb space
#'
#' @param city City for which to download bike data
#' @param data_dir Directory to which to download the files
#'
#' @note Only files that don't already exist in \code{data_dir} will be
#' downloaded, and this function may thus be used to update a directory of files
#' by downloading more recent files.
#'
#' @export
dl_bikedata <- function(city='nyc', data_dir = tempdir())
{
    files <- file.path (data_dir, basename (citibike_files ()))
    indx <- which (!file.exists (files))
    if (length (indx) > 0)
    {
        for (f in citibike_files () [indx])
        {
            destfile <- file.path (data_dir, basename(f))
            download.file (f, destfile)
        }
    } else
        message ('All data files already exist')
    invisible (list.files (data_dir, pattern='.zip', full.names=TRUE))
}

#' Store nyc-citibike data in spatialite database
#'
#' 
#' @param data_dir A character vector giving the directory containing the
#' \code{.zip} files of citibike data.
#' @param spdb A string containing the path to the spatialite database to 
#' use. It will be created automatically.
#' @param quiet If FALSE, progress is displayed on screen
#'
#' @note This function can take quite a long time to execute (typically > 10
#' minutes), and generates a spatialite database file several gigabytes in size.
#' 
#' @export
store_bikedata <- function (data_dir, spdb, quiet=FALSE)
{
    if (file.exists (spdb))
        stop ('File named ', spdb, ' already exists')

    # platform-independent unzipping is much easier in R
    flist <- file.path (data_dir, list.files (data_dir, pattern=".zip"))
    if (!quiet)
        message ('Extracting files ', appendLF=FALSE)
    for (f in flist)
    {
        fcsv <- unzip (f, list=TRUE)$Name
        if (!exists (fcsv))
            unzip (f, exdir=data_dir)
        if (!quiet)
            message ('.', appendLF=FALSE)
    }
    if (!quiet)
        message (' done')
    flist_csv <- file.path (data_dir, list.files (data_dir, pattern=".csv"))
    ntrips <- importDataToSpatialite (flist_csv, spdb, quiet)
    if (!quiet)
        message ('total trips read = ', 
                 format (ntrips, big.mark=',', scientific=FALSE))
    invisible (file.remove (flist_csv))
}
