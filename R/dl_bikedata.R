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
#' @export
dl_bikedata <- function(city='nyc', data_dir = tempdir()){
    # TODO: 13:15 are there for testing purposes only - remove later!
    for (f in citibike_files ()[13:15]){
        destfile_zip <- file.path(data_dir, basename(f))
        destfile_csv <- paste0 (tools::file_path_sans_ext(destfile_zip), '.csv')
        if (!file.exists (destfile_csv))
        {
            if (!file.exists (destfile_zip))
                download.file (f, destfile_zip)
            unzip (destfile_zip, exdir=data_dir)
            file.remove (destfile_zip)
        }
    }

    print(paste0('Data saved at: ', list.files(data_dir, pattern = 'csv',
                                               full.names = TRUE)))
    invisible (list.files(data_dir, pattern='.csv', full.names=TRUE))
}

#' Estalish postgres database for nyc-citibike data
#'
#' @param data_dir Directory to which to download the files
#' @export
store_bikedata <- function(data_dir=tempdir()){
    #can't check with dl_bikedata because unzipped files are often named
    #differently to zip archives
    #flist <- dl_bikedata(data_dir=data_dir)
    flist <- list.files (data_dir, pattern='.csv')
    flist <- flist [grep ('citi', flist, ignore.case=TRUE)]
    flist <- sapply (flist, function (i) paste0 (data_dir, '/', i))
    chk <- system('createdb nyc-citibike-data')
    if (chk != 0)
        stop ('postgres database could not be created')

    system('psql nyc-citibike-data -f ./inst/sh/create_schema.sql')
    for (f in flist)
    {
        message('loading data for ', f, ' into postgres database ... ')
        system ('psql nyc-citibike-data -f ./inst/sh/create_raw.sql')
        sedcmd <- paste0 ("'s/\\\"//g; s/\\\\\\N//' \"", f, "\"")
                          #sedcmd <- paste0 ("'s/\\\\\\N//' \"", f, "\"")
                          cpycmd <- 'COPY trips_raw FROM stdin CSV HEADER;'
                          system (paste0 ("sed $", sedcmd, " | psql nyc-citibike-data -c \"",
                                          cpycmd, "\""))
                          message ('processing raw data ... ')
                          system ('psql nyc-citibike-data -f ./inst/sh/populate_trips_from_raw.sql')
    }
    message ('constructing final data tables ... ')
    system ('psql nyc-citibike-data -f ./inst/sh/prepare_tables.sql')
}

#' Store data in spatialite database
#'
#' 
#' @param datafiles A character vector containin the paths to the citibike 
#' .csv files to import.
#' @param spdb A string containing the path to the spatialite database to 
#' use. It will be created automatically.
#' @param quiet If FALSE, progress is displayed on screen
#'
#' @export
store_bikedata_spl <- function (datafiles, spdb, quiet=FALSE)
{
    ntrips <- importDataToSpatialite (datafiles, spdb, quiet)
    if (!quiet)
        message ('total trips read = ', 
                 format (ntrips, big.mark=',', scientific=FALSE))
}
