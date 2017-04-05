context ("store data in db")

require (testthat)

test_that ('read and append data', {
               expect_silent (store_bikedata (data_dir = "..",
                                              bikedb = "testdb",
                                              quiet = TRUE))
               invisible (file.remove ("testdb"))
})

store_bikedata (data_dir = "..", bikedb = "testdb")

test_that ('read data', {
               db <- dplyr::src_sqlite ('testdb', create = F)

               trips <- dplyr::collect (dplyr::tbl (db, 'trips'))
               expect_equal (dim (trips), c (162, 11))
               nms <- c ("id", "city", "trip_duration", "start_time",
                         "stop_time", "start_station_id", "end_station_id",
                         "bike_id", "user_type", "birth_year", "gender")
               expect_equal (names (trips), nms)

               stns <- dplyr::collect (dplyr::tbl (db, 'stations'))
               expect_equal (dim (stns), c (9, 6))
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
                                                  'first_trip', 'last_trip'))
               expect_equal (db_stats$num_trips, 162)
               expect_equal (db_stats$num_stations, 9)
})

invisible (file.remove ("testdb"))
