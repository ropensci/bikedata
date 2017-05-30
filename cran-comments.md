# Test environments

* Ubuntu 12.04 (on `travis-ci`): R-release, R-devel
* Ubuntu 14.04 (on `travis-ci`): R-release, R-devel
* OXS: R-release (on `travis-ci`)
* Windows Visual Studio 2015 (on `appveyor`; `x64`): R-release, R-devel
* win-builder (R-release, R-devel, R-oldrelease)

Note that checking package with valgrind identifies numerous memory leaks, all
of which arise during data construction via the bundled `sqlite3.c` code from
http://www.sqlite.org (similar memory leads arise with a valgrind check of the R
package `RSQLite`). All database disconnections are nevertheless checked and
successful tests therefore indicate that the code successfully constructs and
disconnects from the resultant database. Successful tests accordingly indicate
that these memory leaks do not affect package functionality.

# R CMD check results

0 errors | 0 warnings | 1 note
checking installed package size ... NOTE
* installed size is  6.7Mb
    sub-directories of 1Mb or more:
        doc    2.6Mb
        libs   3.1Mb
            
Large size primarily due to bundled C++ SQLite3 libs.
