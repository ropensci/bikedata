/***************************************************************************
 *  Project:    bikedata
 *  File:       StationData.h
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
 *  Dependencies:       libboost (via R "BH")
 *
 *  Compiler Options:   -std=c++11 -lzip
 ***************************************************************************/

#pragma once

#include "common.h"

#include <dirent.h>
#include <stdlib.h> // for EXIT_FAILURE
#include <string.h>
#include <fstream>
#include <math.h>
#include <iostream>
#include <stdio.h>
#include <time.h>
#include <vector>
#include <iomanip> // for setfill
#include <sys/ioctl.h> // for console width: Linux only!

#include <Rcpp.h>

class Stations // for both bikes and trains
{
    /*
     * Stations and BikeStation data are generic classes into which all data
     * is initially loaded, and the only classes on which subsequent
     * manipulations should be make. The descendent class RideData exists only
     * to load data into the standard StationData and BikeStation classes.
     */
    protected:
        std::string _dirName;
        const std::string _city;
        bool _standardise;
        // Standardises ntrips to unit sum, so covariances do not depend on
        // scales of actual numbers of trips. Set to true in initialisation.
        bool _deciles;
    public:
        std::string fileName;
        Stations (std::string str)
            : _city (str)
        {
            _standardise = true; // false doesn't make sense
            _deciles = false;
            _dirName = GetDirName ();
        }
        ~Stations ()
        {
        }

        int getStandardise () { return _standardise;    }

        std::string returnDirName () { return _dirName; }
        std::string returnCity () { return _city;   }
        bool returnDeciles () { return _deciles;    }
        
        std::string GetDirName ();
};


class StationData : public Stations
{
    protected:
        int _numStations, _maxStation;
        std::vector <int> _StationIndex;
    public:
        int tempi;
        std::vector <int> missingStations;
        std::vector <std::string> filelist;
        struct OneStation 
        {
            std::string name; // for train stations only
            int ID;
            float lon, lat;
        };
        std::vector <OneStation> StationList;
        dmat ntrips; // dmat to allow standardisation to unit sum

        StationData (std::string str)
            : Stations (str)
        {
            GetDirList ();
            _maxStation = GetStations ();
            _numStations = StationList.size ();
            missingStations.resize (0);
            InitialiseArrays ();
            if (_city.substr (0, 6) != "oyster")
                MakeStationIndex ();
        }
        ~StationData()
        {
            filelist.resize (0);
            missingStations.resize (0);
            ntrips.resize (0, 0);
        }

        int returnNumStations () { return _numStations; }
        int returnMaxStation () { return _maxStation;   }

        void GetDirList ();
        int GetStations ();
        void MakeStationIndex ();
        double CountTrips ();

        int writeNumTrips (std::string fname);

        void InitialiseArrays ()
        {
            ntrips.resize (_numStations, _numStations);
            for (int i=0; i<_numStations; i++)
                for (int j=0; j<_numStations; j++)
                    ntrips (i, j) = 0.0;
        }
}; // end class StationData
