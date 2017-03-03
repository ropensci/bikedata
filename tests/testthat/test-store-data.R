context ("store data in db")

test_that ('read data', {
               expect_silent (store_bikedata ("..", "testdb", quiet=TRUE))
               expect_error (store_bikedata ("../tests", "testdb"),
                             "File named testdb already exists")
               expect_true (file.exists ("testdb"))
               invisible (file.remove ("testdb"))
               expect_message (store_bikedata ("..", "testdb"))
               db <- dplyr::src_sqlite ('testdb', create=F)
               trips <- dplyr::collect (dplyr::tbl (db, 'trips'))
               expect_equal (dim (trips), c (81, 11))
               stns <- dplyr::collect (dplyr::tbl (db, 'stations'))
               expect_equal (nrow (stns), 9)
               invisible (file.remove ("testdb"))
})
