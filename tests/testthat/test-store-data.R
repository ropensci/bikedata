context ("write and store data in db")

require (testthat)

# test_all used to switch off tests on CRAN
test_all <- (identical (Sys.getenv ("MPADGE_LOCAL"), "true") |
             identical (Sys.getenv ("TRAVIS"), "true") |
             identical (Sys.getenv ("APPVEYOR"), "True"))

# NOTE: The direct routines used to create the database are not guaranteed to
# give identical results each time, particularly because stations for some
# cities are downloaded from API servers which may give variable numbers of
# stations, and even lead to variable numbers of trips. This function therefore
# tests the entire "live" API interface, while the remainder test the
# pre-generated database stored in /inst/db, which always generates reliable
# results.
#
# All files have 200 trips, but LA stations are read from trips, and has 2 trips
# that end at station#3000 which has no lat-lon, so there are only 198 trips and
# 50 stations, rather than 200 and 51.
#
# DC stations are read from internal data; CH from unzipped station data file
# containing 581 stations; and LO from server delivering all 777 stations
#
# Final expected values (from "bike_summary_stats ()"):
# city  |   ntrips  |   nstations
# ------|-----------|-------------
#  BO   |   200     |   93
#  CH   |   200     |   581
#  DC   |   200     |   456
#  LA   |   198     |   50
#  LO   |   200     |   776
#  NY   |   200     |   233
# ------|-----------|-------------
# Total |   1198    |   2189
# London in particlar expands rapidly and so tests are all for values >= these.
# To ensure this is failsafe, tests for numbers of stations are simply
# >= 93 + 581 + 456 + 5 + 233 + 700 = 2113


test_that ('write and store data', {
               bikedb <- file.path (tempdir (), "testdb")
               expect_silent (bike_write_test_data (data_dir = tempdir ()))
               expect_silent (n <- store_bikedata (data_dir = tempdir (),
                                                   bikedb = bikedb,
                                                   quiet = TRUE))
               expect_true (file.exists (bikedb))
               expect_silent (index_bikedata_db (bikedb = bikedb))
               # some windows test machines do not allow file deletion, so
               # numbers of lines are incremented with each appveyor/CRAN matrix
               # test. The following is therefore >= rather than just ==
               #expect_equal (n, 1198)
               expect_true (n >= 1198)
})

test_that ('stations from downloaded data', {
               bikedb <- file.path (tempdir (), "testdb")
               st <- bike_stations (bikedb)
               if (test_all)
               {
                   # This sometimes fails on some cran windoze machines for some
                   # reason
                   expect_true (nrow (st) > 2113)
               }
})

test_that ('remove data', {
               expect_equal (bike_rm_test_data (data_dir = tempdir ()), 6)
})

bikedb <- file.path (tempdir (), "testdb")
chk <- tryCatch (file.remove (bikedb),
                 warning = function (w) NULL,
                 error = function (e) NULL)
