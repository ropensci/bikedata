<!-- README.md is generated from README.Rmd. Please edit that file -->
[![Build Status](https://travis-ci.org/mpadge/bikedata.svg)](https://travis-ci.org/mpadge/bikedata) [![Project Status: Concept - Minimal or no implementation has been done yet.](http://www.repostatus.org/badges/0.1.0/concept.svg)](http://www.repostatus.org/#concept) [![CRAN\_Status\_Badge](http://www.r-pkg.org/badges/version/bikedata)](http://cran.r-project.org/web/packages/bikedata) 

bikedata
========

R package to load data from public bicycle hire systems. Currently a proof-of-concept that only loads data from the New York City [citibike scheme](https://www.citibikenyc.com/).

Installation
------------

``` r
devtools::install_github("mpadge/bikedata")
```

    #> Loading bikedata

Usage
-----

``` r
library(bikedata)

# current verison
packageVersion("bikedata")
```

### Rcpp

``` r
ptm <- proc.time ()
read_bikedata ()
#> There are 332 stations [max#=3003] and 3 trip files.
#> Reading file [ 0/3]: 201506-citibike-tripdata.csv with 941219 records
#> Reading file [ 1/3]: 201507-citibike-tripdata.csv with 1085676 records
#> Reading file [ 2/3]: 201508-citibike-tripdata.csv with 1179044 records
#> Total number of trips = 0
#> [1] 0
proc.time () - ptm
#>    user  system elapsed 
#>   9.316   0.556   9.872
```

**NOTE the time taken**

### postgres

[citibike data](https://www.citibikenyc.com/system-data) first have to be downloaded:

``` r
dl_bikedata ()
```

And stored in a `postgres` database with the following lines. The timing of these is directly comparable to the `Rcpp` version above.

``` r
ptm <- proc.time ()
store_bikedata (data_dir="/data/data/junk/csv")
#> [1] "Data saved at: /data/data/junk/csv/201506-citibike-tripdata.csv"
#> [2] "Data saved at: /data/data/junk/csv/201507-citibike-tripdata.csv"
#> [3] "Data saved at: /data/data/junk/csv/201508-citibike-tripdata.csv"
#> loading data for /data/data/junk/csv/201506-citibike-tripdata.csv into postgres database ...
#> processing raw data ...
#> loading data for /data/data/junk/csv/201507-citibike-tripdata.csv into postgres database ...
#> processing raw data ...
#> loading data for /data/data/junk/csv/201508-citibike-tripdata.csv into postgres database ...
#> processing raw data ...
#> constructing final data tables ...
proc.time () - ptm
#>    user  system elapsed 
#>   2.716   0.588  62.682
```

Note that `store_bikedata()` will also download the data if they don't already exist.

These data can then be accessed with the `RPostgreSQL` package:

``` r
library(RPostgreSQL)
#> Loading required package: DBI
drv <- dbDriver("PostgreSQL")
citibike_con = dbConnect(drv, dbname = "nyc-citibike-data")

query <- function(sql, con = citibike_con) 
  fetch(dbSendQuery(con, sql), n = 1e8)
get_ntrips <- function ()
{
    n <- query("SELECT * FROM station_to_station_counts")
    stns <- sort (unique (n$start_station_id))
    # transpose so index is [from, ti]
    n <- t (array (n$count, dim=rep (length (stns), 2)))
    rownames (n) <- colnames (n) <- stns
    return (n)
}
nt <- get_ntrips ()
junk <- dbDisconnect (citibike_con)
cat (format (sum (nt), big.mark=",", scientific=FALSE), 
     "trips between", dim (nt)[1], "stations\n")
#> 5,876,302 trips between 421 stations
```

(These numbers reflect only a select few of all possible files---there are far more data than that in total.)

At present the `postgres` database has to be manually removed with

``` r
system ("dropdb nyc-citibike-data")
```

### Test Results

``` r
library(bikedata)
library(testthat)

date()

test_dir("tests/")
```
