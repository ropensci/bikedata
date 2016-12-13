/***************************************************************************
 *  Project:    bikedata
 *  File:       Utils.h
 *  Language:   C++
 *
 *  bike-data is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the Free
 *  Software Foundation, either version 3 of the License, or (at your option)
 *  any later version. See <https://www.gnu.org/licenses/>
 *
 *  bike-data is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 *  or FITNESS FOR A PARTICULAR PURPOSE.  For more details, see the GNU General
 *  Public License at <https://www.gnu.org/licenses/>
 *
 *  Copyright   Mark Padgham November 2016
 *  Author:     Mark Padgham
 *  E-Mail:     mark.padgham@email.com
 *
 *  Description:    Constructs correltaion matrices between all stations of
 *                  public bicycle hire systems for London, UK, and Boston,
 *                  Chicago, Washington DC, and New York, USA. Also analyses
 *                  Oystercard data for London.
 *
 *  Limitations:
 *
 *  Dependencies:       libboost
 *
 *  Compiler Options:   -std=c++11 -lzip
 ***************************************************************************/

#pragma once

#include <stdlib.h> // has abs function
#include <math.h>
#include <iostream>
#include <stdio.h>
#include <time.h>
#include <limits.h>
#include <vector>
#include <string>
#include <iomanip> // for setfill
#include <sys/ioctl.h> // for console width: Linux only!
#include <ctype.h>
#include <fstream>
#include <assert.h>

// [[Rcpp::depends(BH)]]
#include <Rcpp.h>

#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>

typedef boost::numeric::ublas::matrix <int> imat;
typedef boost::numeric::ublas::matrix <double> dmat;

const double DOUBLE_MAX = std::numeric_limits<double>::max (),
    DOUBLE_MIN = -DOUBLE_MAX,
    FLOAT_MAX = std::numeric_limits <float>::max ();
