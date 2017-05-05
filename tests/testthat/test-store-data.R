context ("store data in db")

require (testthat)

test_that ('read and append data', {
               #bike_write_test_data ()
               bikedb <- file.path (getwd (), "testdb")
               expect_silent (store_bikedata (data_dir = "..",
                                              bikedb = bikedb,
                                              quiet = TRUE))
               #bike_rm_test_data ()
               invisible (file.remove (bikedb))
})

#bike_write_test_data ()
bikedb <- file.path (getwd (), "testdb")
store_bikedata (data_dir = "..", bikedb = bikedb)
# NOTE:
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
#  LO   |   200     |   778
#  NY   |   200     |   233
# ------|-----------|-------------
# Total |   1198    |   2191
# London in particlar expands rapidly and so tests are all for values >= these
test_that ('read data', {
               db <- dplyr::src_sqlite (bikedb, create = F)

               trips <- dplyr::collect (dplyr::tbl (db, 'trips'))
               expect_equal (dim (trips), c (1198, 11))
               nms <- c ("id", "city", "trip_duration", "start_time",
                         "stop_time", "start_station_id", "end_station_id",
                         "bike_id", "user_type", "birth_year", "gender")
               expect_equal (names (trips), nms)

               stns <- dplyr::collect (dplyr::tbl (db, 'stations'))
               #expect_equal (dim (stns), c (2191, 6))
               expect_true (nrow (stns) >= 2191)
               expect_equal (names (stns), c ('id', 'city', 'stn_id', 'name',
                                              'longitude', 'latitude'))
})

test_that ('date limits', {
               x <- bike_datelimits (bikedb)
               expect_is (x, 'character')
               expect_length (x, 2)
})

test_that ('db stats', {
               db_stats <- bike_summary_stats (bikedb)
               expect_is (db_stats, 'data.frame')
               expect_equal (names (db_stats), c ('num_trips', 'num_stations',
                                                  'first_trip', 'last_trip',
                                                  'latest_files'))
               expect_equal (dim (db_stats), c (7, 5))
               expect_equal (rownames (db_stats), c ('all', 'bo', 'ch', 'dc',
                                                     'la', 'lo', 'ny'))
               expect_equal (sum (db_stats$num_trips), 2396)
               expect_true (sum (db_stats$num_stations) >= 4382)
})

#bike_rm_test_data ()
invisible (file.remove (bikedb))
