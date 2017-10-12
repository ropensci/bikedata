0.0.4.99
===================
- New function `bike_distmat()` calculates distance matrices between all pairs
  of stations as routed through street networks for each city.
- Helper function `match_matrices()` matches distance and trip matrices by start
  and end stations, so they can be directly compared in standard statistical
  routines.
- North American Bike Share Association (NABSA) systems (currently LA and
  Philly) now distinguish member versus non-member based on whether usage is
  30-day pass or "Walk-up".

minor changes
- `dl_bikedata()` function also aliased to `download_bikedata()`, so both do the
  same job.
- Repeated runs of `store_bikedata()` on pre-existing databases sometimes
  re-added old data. This has now been fixed so only new data are added with
  each repeated call.
- Dates for NABSA cities (LA and Philadelphia) are given in different formats,
  all of which are now appropriately handled.


0.0.4
===================
- Database no longer automatically indexed; rather indexes must be actively
  generated with `index_bikedata_db()`. This makes multiple usages of
  `store_bikedata()` faster and easier.
- `store_bikedata()` fixed so it only unzips files not already in database (it
  used to unzip them all)
- Internal changes to improve consistency (mostly through using the DBI
  package).


0.0.3
===================
- Minor changes only
- More informative messages when data for specified dates not available

0.0.2
===================
- No change to package functionality
- Drop dplyr dependency after dplyr 0.7 upgrade

0.0.1 (31 May 2017)
===================
- Initial CRAN release
