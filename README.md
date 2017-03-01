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
dl_bikedata (data_dir='/tmp')
```

The resultant files can then be read into an `spatialite` database with

``` r
store_bikedata_spl (data_dir='/tmp', 'spdb')
```

Numbers of trips between each pair of stations ('trip matrices') can then be obtained in square matrix form from

``` r
tmat <- tripmat ('spdb')
format (sum (tmat), big.mark=',')
```

    #> [1] "2,886,218"

(This is only a small selection of all data.)

``` r
dim (tmat)
```

    #> [1] 330 330

### Filtering trips

Trip matrices can be constructed for trips filtered by dates and/or times. The temporal extent of the database can be readily viewed with

``` r
get_datelimits ('spdb')
```

    #>                 first                  last 
    #> "2014-07-01 00:00:04" "2014-11-16 08:50:59"

The `tripmat` function accepts four optional arguments specifying `start_date`, `end_date`, `start_time`, and `end_time`.

#### Filtering trips by date

A trip matrix constructed from trips beginning after a given date with the `start_date` argument.

``` r
tmat <- tripmat ('spdb', start_date="20140901")
format (sum (tmat), big.mark=',')
```

    #> [1] "953,887"

As expected, that reduces numbers of trips. End dates can also be specified

``` r
tmat <- tripmat ('spdb', start_date=20140901, end_date="14/09/03")
format (sum (tmat), big.mark=',')
```

    #> [1] "89,508"

Note that dates can be specified in almost any format, as long as the order is `year-month-day`.

#### Filtering trips by time of day

Trips can also be selected starting and/or ending at specific times of day.

``` r
tmat <- tripmat ('spdb', start_time=0, end_time=1)
format (sum (tmat), big.mark=',')
```

    #> [1] "34,508"

Single values are interpeted to specify hours, so the above request returns only those rides start at or after midnight (`00:00:00`) and finishing prior to and including 1am (`01:00:00`). Single numeric values of `23` are interpreted as the end of a day (`23:59:59`).

Dates and times can of course be combined

``` r
tmat <- tripmat ('spdb', start_date=20140901, end_date="14,09,01",
                 start_time=6, end_time=9)
format (sum (tmat), big.mark=',')
```

    #> [1] "354,574"
