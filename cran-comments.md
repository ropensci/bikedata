# Test environments

* Ubuntu 12.04 (on `travis-ci`): R-release, R-devel
* Ubuntu 14.04 (on `travis-ci`): R-release, R-devel
* OXS: R-release (on `travis-ci`)
* Windows Visual Studio 2015 (on `appveyor`; `x64`): R-release, R-devel
* win-builder (R-release, R-devel, R-oldrelease)


# R CMD check results

Two NOTEs are generated: one regarding potential mis-spelling of "Los" and
"Angeles", about which I can do nothing; the other concerns large installed
package size, which is primarily due to bundled C++ SQLite3 libs, along with
extensive internal C++ routines.
