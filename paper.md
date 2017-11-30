---
title: bikedata
tags:
    - public hire bicycle
    - open data
    - R
authors:
    - name: Mark Padgham
      affiliation: 1
      orcid: 0000-0003-2172-5265
    - name: Richard Ellison
      affiliation: 2
affiliations:
    - name: Department of Geoinformatics, University of Salzburg, Austria
      index: 1
    - name: Institute of Transport and Logistics Studies, The University of Sydney, Australia
      index: 2
bibliography: paper.bib
date: 28 Nov 2017
---

# Summary

The R package `bikedata` collates and facilitates access to arguably the world's
largest open ongoing dataset on human mobility. All other comparable sources of
data (such public transit data, or mobile phone data) are either not publicly
available, or have been released only at single distinct times for single
distinct purposes. Many public hire bicycle systems in the U.S.A., along with
Santander Cycles in London, U.K., issue ongoing releases of their usage data,
providing a unique source of data for analysing, visualising, and understanding
human movement and urban environments [@Austwick2013; @Borgnat2011;
@Padgham2012].  Such data provide an invaluable resource for urban planners,
geographers, social and health scientists and policy makers, data visualisation
specialists, and data-affine users of the systems themselves.  The `bikedata`
package aims to provide unified access to usage statistics from all public hire
bicycle systems which provide  data. These currently including Santander Cycles
in London, U.K., and from the U.S.A., citibike in New York City NY, Divvy in
Chicago IL, Capital Bikeshare in Washington DC, Hubway in Boston MA, Metro in
Los Angeles LA, and Indego in Philadelphia PA. Additional systems will be added
on an ongoing basis.  The package facilitates the three necessary steps of (1)
downloading data; (2) storing data in a readily accessible form (in this case in
a single SQLite3 database); (3) extracting aggregate statistics.  The two
primary aggregate statistics are matrices of numbers of trips between all pairs
of stations, and daily time series. Both forms of aggregation may be extracted
for specific dates, times, or demographic characteristics of cyclists.

# References
