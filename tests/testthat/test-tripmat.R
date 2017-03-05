context ("tripmat")

test_that ('tripmat-full', {
               store_bikedata ("..", "testdb")
               expect_silent (tm <- tripmat ("testdb", quiet=TRUE))
               expect_equal (dim (tm), c (9, 9))
               invisible (file.remove ("testdb"))
})

test_that ('tripmat-startday', {
               store_bikedata ("..", "testdb")
               expect_silent (tm <- tripmat ("testdb", start_date=20141201,
                                             quiet=TRUE))
               expect_equal (dim (tm), c (9, 9))
               invisible (file.remove ("testdb"))
})

