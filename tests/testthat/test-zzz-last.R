
is_cran <- identical (Sys.getenv ('NOT_CRAN'), 'false')

if (!is_cran)
{
    data_dir <- getwd ()
    nf <- length (list.files (data_dir, pattern = '.zip'))
    if (nf > 0)
        bike_rm_test_data (data_dir = data_dir)
    bikedb <- file.path (getwd (), "testdb")
    if (file.exists (bikedb))
        invisible (file.remove (bikedb))
}
