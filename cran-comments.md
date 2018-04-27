# Test environments

* Ubuntu 14.04 (on `travis-ci`): R-release, R-devel
* OSX: R-release (on `travis-ci`)
* Windows Visual Studio 2015 (on `appveyor`; `x64`): R-release, R-devel
* win-builder (R-release, R-devel, R-oldrelease)
* Package also checked using `rocker/r-devel-san` with clean results.
* local valgrind --memcheck gives clean results


# R CMD check results

The only NOTE generated regards package size, which is unavoidable due to necessity of internally bundling SQLite3 header library, along with extensive internal C++ routines.

Additional NOTEs for r-oldrelease as with previous submissions of this package:
1. Possible mis-spelling of "Los" and "Angeles"
2. Authors@R with no valid roles for two instances of 'role = "rev"'
