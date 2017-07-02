# Test environments

* Ubuntu 12.04 (on `travis-ci`): R-release, R-devel
* Ubuntu 14.04 (on `travis-ci`): R-release, R-devel
* OXS: R-release (on `travis-ci`)
* Windows Visual Studio 2015 (on `appveyor`; `x64`): R-release, R-devel
* win-builder (R-release, R-devel, R-oldrelease)

Note that checking package with `valgrind` identifies numerous memory leaks, all
of which arise during database construction via the bundled `sqlite3.c` code
from http://www.sqlite.org, and that similar memory leads arise with a valgrind
check of the R package `RSQLite` which also relies on this bundle. All database
disconnections are nevertheless checked and successful tests therefore indicate
that the code successfully constructs and disconnects from the resultant
database. Successful tests accordingly indicate that these memory leaks do not
affect package functionality.

# R CMD check results

Only NOTE concerns large installed package size, which is primarily due to
bundled C++ SQLite3 libs, along with extensive internal C++ routines.
