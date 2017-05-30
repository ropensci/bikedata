context ("tripmat")

require (testthat)

bikedb <- system.file ('db', 'testdb.sqlite', package = 'bikedata')
is_cran <- identical (Sys.getenv ("_R_CHECK_CRAN_INCOMING_"), 'true')

test_that ('no db', {
               errtxt <- paste0 ('bikedb ',
                                 file.path (tempdir (), 'junk'),
                                 ' does not exist')
               # The file.path construction does not give identical results on
               # windows machines
               #if (!is_cran)
               #    expect_error (tm <- bike_tripmat (bikedb = 'junk'), errtxt)
})

test_that ('tripmat-full', {
               expect_error (tm <- bike_tripmat (bikedb = bikedb, quiet = TRUE),
                               'Calls to bike_tripmat must specify city')
})

test_that ('tripmat-cities', {
               expect_equal (sum (bike_tripmat (bikedb = bikedb,
                                                city = 'bo')), 200)
               expect_equal (sum (bike_tripmat (bikedb = bikedb,
                                                city = 'ch')), 200)
               expect_equal (sum (bike_tripmat (bikedb = bikedb,
                                                city = 'dc')), 200)
               expect_equal (sum (bike_tripmat (bikedb = bikedb,
                                                city = 'la')), 198)
               expect_equal (sum (bike_tripmat (bikedb = bikedb,
                                                city = 'lo')), 193)
               expect_equal (sum (bike_tripmat (bikedb = bikedb,
                                                city = 'ny')), 200)
})

test_that ('tripmat-startday', {
               expect_silent (tm <- bike_tripmat (bikedb = bikedb, city = 'ny',
                                                  start_date = 20161201,
                                                  quiet = TRUE))
               expect_equal (dim (tm), c (233, 233))
               expect_equal (sum (tm), 200)
})

test_that ('tripmat-endday', {
               expect_silent (tm <- bike_tripmat (bikedb = bikedb, city = 'ny',
                                                  end_date = 20161201,
                                                  quiet = TRUE))
               expect_equal (dim (tm), c (233, 233))
               expect_equal (sum (tm), 200)
})

test_that ('tripmat-start-and-endday', {
               expect_silent (tm <- bike_tripmat (bikedb = bikedb, city = 'ny',
                                                  start_date = 20161201,
                                                  end_date = 20161201,
                                                  quiet = TRUE))
               expect_equal (dim (tm), c (233, 233))
               expect_equal (sum (tm), 200)
})

test_that ('tripmat-starttime', {
               expect_silent (tm <- bike_tripmat (bikedb = bikedb, city = 'ny',
                                                  start_time = 1,
                                                  quiet = TRUE))
               expect_equal (sum (tm), 77)
               expect_silent (tm2 <- bike_tripmat (bikedb = bikedb, city = 'ny',
                                                   start_time = "1",
                                                   quiet = TRUE))
               expect_silent (tm3 <- bike_tripmat (bikedb = bikedb, city = 'ny',
                                                   start_time = "01:00",
                                                   quiet = TRUE))
               expect_identical (tm, tm2)
               expect_identical (tm, tm3)
})

test_that ('tripmat-endtime', {
               expect_silent (tm <- bike_tripmat (bikedb = bikedb, city = 'ny',
                                                  end_time = 1, quiet = TRUE))
               expect_equal (sum (tm), 140)
               expect_silent (tm2 <- bike_tripmat (bikedb = bikedb, city = 'ny',
                                                   end_time = "1",
                                                   quiet = TRUE))
               expect_silent (tm3 <- bike_tripmat (bikedb = bikedb, city = 'ny',
                                                   end_time = "1:00",
                                                   quiet = TRUE))
               expect_identical (tm, tm2)
               expect_identical (tm, tm3)
})

test_that ('tripmat-start-and-endtime', {
               expect_silent (tm <- bike_tripmat (bikedb = bikedb, city = 'ny',
                                                  start_time = "00:00",
                                                  end_time = "01:00",
                                                  quiet = TRUE))
               expect_equal (sum (tm), 140)
})

test_that ('tripmat-starttime-startdate', {
               expect_silent (tm <- bike_tripmat (bikedb = bikedb, city = 'ny',
                                                  start_date = 20161201,
                                                  start_time = 1,
                                                  quiet = TRUE))
               expect_equal (sum (tm), 77)
})

test_that ('tripmat-endtime-enddate', {
               expect_silent (tm <- bike_tripmat (bikedb = bikedb, city = 'ny',
                                                  end_date = 20161201,
                                                  end_time = 1, quiet = TRUE))
               expect_equal (sum (tm), 140)
})

test_that ('tripmat-startendtime-startenddate', {
               expect_silent (tm <- bike_tripmat (bikedb = bikedb, city = 'ny',
                                                  start_date = 20161201,
                                                  end_date = 20161201,
                                                  start_time = 1,
                                                  end_time = 2, quiet = TRUE))
               expect_equal (sum (tm), 77)
})

test_that ('weekday', {
               expect_silent (tm <- bike_tripmat (bikedb = bikedb, city = 'ny',
                                                  weekday = 5, quiet = TRUE))
               expect_equal (sum (tm), 200)
               expect_silent (tm <- bike_tripmat (bikedb = bikedb, city = 'ny',
                                                  weekday = c('f', 'sa', 'th'),
                                                  quiet = TRUE))
               expect_equal (sum (tm), 200)
               expect_error (tm <- bike_tripmat (bikedb = bikedb, city = 'ny',
                                                 weekday = c('f', 'th', 's'),
                                                 quiet = T),
                             'weekday specification is ambiguous')
})

test_that ('tripmat-demography', {
               expect_silent (tm <- bike_tripmat (bikedb = bikedb, city = 'ny',
                                                  member = 1))
               expect_equal (sum (tm), 191)
               expect_silent (tm <- bike_tripmat (bikedb = bikedb, city = 'ny',
                                                  birth_year = 1976))
               expect_equal (sum (tm), 4)
               expect_silent (tm <- bike_tripmat (bikedb = bikedb, city = 'ny',
                                                  birth_year = 1976:1990))
               expect_equal (sum (tm), 98)
               expect_silent (tm <- bike_tripmat (bikedb = bikedb, city = 'ny',
                                                  gender = 'f'))
               expect_equal (sum (tm), 22)
               expect_silent (tm <- bike_tripmat (bikedb = bikedb, city = 'ny',
                                                  gender = 'm', 
                                                  birth_year = 1976:1990))
               expect_equal (sum (tm), 89)
})

