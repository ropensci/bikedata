
data_dir <- getwd ()
nf <- length (list.files (data_dir, pattern = '.zip'))
if (nf > 0)
    bike_rm_test_data (data_dir = data_dir)
bikedb <- file.path (getwd (), "testdb")
if (file.exists (bikedb))
    chk <- tryCatch (file.remove (bikedb), 
                     warning = function (w) NULL,
                     error = function (e) NULL)
