/***************************************************************************
 *  Project:    bikedata
 *  File:       mainBikes.h
 *  Language:   C++
 *
 *  bikedata is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the Free
 *  Software Foundation, either version 3 of the License, or (at your option)
 *  any later version. See <https://www.gnu.org/licenses/>
 *
 *  bikedata is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 *  or FITNESS FOR A PARTICULAR PURPOSE.  For more details, see the GNU General
 *  Public License at <https://www.gnu.org/licenses/>
 *
 *  Copyright   Mark Padgham December 2016
 *  Author:     Mark Padgham
 *  E-Mail:     mark.padgham@email.com
 *
 *  Description:    Loads data from NYC citibike bicycle hire system.
 *
 *  Limitations:
 *
 *  Dependencies:       libboost
 *
 *  Compiler Options:   -std=c++11 -lzip
 ***************************************************************************/

#pragma once

#include "common.h"
#include "StationData.h"
#include "RideData.h"

void readNYC (RideData rideData);
