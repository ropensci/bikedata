context ("store data in db")

require (testthat)

test_that ('read and append data', {
               expect_silent (store_bikedata ("..", "testdb", quiet=TRUE))
               expect_silent (store_bikedata ("..", "testdb", quiet=TRUE))
               invisible (file.remove ("testdb"))
})

store_bikedata ("..", "testdb")

test_that ('read data', {
               db <- dplyr::src_sqlite ('testdb', create=F)

               trips <- dplyr::collect (dplyr::tbl (db, 'trips'))
               expect_equal (dim (trips), c (162, 11))
               stns <- dplyr::collect (dplyr::tbl (db, 'stations'))
               expect_equal (nrow (stns), 9)
})

test_that ('date limits', {
               x <- get_datelimits ('testdb')
               expect_is (x, 'character')
               expect_length (x, 2)
})

test_that ('db size', {
               expect_equal (num_trips_in_db ('testdb'), 162)
})

invisible (file.remove ("testdb"))
