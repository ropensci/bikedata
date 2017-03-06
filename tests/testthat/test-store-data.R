context ("store data in db")

test_that ('read data', {
               expect_silent (store_bikedata ("..", "testdb", quiet=TRUE))
               invisible (file.remove ("testdb"))
})

store_bikedata ("..", "testdb")

test_that ('read data', {
               expect_error (store_bikedata ("../tests", "testdb"),
                             "File named testdb already exists")
               expect_true (file.exists ("testdb"))
               invisible (file.remove ("testdb"))
               expect_message (store_bikedata ("..", "testdb"))
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

invisible (file.remove ("testdb"))
