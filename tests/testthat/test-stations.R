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
