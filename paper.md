---
title: bikedata
tags:
    - public hire bicycle
    - open data
    - R
authors:
    - name: Mark Padgham
      affiliation: 1
    - name: Richard Ellison
      affiliation: 3
affiliations:
    - name: Department of Geoinformatics, University of Salzburg, Austria
      index: 1
    - name: Institute of Transport and Logistics Studies, The University of
      Sydney, Australia
      index: 2
date: 31 May 2017
---

# Summary

The R package `bikedata` aims to download and aggregate data from all public
hire bicycle systems which provide open data. These currently including
Santander Cycles in London, U.K., and from the U.S.A., citibike in New York City
NY, Divvy in Chicago IL, Capital Bikeshare in Washington DC, Hubway in Boston
MA, and Metro in Los Angeles LA. The list of stations will be expanded on an
ongoing basis. The package facilitates the three necessary steps of (1)
downloading data; (2) storing data in a readily accessible form (in this case in
a single SQLite3 database); (3) extracting aggregate statistics. The two primary
aggregate statistics are matrixes of aggregate numbers of trips between all
pairs of stations, and daily time series of numbers of trips. Both forms of
aggregation may be extracted for specific dates, times, or demographic
characteristics of cyclists.
