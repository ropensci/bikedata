Previous versions failed intermittently because of intermittent http::GET
errors in tests. This submission fixes these failures by wrapping all http
requests in `tryCatch` statements to ensure that they "fail gracefully" without
generating errors or warnings.

Note nevertheless that the submission fails on win-builder R 3.5.3
(old-rel)with a fail on install because "Error : package 'osmdata' was
installed by an R version with different internals; it needs to be reinstalled
for use with this R version".

Other than that, there are no notes on the following:

# Test environments

* Ubuntu 16.04 (on `travis-ci`): R-release, R-devel
* OSX: R-release (on `travis-ci`)
* Windows Visual Studio 2015 (on `appveyor`; `x64`): R-release, R-devel
* win-builder (R-release, R-devel, R-oldrelease)
* Package also checked using `rocker/r-devel-san` with clean results.
* local valgrind --memcheck gives clean results

