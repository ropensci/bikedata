context ("tripmat")

test_that ('tripmat-full', {
               store_bikedata ("..", "testdb")
               expect_silent (tm <- tripmat ("testdb", quiet=TRUE))
               expect_equal (dim (tm), c (9, 9))
               expect_equal (sum (tm), 81)
               invisible (file.remove ("testdb"))
})

test_that ('tripmat-startday', {
               store_bikedata ("..", "testdb")
               expect_silent (tm <- tripmat ("testdb", start_date=20150102,
                                             quiet=TRUE))
               expect_equal (dim (tm), c (9, 9))
               expect_equal (sum (tm), 36)
               invisible (file.remove ("testdb"))
})

