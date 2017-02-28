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

Usage
-----

Data from the NYC citibike system can can be downloaded with

``` r
dl_bikedata (data_dir='tmp/')
```

The resultant files can then be read into an `spatialite` database with

``` r
files <- paste0 ('tmp/', list.files ('tmp/'))
store_bikedata_spl (files, "junk")
```

Numbers of trips between each pair of stations can then be obtained in square matrix form from

``` r
tmat <- tripmat ("junk")
```
