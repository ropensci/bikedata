context ("stations")

require (testthat)

store_bikedata (data_dir = "..", bikedb = "testdb")

test_that ('station data', {
               st <- bike_stations ('testdb')
               expect_equal (nrow (st), 2178)
})

invisible (file.remove ("testdb"))
