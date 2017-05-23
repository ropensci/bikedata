
is_cran <- identical (Sys.getenv ("_R_CHECK_CRAN_INCOMING_"), 'true')
is_travis <- identical (Sys.getenv ("TRAVIS"), 'true')

if (!is_cran | is_travis)
{
    data_dir <- getwd ()
    nf <- length (list.files (data_dir, pattern = '.zip'))
    if (nf > 0)
        bike_rm_test_data (data_dir = data_dir)
    bikedb <- file.path (getwd (), "testdb")
    if (file.exists (bikedb))
        invisible (file.remove (bikedb))
}
