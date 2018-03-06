* Previous errors resulted from changes in directory structure for one Amazon
  AWS service accessed by the package. Code now updated to successfully manage
  these recently modified directories.

# Test environments

* Ubuntu 14.04 (on `travis-ci`): R-release, R-devel
* OSX: R-release (on `travis-ci`)
* Windows Visual Studio 2015 (on `appveyor`; `x64`): R-release, R-devel
* win-builder (R-release, R-devel, R-oldrelease)

Package also checked using both local memory sanitzer and `rocker/r-devel-san` with clean results.


# R CMD check results

The following NOTEs are generated:
1. Potential mis-spelling of "Los" and "Angeles" (on R-release, R-oldrelease).
2. Note regarding package size due to necessity of internally bundling SQLite3 header library, along with extensive internal C++ routines.
