context ("daily trips")

require (testthat)

#bikedb <- file.path (getwd (), "testdb")
bikedb <- system.file ('db', 'testdb.sqlite', package = 'bikedata')

test_that ('no db arg', {
               expect_error (bike_daily_trips (),
                             "Can't get daily trips if bikedb isn't provided")
})

test_that ('db does not exist', {
               expect_error (bike_daily_trips (a), "object 'a' not found")
               expect_error (bike_daily_trips ('a'), 'file a does not exist')
               expect_error (bike_daily_trips (a = 'a'), 'unused argument')
               expect_error (bike_daily_trips (bikedb = 'a'),
                             'file a does not exist')
})

test_that ('no city', {
               expect_error (bike_daily_trips (bikedb),
                     "bikedb contains multiple cities; please specify one")
})

test_that ('daily trips', {
               expect_equal (bike_daily_trips (bikedb = bikedb,
                                               city = 'ny')$ntrips, 200)
               expect_equal (bike_daily_trips (bikedb = bikedb,
                                               city = 'ny',
                                               member = TRUE)$ntrips, 191)
               expect_equal (bike_daily_trips (bikedb = bikedb, city = 'ny',
                                               gender = 'f')$ntrips, 22)
               expect_equal (bike_daily_trips (bikedb = bikedb, city = 'ny',
                                               station = '173',
                                               gender = 1)$ntrips, 1)
})
