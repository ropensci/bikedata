<!-- README.md is generated from README.Rmd. Please edit that file -->
[![Build Status](https://travis-ci.org/mpadge/bikedata.svg)](https://travis-ci.org/mpadge/bikedata) [![Build status](https://ci.appveyor.com/api/projects/status/github/mpadge/bikedata?svg=true)](https://ci.appveyor.com/project/mpadge/bikedata) [![codecov](https://codecov.io/gh/mpadge/bikedata/branch/master/graph/badge.svg)](https://codecov.io/gh/mpadge/bikedata) [![Project Status: WIP](http://www.repostatus.org/badges/latest/wip.svg)](http://www.repostatus.org/#wip) [![CRAN\_Status\_Badge](http://www.r-pkg.org/badges/version/bikedata)](http://cran.r-project.org/web/packages/bikedata)

bikedata
================

-   [1 Installation](#installation)
-   [2 Usage](#usage)
    -   [2.1 Filtering trips by dates, times, and weekdays](#filtering-trips-by-dates-times-and-weekdays)
    -   [2.2 Filtering trips by demographic characteristics](#filtering-trips-by-demographic-characteristics)

The `bikedata` package aims to enable ready importing of historical trip data from all public bicycle hire systems which provide data, and will be expanded on an ongoing basis as more systems publish open data. Cities and names of associated public bicycle systems currently included, along with numbers of bikes and of docking stations, are:

| City                     | Hire Bicycle System                                                   | Number of Bicycles | Number of Docking Stations |
|--------------------------|-----------------------------------------------------------------------|--------------------|----------------------------|
| London, U.K.             | [Santander Cycles](https://tfl.gov.uk/modes/cycling/santander-cycles) | 13,600             | 839                        |
| New York City NY, U.S.A. | [citibike](https://www.citibikenyc.com/)                              | 7,000              | 458                        |
| Chicago IL, U.S.A.       | [Divvy](https://www.divvybikes.com/)                                  | 5,837              | 576                        |
| Washingon DC, U.S.A.     | [Capital BikeShare](https://www.capitalbikeshare.com/)                | 4,457              | 406                        |
| Boston MA, U.S.A.        | [Hubway](https://www.thehubway.com/)                                  | 1,461              | 158                        |
| Los Angeles CA, U.S.A.   | [Metro](https://bikeshare.metro.net/)                                 | 1,000              | 65                         |

These data include the places and times at which all trips start and end. Some systems provide additional demographic data including years of birth and genders of cyclists.

------------------------------------------------------------------------

1 Installation
--------------

Currently a development version only which can be installed with the following command.

``` r
devtools::install_github("mpadge/bikedata")
```

------------------------------------------------------------------------

2 Usage
-------

Data may downloaded for a particular city and stored in an `SQLite3` database with the simple command:

``` r
store_bikedata (city = 'nyc', bikedb = 'bikedb')
```

where the `bikedb` parameter provides the name for the database. The primary objects returned by the `bikedata` packages are 'trip matrices' which contain aggregate numbers of trips between each pair of stations. These can then be obtained with:

``` r
tm <- bike_tripmat ('bikedb')
dim (tm); format (sum (tm), big.mark=',')
```

    #> [1] 689 689
    #> [1] "36,902,025"

Note that the associated databases can be very large (in the above case, almost 37 million individual trips), and extracting these data can take quite some time.

A summary of all data contained in a given database can be produced as

``` r
bike_summary_stats (bikedb = 'bikedb')
#>    num_trips num_stations          first_trip       last_trip latest_files
#> ny  36902025          689 2014-11-01 00:00:11 2017-01-31 9:59        TRUE
```

The final field, `latest_files`, indicates whether the files in the database are up to date with the latest published files.

### 2.1 Filtering trips by dates, times, and weekdays

Trip matrices can be constructed for trips filtered by dates, days of the week, times of day, or any combination of these. The temporal extent of a `bikedata` database is given in the above `bike_summary_stats()` function, or can be directly viewed with

``` r
bike_datelimits (bikedb = 'bikedb')
```

    #>                 first                  last 
    #> "2013-07-01 00:00:00" "2017-01-19 08:08:17"

Additional temporal arguments which may be passed to the `bike_tripmat` function include `start_date`, `end_date`, `start_time`, `end_time`, and `weekday`. Dates and times may be specified in almost any format, but larger units must always precede smaller units (so years before months before days; hours before minutes before seconds). The following examples illustrate the variety of acceptable formats for these arguments.

``` r
tm <- bike_tripmat ('bikedb', start_date="20160101")
tm <- bike_tripmat ('bikedb', start_date=20140901, end_date="14/09/03")
tm <- bike_tripmat ('bikedb', start_time=0, end_time=1) # 00:00 - 01:00
tm <- bike_tripmat ('bikedb', start_date=20150101, end_date="15,12,31",
                 start_time=6, end_time=24) # 06:00 - 23:59
tm <- bike_tripmat ('bikedb', weekday=1) # 1 = Sunday
tm <- bike_tripmat ('bikedb', weekday=c('m', 'Th'))
tm <- bike_tripmat ('bikedb', weekday=2:6, start_time=6:30, end_time=10:15:25)
```

### 2.2 Filtering trips by demographic characteristics

(coming soon ...)
