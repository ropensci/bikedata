context ("stations")

require (testthat)

data_dir <- getwd ()
# CRAN does not permit files to be removed, so this only writes them if they
# don't already exist
nf <- length (list.files (data_dir, pattern = '.zip'))
if (nf < 6)
    bike_write_test_data (data_dir = data_dir)
bikedb <- file.path (getwd (), "testdb")
if (!exists (bikedb))
    store_bikedata (data_dir = data_dir, bikedb = bikedb)

test_that ('station data', {
               st <- bike_stations (bikedb)
               expect_true (nrow (st) >= 2191)
})

#invisible (file.remove (file.path (tempdir (), "testdb")))
n <- bike_rm_test_data (data_dir = data_dir)
invisible (file.remove (bikedb))
