.onAttach <- function(libname, pkgname) {
    f <- file.path (tempdir (), "bikedata_headers.csv")
    write.csv (sysdata$headers, file = f, row.names = FALSE)
    f <- file.path (tempdir (), "field_names.csv")
    write.csv (sysdata$field_names, file = f, row.names = FALSE, quote = FALSE)

    msg <- paste0 ("Data for London, U.K. powered by TfL Open Data:\n",
                   "  Contains OS data \u24B8 Crown copyright and ",
                   "database rights 2016\n",
                   "Data for New York City provided and owned by:\n",
                   "  NYC Bike Share, LLC and ",
                   "Jersey City Bike Share, LLC (\"Bikeshare\")\n",
                   "  see https://www.citibikenyc.com/data-sharing-policy\n",
                   "Data for Washington DC (Captialbikeshare), ",
                   "Chiago (Divvybikes) and Boston (Hubway)\n",
                   "  provided and owned by Motivate International Inc.\n",
                   "  see https://www.capitalbikeshare.com/data-license-agreement\n", #nolint
                   "  and https://www.divvybikes.com/data-license-agreement\n",
                   "  and https://www.thehubway.com/data-license-agreement\n",
                   "Nice Ride Minnesota license",
                   "  https://www.niceridemn.org/data_license")
    packageStartupMessage (msg)
}

.onLoad <- function (libname, pkgname)
{
    # make data set names global to avoid CHECK notes
    utils::globalVariables ("sysdata")
    invisible ()
}
