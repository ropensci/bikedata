context ("tripmat")

require (testthat)

store_bikedata (data_dir = "..", bikedb = "testdb")

test_that ('tripmat-full', {
               expect_message (tm <- bike_tripmat ("testdb", quiet = TRUE),
                               'Calls to tripmat should specify city')
               #expect_equal (dim (tm), c (2189, 2189))
               #expect_equal (sum (tm), 1196)
               expect_equal (dim (tm), c (831, 831))
               expect_equal (sum (tm), 798)
})

test_that ('tripmat-startday', {
               expect_silent (tm <- bike_tripmat ("testdb", city = 'ny',
                                                  start_date = 20161201,
                                                  quiet = TRUE))
               expect_equal (dim (tm), c (233, 233))
               expect_equal (sum (tm), 200)
})

test_that ('tripmat-endday', {
               expect_silent (tm <- bike_tripmat ("testdb", city = 'ny',
                                                  end_date = 20161201,
                                                  quiet = TRUE))
               expect_equal (dim (tm), c (233, 233))
               expect_equal (sum (tm), 200)
})

test_that ('tripmat-start-and-endday', {
               expect_silent (tm <- bike_tripmat ("testdb", city = 'ny',
                                                  start_date = 20161201,
                                                  end_date = 20161201,
                                                  quiet = TRUE))
               expect_equal (dim (tm), c (233, 233))
               expect_equal (sum (tm), 200)
})

test_that ('tripmat-starttime', {
               expect_silent (tm <- bike_tripmat ("testdb", city = 'ny',
                                                  start_time = 1,
                                                  quiet = TRUE))
               expect_equal (sum (tm), 77)
               expect_silent (tm2 <- bike_tripmat ("testdb", city = 'ny',
                                                   start_time = "1",
                                                   quiet = TRUE))
               expect_silent (tm3 <- bike_tripmat ("testdb", city = 'ny',
                                                   start_time = "01:00",
                                                   quiet = TRUE))
               expect_identical (tm, tm2)
               expect_identical (tm, tm3)
})

test_that ('tripmat-endtime', {
               expect_silent (tm <- bike_tripmat ("testdb", city = 'ny',
                                                  end_time = 1, quiet = TRUE))
               expect_equal (sum (tm), 140)
               expect_silent (tm2 <- bike_tripmat ("testdb", city = 'ny',
                                                   end_time = "1",
                                                   quiet = TRUE))
               expect_silent (tm3 <- bike_tripmat ("testdb", city = 'ny',
                                                   end_time = "1:00",
                                                   quiet = TRUE))
               expect_identical (tm, tm2)
               expect_identical (tm, tm3)
})

test_that ('tripmat-start-and-endtime', {
               expect_silent (tm <- bike_tripmat ("testdb", city = 'ny',
                                                  start_time = "00:00",
                                                  end_time = "01:00",
                                                  quiet = TRUE))
               expect_equal (sum (tm), 140)
})

test_that ('tripmat-starttime-startdate', {
               expect_silent (tm <- bike_tripmat ("testdb", city = 'ny',
                                                  start_date = 20161201,
                                                  start_time = 1,
                                                  quiet = TRUE))
               expect_equal (sum (tm), 77)
})

test_that ('tripmat-endtime-enddate', {
               expect_silent (tm <- bike_tripmat ("testdb", city = 'ny',
                                                  end_date = 20161201,
                                                  end_time = 1, quiet = TRUE))
               expect_equal (sum (tm), 140)
})

test_that ('tripmat-startendtime-startenddate', {
               expect_silent (tm <- bike_tripmat ("testdb", city = 'ny',
                                                  start_date = 20161201,
                                                  end_date = 20161201,
                                                  start_time = 1,
                                                  end_time = 2, quiet = TRUE))
               expect_equal (sum (tm), 77)
})

test_that ('weekday', {
               expect_silent (tm <- bike_tripmat ("testdb", city = 'ny',
                                                  weekday = 5, quiet = TRUE))
               expect_equal (sum (tm), 200)
               expect_silent (tm <- bike_tripmat ("testdb", city = 'ny',
                                                  weekday = c('f', 'sa', 'th'),
                                                  quiet = TRUE))
               expect_equal (sum (tm), 200)
               expect_error (tm <- bike_tripmat ("testdb", city = 'ny',
                                                 weekday = c('f', 'th', 's'),
                                                 quiet = T),
                             'weekday specification is ambiguous')
})

invisible (file.remove ("testdb"))
