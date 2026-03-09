<!-- README.md is generated from README.Rmd. Please edit that file -->

[![R build
status](https://github.com/ropensci/bikedata/workflows/R-CMD-check/badge.svg)](https://github.com/ropensci/bikedata/actions?query=workflow%3AR-CMD-check)
[![codecov](https://codecov.io/gh/ropensci/bikedata/branch/master/graph/badge.svg)](https://codecov.io/gh/ropensci/bikedata)
[![Project Status:
Active](http://www.repostatus.org/badges/latest/active.svg)](https://www.repostatus.org/)
[![CRAN_Status_Badge](https://www.r-pkg.org/badges/version/bikedata)](https://cran.r-project.org/package=bikedata)
[![CRAN
Downloads](https://cranlogs.r-pkg.org/badges/grand-total/bikedata?color=orange)](https://cran.r-project.org/package=bikedata)
[![](http://badges.ropensci.org/116_status.svg)](https://github.com/ropensci/software-review/issues/116)
[![status](https://joss.theoj.org/papers/10.21105/joss.00471/status.svg)](https://joss.theoj.org/papers/10.21105/joss.00471)

The `bikedata` package aims to enable ready importing of historical trip
data from all public bicycle hire systems which provide data, and will
be expanded on an ongoing basis as more systems publish open data.
Cities and names of associated public bicycle systems currently
included, along with numbers of bikes and of docking stations (from
[wikipedia](https://en.wikipedia.org/wiki/List_of_bicycle-sharing_systems#Cities)),
are

| City                           | Hire Bicycle System                                                   | Number of Bicycles | Number of Docking Stations |
|--------------------------------|-----------------------------------------------------------------------|--------------------|----------------------------|
| London, U.K.                   | [Santander Cycles](https://tfl.gov.uk/modes/cycling/santander-cycles) | 13,600             | 839                        |
| San Francisco Bay Area, U.S.A. | [Ford GoBike](https://www.fordgobike.com/)                            | 7,000              | 540                        |
| New York City NY, U.S.A.       | [citibike](https://www.citibikenyc.com/)                              | 7,000              | 458                        |
| Chicago IL, U.S.A.             | [Divvy](https://www.divvybikes.com/)                                  | 5,837              | 576                        |
| Montreal, Canada               | [Bixi](https://bixi.com/)                                             | 5,220              | 452                        |
| Washingon DC, U.S.A.           | [Capital BikeShare](https://www.capitalbikeshare.com/)                | 4,457              | 406                        |
| Guadalajara, Mexico            | [mibici](https://www.mibici.net/)                                     | 2,116              | 242                        |
| Minneapolis/St Paul MN, U.S.A. | [Nice Ride](https://www.niceridemn.com/)                              | 1,833              | 171                        |
| Boston MA, U.S.A.              | [Hubway](https://www.bluebikes.com/)                                  | 1,461              | 158                        |
| Philadelphia PA, U.S.A.        | [Indego](https://www.rideindego.com)                                  | 1,000              | 105                        |
| Los Angeles CA, U.S.A.         | [Metro](https://bikeshare.metro.net/)                                 | 1,000              | 65                         |

These data include the places and times at which all trips start and
end. Some systems provide additional demographic data including years of
birth and genders of cyclists. The list of cities may be obtained with
the `bike_cities()` functions, and details of which include demographic
data with `bike_demographic_data()`.

The following provides a brief overview of package functionality. For
more detail, see the
[vignette](https://docs.ropensci.org/bikedata/articles/bikedata.html).

------------------------------------------------------------------------

## 1 Installation

Currently a development version only which can be installed with the
following command,

``` r
devtools::install_github("ropensci/bikedata")
```

and then loaded the usual way

``` r
library (bikedata)
```

## 2 Usage

Data may downloaded for a particular city and stored in an `SQLite3`
database with the simple command,

``` r
store_bikedata (city = 'nyc', bikedb = 'bikedb', dates = 201601:201603)
# [1] 2019513
```

where the `bikedb` parameter provides the name for the database, and the
optional argument `dates` can be used to specify a particular range of
dates (Jan-March 2016 in this example). The `store_bikedata` function
returns the total number of trips added to the specified database. The
primary objects returned by the `bikedata` packages are ‘trip matrices’
which contain aggregate numbers of trips between each pair of stations.
These are extracted from the database with:

``` r
tm <- bike_tripmat (bikedb = 'bikedb')
dim (tm); format (sum (tm), big.mark = ',')
```

    #> [1] 518 518
    #> [1] "2,019,513"

During the specified time period there were just over 2 million trips
between 518 bicycle docking stations. Note that the associated databases
can be very large, particularly in the absence of `dates` restrictions,
and extracting these data can take quite some time.

Data can also be aggregated as daily time series with

``` r
bike_daily_trips (bikedb = 'bikedb')
```

    #> # A tibble: 87 x 2
    #>    date       numtrips
    #>    <chr>         <dbl>
    #>  1 2016-01-01    11172
    #>  2 2016-01-02    14794
    #>  3 2016-01-03    15775
    #>  4 2016-01-04    19879
    #>  5 2016-01-05    18326
    #>  6 2016-01-06    24922
    #>  7 2016-01-07    28215
    #>  8 2016-01-08    29131
    #>  9 2016-01-08    21140
    #> 10 2016-01-10    14481
    #> # … with 77 more rows

A summary of all data contained in a given database can be produced as

``` r
bike_summary_stats (bikedb = 'bikedb')
#>    num_trips num_stations          first_trip       last_trip latest_files
#> ny  2019513          518 2016-01-01 00:00    2016-03-31 23:59        FALSE
```

The final field, `latest_files`, indicates whether the files in the
database are up to date with the latest published files.

### 2.1 Filtering trips by dates, times, and weekdays

Trip matrices can be constructed for trips filtered by dates, days of
the week, times of day, or any combination of these. The temporal extent
of a `bikedata` database is given in the above `bike_summary_stats()`
function, or can be directly viewed with

``` r
bike_datelimits (bikedb = 'bikedb')
```

    #>              first               last 
    #> "2016-01-01 00:00" "2016-03-31 23:59"

Additional temporal arguments which may be passed to the `bike_tripmat`
function include `start_date`, `end_date`, `start_time`, `end_time`, and
`weekday`. Dates and times may be specified in almost any format, but
larger units must always precede smaller units (so years before months
before days; hours before minutes before seconds). The following
examples illustrate the variety of acceptable formats for these
arguments.

``` r
tm <- bike_tripmat ('bikedb', start_date = "20160102")
tm <- bike_tripmat ('bikedb', start_date = 20160102, end_date = "16/02/28")
tm <- bike_tripmat ('bikedb', start_time = 0, end_time = 1) # 00:00 - 01:00
tm <- bike_tripmat ('bikedb', start_date = 20160101, end_date = "16,02,28",
                 start_time = 6, end_time = 24) # 06:00 - 23:59
tm <- bike_tripmat ('bikedb', weekday = 1) # 1 = Sunday
tm <- bike_tripmat ('bikedb', weekday = c('m', 'Th'))
tm <- bike_tripmat ('bikedb', weekday = 2:6,
                    start_time = "6:30", end_time = "10:15:25")
```

### 2.2 Filtering trips by demographic characteristics

Trip matrices can also be filtered by demographic characteristics
through specifying the three additional arguments of `member`, `gender`,
and `birth_year`. `member = 0` is equivalent to `member = FALSE`, and
`1` equivalent to `TRUE`. `gender` is specified numerically such that
values of `2`, `1`, and `0` respectively translate to female, male, and
unspecified. The following lines demonstrate this functionality

``` r
sum (bike_tripmat ('bikedb', member = 0))
sum (bike_tripmat ('bikedb', gender = 'female'))
sum (bike_tripmat ('bikedb', weekday = 'sat', birth_year = 1980:1990,
                   gender = 'unspecified'))
```

### 3. Citation

``` r
citation ("bikedata")
#> 
#> To cite bikedata in publications use:
#> 
#>   Mark Padgham, Richard Ellison (2017). bikedata Journal of Open Source Software, 2(20). URL
#>   https://doi.org/10.21105/joss.00471
#> 
#> A BibTeX entry for LaTeX users is
#> 
#>   @Article{,
#>     title = {bikedata},
#>     author = {Mark Padgham and Richard Ellison},
#>     journal = {The Journal of Open Source Software},
#>     year = {2017},
#>     volume = {2},
#>     number = {20},
#>     month = {Dec},
#>     publisher = {The Open Journal},
#>     url = {https://doi.org/10.21105/joss.00471},
#>     doi = {10.21105/joss.00471},
#>   }
```

### 4. Code of Conduct

Please note that this project is released with a [Contributor Code of
Conduct](https://ropensci.org/code-of-conduct/). By contributing to this
project you agree to abide by its terms.

[![ropensci_footer](https://ropensci.org//public_images/github_footer.png)](https://ropensci.org/)

## Contributors


<!-- ALL-CONTRIBUTORS-LIST:START - Do not remove or modify this section -->
<!-- prettier-ignore-start -->
<!-- markdownlint-disable -->

All contributions to this project are gratefully acknowledged using the [`allcontributors` package](https://github.com/ropensci/allcontributors) following the [allcontributors](https://allcontributors.org) specification. Contributions of any kind are welcome!

### Code

<table>

<tr>
<td align="center">
<a href="https://github.com/mpadge">
<img src="https://avatars.githubusercontent.com/u/6697851?v=4" width="100px;" alt=""/>
</a><br>
<a href="https://github.com/ropensci/bikedata/commits?author=mpadge">mpadge</a>
</td>
<td align="center">
<a href="https://github.com/richardellison">
<img src="https://avatars.githubusercontent.com/u/10625733?v=4" width="100px;" alt=""/>
</a><br>
<a href="https://github.com/ropensci/bikedata/commits?author=richardellison">richardellison</a>
</td>
<td align="center">
<a href="https://github.com/tbuckl">
<img src="https://avatars.githubusercontent.com/u/98956?v=4" width="100px;" alt=""/>
</a><br>
<a href="https://github.com/ropensci/bikedata/commits?author=tbuckl">tbuckl</a>
</td>
<td align="center">
<a href="https://github.com/szymanskir">
<img src="https://avatars.githubusercontent.com/u/29870975?v=4" width="100px;" alt=""/>
</a><br>
<a href="https://github.com/ropensci/bikedata/commits?author=szymanskir">szymanskir</a>
</td>
<td align="center">
<a href="https://github.com/arfon">
<img src="https://avatars.githubusercontent.com/u/4483?v=4" width="100px;" alt=""/>
</a><br>
<a href="https://github.com/ropensci/bikedata/commits?author=arfon">arfon</a>
</td>
<td align="center">
<a href="https://github.com/graceli8">
<img src="https://avatars.githubusercontent.com/u/23415421?v=4" width="100px;" alt=""/>
</a><br>
<a href="https://github.com/ropensci/bikedata/commits?author=graceli8">graceli8</a>
</td>
<td align="center">
<a href="https://github.com/jimhester">
<img src="https://avatars.githubusercontent.com/u/205275?v=4" width="100px;" alt=""/>
</a><br>
<a href="https://github.com/ropensci/bikedata/commits?author=jimhester">jimhester</a>
</td>
</tr>


<tr>
<td align="center">
<a href="https://github.com/maelle">
<img src="https://avatars.githubusercontent.com/u/8360597?v=4" width="100px;" alt=""/>
</a><br>
<a href="https://github.com/ropensci/bikedata/commits?author=maelle">maelle</a>
</td>
<td align="center">
<a href="https://github.com/sckott">
<img src="https://avatars.githubusercontent.com/u/577668?v=4" width="100px;" alt=""/>
</a><br>
<a href="https://github.com/ropensci/bikedata/commits?author=sckott">sckott</a>
</td>
</tr>

</table>


### Issue Authors

<table>

<tr>
<td align="center">
<a href="https://github.com/krlmlr">
<img src="https://avatars.githubusercontent.com/u/1741643?u=caaf26641c159b84fe1b6d506f57fcea302a556c&v=4" width="100px;" alt=""/>
</a><br>
<a href="https://github.com/ropensci/bikedata/issues?q=is%3Aissue+author%3Akrlmlr">krlmlr</a>
</td>
<td align="center">
<a href="https://github.com/dpprdan">
<img src="https://avatars.githubusercontent.com/u/1423562?u=641a09e8d193d9a34951e623a97a8ab67e8bf3e4&v=4" width="100px;" alt=""/>
</a><br>
<a href="https://github.com/ropensci/bikedata/issues?q=is%3Aissue+author%3Adpprdan">dpprdan</a>
</td>
<td align="center">
<a href="https://github.com/MathieuMarauri">
<img src="https://avatars.githubusercontent.com/u/11965307?u=0b7cdf502b243cd3ecd9a0e186d036f618c0db1f&v=4" width="100px;" alt=""/>
</a><br>
<a href="https://github.com/ropensci/bikedata/issues?q=is%3Aissue+author%3AMathieuMarauri">MathieuMarauri</a>
</td>
<td align="center">
<a href="https://github.com/JorisKlingen">
<img src="https://avatars.githubusercontent.com/u/34268395?v=4" width="100px;" alt=""/>
</a><br>
<a href="https://github.com/ropensci/bikedata/issues?q=is%3Aissue+author%3AJorisKlingen">JorisKlingen</a>
</td>
<td align="center">
<a href="https://github.com/Robinlovelace">
<img src="https://avatars.githubusercontent.com/u/1825120?u=4b78d134ed1814b0677455f45d932b3b4a6ba3a5&v=4" width="100px;" alt=""/>
</a><br>
<a href="https://github.com/ropensci/bikedata/issues?q=is%3Aissue+author%3ARobinlovelace">Robinlovelace</a>
</td>
<td align="center">
<a href="https://github.com/tbdv">
<img src="https://avatars.githubusercontent.com/u/32847647?u=c794e81a62951dea29f88c223db578daff92f103&v=4" width="100px;" alt=""/>
</a><br>
<a href="https://github.com/ropensci/bikedata/issues?q=is%3Aissue+author%3Atbdv">tbdv</a>
</td>
<td align="center">
<a href="https://github.com/geospatialmik">
<img src="https://avatars.githubusercontent.com/u/75185080?v=4" width="100px;" alt=""/>
</a><br>
<a href="https://github.com/ropensci/bikedata/issues?q=is%3Aissue+author%3Ageospatialmik">geospatialmik</a>
</td>
</tr>


<tr>
<td align="center">
<a href="https://github.com/RichardBean">
<img src="https://avatars.githubusercontent.com/u/30681244?u=d98b23ecdb5fe6101b37c62262a03bc114f1fe2b&v=4" width="100px;" alt=""/>
</a><br>
<a href="https://github.com/ropensci/bikedata/issues?q=is%3Aissue+author%3ARichardBean">RichardBean</a>
</td>
</tr>

</table>


### Issue Contributors

<table>

<tr>
<td align="center">
<a href="https://github.com/pstraforelli">
<img src="https://avatars.githubusercontent.com/u/55718352?u=e1ad6ba0b0973d44108683c8b4f0ad8830a79036&v=4" width="100px;" alt=""/>
</a><br>
<a href="https://github.com/ropensci/bikedata/issues?q=is%3Aissue+commenter%3Apstraforelli">pstraforelli</a>
</td>
<td align="center">
<a href="https://github.com/complexgenome">
<img src="https://avatars.githubusercontent.com/u/5683969?v=4" width="100px;" alt=""/>
</a><br>
<a href="https://github.com/ropensci/bikedata/issues?q=is%3Aissue+commenter%3Acomplexgenome">complexgenome</a>
</td>
</tr>

</table>

<!-- markdownlint-enable -->
<!-- prettier-ignore-end -->
<!-- ALL-CONTRIBUTORS-LIST:END -->
