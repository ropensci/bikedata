.onAttach <- function(libname, pkgname) {
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
                   "  and https://www.thehubway.com/data-license-agreement")
    packageStartupMessage (msg)
}
