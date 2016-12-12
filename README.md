<!-- README.md is generated from README.Rmd. Please edit that file -->
[![Build Status](https://travis-ci.org/mpadge/bikedata.svg)](https://travis-ci.org/mpadge/bikedata) [![Project Status: Concept - Minimal or no implementation has been done yet.](http://www.repostatus.org/badges/0.1.0/concept.svg)](http://www.repostatus.org/#concept) [![CRAN\_Status\_Badge](http://www.r-pkg.org/badges/version/bikedata)](http://cran.r-project.org/web/packages/bikedata) ![downloads](http://cranlogs.r-pkg.org/badges/grand-total/bikedata)

bikedata
========

R package to load data from public bicycle hire systems. Currently a proof-of-concept that only loads data from the New York City [citibike scheme](https://www.citibikenyc.com/).

### Installation

``` r
devtools::install_github("mpadge/bikedata")
```

    #> Loading bikedata
    #> Updating bikedata documentation
    #> Loading bikedata

### Usage

``` r
library(bikedata)

# current verison
packageVersion("bikedata")
```

[citibike data](https://www.citibikenyc.com/system-data) first have to be downloaded:

``` r
dl_bikedata ()
trying URL 'https://s3.amazonaws.com/tripdata/201506-citibike-tripdata.zip'
Content type 'application/zip' length 22888858 bytes (21.8 MB)
===============================================
downloaded 21.8 MB

trying URL 'https://s3.amazonaws.com/tripdata/201507-citibike-tripdata.zip'
Content type 'application/zip' length 34518665 bytes (32.9 MB)
===============================================
downloaded 32.9 MB

trying URL 'https://s3.amazonaws.com/tripdata/201508-citibike-tripdata.zip'
Content type 'application/zip' length 38041594 bytes (36.3 MB)
===============================================
downloaded 36.3 MB

[1] "Data save at: /tmp/RmpaWphyb/201506-citibike-tripdata.zip"
[2] "Data save at: /tmp/RmpaWphyb/201507-citibike-tripdata.zip"
[3] "Data save at: /tmp/RmpaWphyb/201508-citibike-tripdata.zip"
```

And stored in a `postgres` database with

``` r
store_bikedata ()
```

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
