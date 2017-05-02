context ("store data in db")

require (testthat)

test_that ('read and append data', {
               expect_silent (store_bikedata (data_dir = "..",
                                              bikedb = "testdb",
                                              quiet = TRUE))
               invisible (file.remove ("testdb"))
})

store_bikedata (data_dir = "..", bikedb = "testdb")
# NOTE:
# All files have 200 trips, but LA stations are read from trips, and has 2 trips
# that end at station#3000 which has no lat-lon, so there are only 198 trips and
# 50 stations, rather than 200 and 51.
#
# DC stations are read from internal data; CH from unzipped station data file
# containing 581 stations; and LO from server delivering all 777 stations
#
# Final expected values:
# city  |   ntrips  |   nstations
# ------|-----------|-------------
#  LA   |   198     |   50
#  DC   |   200     |   456
#  NY   |   200     |   233
#  BO   |   200     |   93
#  CH   |   200     |   581
#  LO   |   200     |   777
# ------|-----------|-------------
# Total |   1198    |   2190
test_that ('read data', {
               db <- dplyr::src_sqlite ('testdb', create = F)

               trips <- dplyr::collect (dplyr::tbl (db, 'trips'))
               expect_equal (dim (trips), c (1198, 11))
               nms <- c ("id", "city", "trip_duration", "start_time",
                         "stop_time", "start_station_id", "end_station_id",
                         "bike_id", "user_type", "birth_year", "gender")
               expect_equal (names (trips), nms)

               stns <- dplyr::collect (dplyr::tbl (db, 'stations'))
               expect_equal (dim (stns), c (2190, 6))
               expect_equal (names (stns), c ('id', 'city', 'stn_id', 'name',
                                              'longitude', 'latitude'))
})

test_that ('date limits', {
               x <- bike_datelimits ('testdb')
               expect_is (x, 'character')
               expect_length (x, 2)
})

test_that ('db stats', {
               db_stats <- bike_summary_stats ('testdb')
               expect_is (db_stats, 'data.frame')
               expect_equal (names (db_stats), c ('num_trips', 'num_stations',
                                                  'first_trip', 'last_trip',
                                                  'latest_files'))
               expect_equal (dim (db_stats), c (7, 5))
               expect_equal (rownames (db_stats), c ('all', 'bo', 'ch', 'dc',
                                                     'la', 'lo', 'ny'))
               expect_equal (sum (db_stats$num_trips), 2396)
               expect_equal (sum (db_stats$num_stations), 4380)
})

invisible (file.remove ("testdb"))
