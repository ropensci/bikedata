* Previous CRAN errors resulted from changes in directory structure for one Amazon AWS service accessed by the package. Code now updated to successfully manage these recently modified directories.
* Two function examples inadvertently wrote to getwd; these have been switched off.

# Test environments

* Ubuntu 14.04 (on `travis-ci`): R-release, R-devel
* OSX: R-release (on `travis-ci`)
* Windows Visual Studio 2015 (on `appveyor`; `x64`): R-release, R-devel
* win-builder (R-release, R-devel, R-oldrelease)

Package also checked using both local memory sanitzer and `rocker/r-devel-san` with clean results.


# R CMD check results

The following NOTEs are generated:
1. Possible mis-spelling of "Los" and "Angeles" (on R-release, R-oldrelease).
2. Note regarding package size due to necessity of internally bundling SQLite3 header library, along with extensive internal C++ routines.
3. Possile mis-spelling in DESCRIPTION of x-schema fields (on R-release, R-oldrelease).
4. Notes on R-release, R-oldrelease regarding Authors@R with no valid roles for two instances of 'role = "rev"', as with previous releases of this package
5. Notes on R-oldrelease regarding (possibly) invalid URLs promted by libcurl error code 35, which Jeroen Ooms has fixed on all later releases
