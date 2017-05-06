context ("stations")

require (testthat)

is_cran <- identical (Sys.getenv ('NOT_CRAN'), 'false')

bikedb <- file.path (getwd (), "testdb")

test_that ('station data', {
               st <- bike_stations (bikedb)
               expect_true (nrow (st) >= 2190)
})
