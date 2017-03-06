context ("stations")

store_bikedata ("..", "testdb")

test_that ('station data', {
               st <- bike_stations ('testdb')
               expect_equal (nrow (st), 9)
})

invisible (file.remove ("testdb"))
