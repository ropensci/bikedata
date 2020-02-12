# CRAN notes for bikedata_0.2.5 submission

This package was previously, "Archived again on 2020-02-12 as check issues were
not corrected on re-submission. UBSAN reports integer overflow, valgrind
reports use of uninitialized values."

This submission rectifies both of these previous issues, as well as prior
issues with failing http calls. It has been tested on:

# Test environments

- CRAN win-builder: R-oldrelease, R-release, R-devel
* Ubuntu 16.04 (on `travis-ci`): R-release, R-devel, R-oldrelease
* OSX: R-release (on `travis-ci`)

