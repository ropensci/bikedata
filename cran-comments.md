# CRAN notes for bikedata_0.2.5 submission

This package was previously, "Archived again on 2020-02-12 as check issues were
not corrected on re-submission. UBSAN reports integer overflow, valgrind
reports use of uninitialized values." My recent resubmission still manifest the
integer overflow problem which this submission now rectifies. The problem arose
through me overseeing an inline conversion to <int> as part of a <long int>
variable. I have confirmed with g++ UBSAN that the present submission fixes the
issue.

This submission has also been tested on:

# Test environments

- CRAN win-builder: R-oldrelease, R-release, R-devel
* Ubuntu 16.04 (on `travis-ci`): R-release, R-devel, R-oldrelease
* OSX: R-release (on `travis-ci`)

