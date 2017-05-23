context ("stations")

require (testthat)

bikedb <- file.path (getwd (), "testdb")

test_that ('station db table', {
               db <- dplyr::src_sqlite (bikedb, create = F)
               stns <- dplyr::collect (dplyr::tbl (db, 'stations'))
               expect_equal (names (stns), c ('id', 'city', 'stn_id', 'name',
                                              'longitude', 'latitude'))
})

test_that ('bike_stations function', {
               st <- bike_stations (bikedb)
               #expect_true (nrow (st) >= 2178)
               expect_equal (length (unique (st$city)), 6)
               expect_equal (nrow (st [st$city == 'bo',]), 93)
               expect_equal (nrow (st [st$city == 'ch',]), 581)
               expect_equal (nrow (st [st$city == 'dc',]), 456)
               expect_equal (nrow (st [st$city == 'la',]), 50)
               # London stations constantly open and close so there is never a
               # constant number
               expect_true (nrow (st [st$city == 'lo',]) > 700)
               expect_equal (nrow (st [st$city == 'ny',]), 233)
})
