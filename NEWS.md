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
