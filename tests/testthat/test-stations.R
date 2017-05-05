context ("stations")

require (testthat)

data_dir <- getwd ()
bike_write_test_data (data_dir = data_dir)
bikedb <- file.path (getwd (), "testdb")
store_bikedata (data_dir = data_dir, bikedb = bikedb)
#store_bikedata (data_dir = "..", bikedb = bikedb)

test_that ('station data', {
               st <- bike_stations (bikedb)
               expect_true (nrow (st) >= 2191)
})

#invisible (file.remove (file.path (tempdir (), "testdb")))
bike_rm_test_data (data_dir = data_dir)
invisible (file.remove (bikedb))
