bikedata
========

R package to load data from public bicycle hire systems. Currently a proof-of-concept that only loads data from the New York City [citibike scheme](https://www.citibikenyc.com/).

<!-- README.md is generated from README.Rmd. Please edit that file -->
[![Build Status](https://travis-ci.org/mpadge/bikedata.svg)](https://travis-ci.org/mpadge/bikedata) [![codecov](https://codecov.io/gh/mpadge/bikedata/branch/master/graph/badge.svg)](https://codecov.io/gh/mpadge/bikedata) [![Project Status: Concept - Minimal or no implementation has been done yet.](http://www.repostatus.org/badges/0.1.0/concept.svg)](http://www.repostatus.org/#concept) [![CRAN\_Status\_Badge](http://www.r-pkg.org/badges/version/bikedata)](http://cran.r-project.org/web/packages/bikedata)


-   [1 Installation](#installation)
-   [2 Usage](#usage)
    -   [2.1 Filtering trips by date](#filtering-trips-by-date)
    -   [2.2 Filtering trips by time of day](#filtering-trips-by-time-of-day)
    -   [2.3 Filtering trips by day of week](#filtering-trips-by-day-of-week)

------------------------------------------------------------------------

1 Installation
--------------

``` r
devtools::install_github("mpadge/bikedata")
```

------------------------------------------------------------------------

2 Usage
-------

Data from the NYC citibike system can can be downloaded with

``` r
dl_bikedata (data_dir='/tmp')
```

The resultant files can then be read into an `spatialite` database with

``` r
store_bikedata (data_dir='/tmp', 'spdb')
```

Numbers of trips between each pair of stations ('trip matrices') can then be obtained in square matrix form from

``` r
tmat <- tripmat ('spdb')
format (sum (tmat), big.mark=',')
```

    #> [1] "36,902,025"

The `spatialite` database of all trips is very large, and the `tripmat` function can take quite some time to execute (like several minutes).

``` r
dim (tmat)
```

    #> [1] 689 689

### 2.1 Filtering trips by date

Trip matrices can be constructed for trips filtered by dates and/or times. The temporal extent of the database can be readily viewed with

``` r
get_datelimits ('spdb')
```

    #>                 first                  last 
    #> "2013-07-01 00:00:00" "2017-01-19 08:08:17"

The `tripmat` function accepts four optional arguments specifying `start_date`, `end_date`, `start_time`, and `end_time`.

A trip matrix constructed from trips beginning after a given date with the `start_date` argument.

``` r
tmat <- tripmat ('spdb', start_date="20160101")
format (sum (tmat), big.mark=',')
```

    #> [1] "13,845,655"

As expected, that reduces numbers of trips. End dates can also be specified

``` r
tmat <- tripmat ('spdb', start_date=20140901, end_date="14/09/03")
format (sum (tmat), big.mark=',')
```

    #> [1] "89,508"

Note that dates can be specified in almost any format, as long as the order is `year-month-day`.

### 2.2 Filtering trips by time of day

Trips can also be selected starting and/or ending at specific times of day.

``` r
tmat <- tripmat ('spdb', start_time=0, end_time=1)
format (sum (tmat), big.mark=',')
```

    #> [1] "34,512"

Single values are interpeted to specify hours, so the above request returns only those rides start at or after midnight (`00:00:00`) and finishing prior to and including 1am (`01:00:00`). Single numeric values of `23` are interpreted as the end of a day (`23:59:59`).

Dates and times can of course be combined

``` r
tmat <- tripmat ('spdb', start_date=20150101, end_date="15,12,31",
                 start_time=6, end_time=10)
format (sum (tmat), big.mark=',')
```

    #> [1] "1,635,974"

### 2.3 Filtering trips by day of week

Trips can also extracted for particular days of the week by specifying the `weekday` argument of `tripmat`. Weekdays can be numeric, start from `1=Sunday`, or any unambiguous character string.

``` r
tmat <- tripmat ('spdb', weekday=7)
format (sum (tmat), big.mark=',')
```

    #> [1] "4,469,250"

Several weekdays can also be selected

``` r
tmat <- tripmat ('spdb', weekday=c('m', 'Th'))
format (sum (tmat), big.mark=',')
```

    #> [1] "10,923,683"

Weekdays can also be specified along with other dates and times

``` r
tmat <- tripmat ('spdb', weekday=2:6, start_time=6, end_time=10)
format (sum (tmat), big.mark=',')
```

    #> [1] "6,333,545"
