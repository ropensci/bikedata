context ("stations")

require (testthat)

#bike_write_test_data ()
store_bikedata (data_dir = "..", bikedb = "testdb")

test_that ('station data', {
               st <- bike_stations ('testdb')
               expect_true (nrow (st) >= 2191)
})

#bike_rm_test_data ()
invisible (file.remove (file.path (tempdir (), "testdb")))
