context ("tripmat")

require (testthat)

store_bikedata ("..", "testdb")

test_that ('tripmat-full', {
               expect_silent (tm <- tripmat ("testdb", quiet=TRUE))
               expect_equal (dim (tm), c (9, 9))
               expect_equal (sum (tm), 162)
})

test_that ('tripmat-startday', {
               expect_silent (tm <- tripmat ("testdb", start_date=20150102,
                                             quiet=TRUE))
               expect_equal (dim (tm), c (9, 9))
               expect_equal (sum (tm), 117)
})

test_that ('tripmat-endday', {
               expect_silent (tm <- tripmat ("testdb", end_date=20150102,
                                             quiet=TRUE))
               expect_equal (dim (tm), c (9, 9))
               expect_equal (sum (tm), 81)
})

test_that ('tripmat-start-and-endday', {
               expect_silent (tm <- tripmat ("testdb", start_date=20150102,
                                             end_date=20150103, quiet=TRUE))
               expect_equal (dim (tm), c (9, 9))
               expect_equal (sum (tm), 81)
})

test_that ('tripmat-starttime', {
               expect_silent (tm <- tripmat ("testdb", start_time=1,
                                             quiet=TRUE))
               expect_equal (sum (tm), 140)
               expect_silent (tm2 <- tripmat ("testdb", start_time="1",
                                             quiet=TRUE))
               expect_silent (tm3 <- tripmat ("testdb", start_time="01:00",
                                             quiet=TRUE))
               expect_identical (tm, tm2)
               expect_identical (tm, tm3)
})

test_that ('tripmat-endtime', {
               expect_silent (tm <- tripmat ("testdb", end_time=4,
                                             quiet=TRUE))
               expect_equal (sum (tm), 88)
               expect_silent (tm2 <- tripmat ("testdb", end_time="4",
                                             quiet=TRUE))
               expect_silent (tm3 <- tripmat ("testdb", end_time="4:00",
                                             quiet=TRUE))
               expect_identical (tm, tm2)
               expect_identical (tm, tm3)
})

test_that ('tripmat-start-and-endtime', {
               expect_silent (tm <- tripmat ("testdb", start_time="02:00",
                                             end_time="04:00", quiet=TRUE))
               expect_equal (sum (tm), 42)
})

test_that ('tripmat-starttime-startdate', {
               expect_silent (tm <- tripmat ("testdb", start_date=20150102,
                                             start_time=1, quiet=TRUE))
               expect_equal (sum (tm), 100)
})

test_that ('tripmat-endtime-enddate', {
               expect_silent (tm <- tripmat ("testdb", end_date=20150103,
                                             end_time=6, quiet=TRUE))
               expect_equal (sum (tm), 90)
})

test_that ('tripmat-startendtime-startenddate', {
               expect_silent (tm <- tripmat ("testdb", start_date=20150102,
                                             end_date=20150103, start_time=1,
                                             end_time=7, quiet=TRUE))
               expect_equal (sum (tm), 59)
})

test_that ('weekday', {
               expect_silent (tm <- tripmat ("testdb", weekday=5:6, quiet=TRUE))
               expect_equal (sum (tm), 81)
               expect_silent (tm <- tripmat ("testdb", weekday=c('f','sa'), 
                                             quiet=TRUE))
               expect_equal (sum (tm), 81)
               expect_error (tm <- tripmat ("testdb", weekday=c('f','s'), quiet=T),
                             'weekday specification is ambiguous')
})

invisible (file.remove ("testdb"))
