context ("daily trips")

require (testthat)

bikedb <- file.path (getwd (), "testdb")

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
