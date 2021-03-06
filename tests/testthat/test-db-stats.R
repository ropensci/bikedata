context ("database stats")

test_all <- (identical (Sys.getenv ("MPADGE_LOCAL"), "true") |
             identical (Sys.getenv ("GITHUB_WORKFLOW"), "test-coverage"))

require (testthat)

bikedb <- system.file ("db", "testdb.sqlite", package = "bikedata")

test_that ("can not read all data", {
               expect_error (trips <- bike_tripmat (bikedb),
                         "bikedb contains multiple cities; please specify one")
               expect_error (trips <- bike_tripmat (bikedb, city = "aa"),
                             "city not recognised")
})

test_that ("dplyr read db", {
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

test_that ("latest files", {
               if (test_all) {

                   tryCatch (x <- bike_latest_files (bikedb),
                             warning = function (w) NULL,
                             error = function (e) NULL)
                   if (!is.null (x)) {

                       expect_true (all (!x))
                       expect_equal (length (x), 6)
                   }
               }
})

test_that ("date limits", {
               x <- bike_datelimits (bikedb)
               expect_is (x, "character")
               expect_length (x, 2)
})

test_that ("db stats", {
               if (test_all) { # summary_stats checks latest_files too

                   tryCatch (db_stats <- bike_summary_stats (bikedb),
                             warning = function (w) NULL,
                             error = function (e) NULL)
                   if (!is.null (db_stats)) {

                       expect_is (db_stats, "data.frame")
                       expect_is (db_stats, "tbl")
                       expect_is (db_stats, "tbl_df")
                       expect_equal (names (db_stats), c ("city",
                                                          "num_trips",
                                                          "num_stations",
                                                          "first_trip",
                                                          "last_trip",
                                                          "latest_files"))
                       expect_equal (dim (db_stats), c (7, 6))
                       expect_true (all (db_stats$city == c ("total", "bo",
                                                             "ch", "dc", "la",
                                                             "lo", "ny")))
                       expect_true (sum (db_stats$num_trips) == 2396)
                       expect_true (sum (db_stats$num_stations) == (2 * 2192))
                   }
               }
})
