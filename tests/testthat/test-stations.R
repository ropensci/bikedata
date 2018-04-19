context ("stations")

require (testthat)

bikedb <- system.file ('db', 'testdb.sqlite', package = 'bikedata')


test_that ('bike_stations function', {
               st <- bike_stations (bikedb)
               expect_equal (names (st), c ('id', 'city', 'stn_id', 'name',
                                            'longitude', 'latitude'))
               expect_true (nrow (st) == 2192)
               expect_equal (length (unique (st$city)), 6)
               expect_equal (nrow (st [st$city == 'bo', ]), 93)
               expect_equal (nrow (st [st$city == 'ch', ]), 581)
               expect_equal (nrow (st [st$city == 'dc', ]), 456)
               expect_equal (nrow (st [st$city == 'la', ]), 50)
               expect_equal (nrow (st [st$city == 'lo', ]), 779)
               expect_equal (nrow (st [st$city == 'ny', ]), 233)
})

# test_all used to switch off tests on CRAN
test_local <- (identical (Sys.getenv ("MPADGE_LOCAL"), "true"))

# extra tests for other cities
test_that ('stations for extra cities', {
               if (test_local)
               {
                   st <- bike_get_gu_stations ()
                   expect_equal (ncol (st), 4)
                   expect_true (nrow (st) > 200) # currently 243

                   data_dir <- tempdir ()
                   flists <- list (flist_csv_stns = NULL)
                   st <- bike_get_bo_stations (flists, data_dir)
                   expect_equal (ncol (st), 4)
                   expect_true (nrow (st) > 200) # currently 300
               }
})
