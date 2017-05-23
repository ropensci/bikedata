library(testthat)
library(bikedata)

is_cran <- identical (Sys.getenv ('NOT_CRAN'), 'false')
if (!is_cran)
    test_check("bikedata")
