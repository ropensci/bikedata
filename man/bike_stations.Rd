% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/stations.R
\name{bike_stations}
\alias{bike_stations}
\title{Extract station matrix from SQLite3 database}
\usage{
bike_stations(bikedb, city)
}
\arguments{
\item{bikedb}{A string containing the path to the SQLite3 database.
If no directory specified, it is presumed to be in \code{tempdir()}.}

\item{city}{Optional city (or vector of cities) for which stations are to be
extracted}
}
\value{
Matrix containing data for each station
}
\description{
Extract station matrix from SQLite3 database
}
\examples{
\dontrun{
data_dir <- tempdir ()
bike_write_test_data (data_dir = data_dir)
# or download some real data!
# dl_bikedata (city = 'la', data_dir = data_dir)
bikedb <- file.path (data_dir, 'testdb')
store_bikedata (data_dir = data_dir, bikedb = bikedb)
# create database indexes for quicker access:
index_bikedata_db (bikedb = bikedb)

stations <- bike_stations (bikedb)
head (stations)

bike_rm_test_data (data_dir = data_dir)
bike_rm_db (bikedb)
# don't forget to remove real data!
# file.remove (list.files (data_dir, pattern = '.zip'))
}
}
