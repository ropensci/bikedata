context ("database stats")

require (testthat)

bikedb <- system.file ('db', 'testdb.sqlite', package = 'bikedata')

test_that ('can not read all data', {
               expect_error (trips <- bike_tripmat (bikedb),
                             'Calls to bike_tripmat must specify city')
               expect_error (trips <- bike_tripmat (bikedb, city = 'aa'),
                             'city not recognised')
})

test_that ('dplyr read db', {
               db <- RSQLite::dbConnect (RSQLite::SQLite(), bikedb,
                                         create = FALSE)
               qry <- "SELECT * FROM trips"
               trips <- RSQLite::dbGetQuery (db, qry)
               RSQLite::dbDisconnect (db)

               expect_equal (dim (trips), c (1198, 11))
               nms <- c ("id", "city", "trip_duration", "start_time",
                         "stop_time", "start_station_id", "end_station_id",
                         "bike_id", "user_type", "birth_year", "gender")
               expect_equal (names (trips), nms)
})

test_that ('latest files', {
               x <- bike_latest_files (bikedb)
               expect_true (all (!x))
               expect_equal (length (x), 6)
})

test_that ('date limits', {
               x <- bike_datelimits (bikedb)
               expect_is (x, 'character')
               expect_length (x, 2)
})

test_that ('db stats', {
               db_stats <- bike_summary_stats (bikedb)
               expect_is (db_stats, 'data.frame')
               expect_is (db_stats, 'tbl')
               expect_is (db_stats, 'tbl_df')
               expect_equal (names (db_stats), c ('city', 'num_trips',
                                                  'num_stations',
                                                  'first_trip', 'last_trip',
                                                  'latest_files'))
               expect_equal (dim (db_stats), c (7, 6))
               expect_true (all (db_stats$city == c ('total', 'bo', 'ch', 'dc',
                                                     'la', 'lo', 'ny')))
               expect_true (sum (db_stats$num_trips) == 2396)
               expect_true (sum (db_stats$num_stations) == (2 * 2192))
})
