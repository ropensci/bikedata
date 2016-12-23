/***************************************************************************
 *  Project:    bikedata
 *  File:       StationData.c++
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


#include "StationData.h"


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                           GETDIRNAME                               **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

std::string Stations::GetDirName ()
{
    std::string dirtxt = "/data/data/junk/zip";
    return dirtxt;
} // end Stations::GetDirName

/************************************************************************
 ************************************************************************
 **                                                                    **
 **                           GETDIRLIST                               **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

void StationData::GetDirList ()
{
    std::string fname;
    DIR *dir;
    struct dirent *ent;

    filelist.resize (0);
    if ((dir = opendir (_dirName.c_str())) != NULL) 
    {
        while ((ent = readdir (dir)) != NULL) 
        {
            fname = ent->d_name;
            if (fname != "." && fname != "..") 
            {
                fname = fname;
                filelist.push_back (fname);
            }
        }
        closedir (dir);
        std::sort (filelist.begin(), filelist.end());
    } else {
        std::string outstr = "ERROR: Directory for city = " +\
                              _city + " at " + _dirName + " does not exist";
        perror ("");
        Rcpp::Rcout << outstr << std::endl;
        //return EXIT_FAILURE;
    }
} // end StationData::GetDirList


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                          GETSTATIONS                               **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

int StationData::GetStations ()
{
    /*
     * Reads from station_latlons which is constructed with getLatLons.py and
     * *MUST* be ordered numerically. Returns _numStations
     *
     * TODO: Write handlers for cases where there are trips to/from stations not
     * in StationList.
     * TODO: Update StationList for nyc, because there seem to be trips to/from
     * one station that is not in list.
     * 
     */
    const std::string dir = "data/"; 
    int ipos, tempi, count;
    bool tube;
    OneStation oneStation;
    std::string fname;
    std::ifstream in_file;
    std::string linetxt;

    StationList.resize (0);
    count = 0;
    oneStation.name = "";

    // _city == "nyc" hard coded here
    fname = dir + "station_latlons_" + _city + ".csv";
    in_file.open (fname.c_str (), std::ifstream::in);
    if (in_file.fail ())
        throw std::runtime_error ("station_latlons not found");
    in_file.clear ();
    in_file.seekg (0); 
    getline (in_file, linetxt, '\n'); // header
    while (!in_file.eof ()) 
    {
        getline (in_file, linetxt,'\n');
        if (linetxt.length () > 1) 
        {
            ipos = linetxt.find(',',0);
            tempi = atoi (linetxt.substr (0, ipos).c_str());
            if (tempi > count) 
                count = tempi;
            oneStation.ID = tempi;
            linetxt = linetxt.substr (ipos + 1, linetxt.length () - ipos - 1);
            ipos = linetxt.find (',', 0);
            oneStation.lat = atof (linetxt.substr (0, ipos).c_str());
            linetxt = linetxt.substr (ipos + 1, linetxt.length () - ipos - 1);
            ipos = linetxt.find (',', 0);
            oneStation.lon = atof (linetxt.substr (0, ipos).c_str());
            StationList.push_back (oneStation);
        }
    }
    in_file.close();

    return count;
} // end StationData::GetStations


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                          MAKESTATIONINDEX                          **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

void StationData::MakeStationIndex ()
{
    // First station is #1 and last is _maxStation, so _StationIndex has 
    // len (_maxStns + 1), with _StationIndex [sti.ID=1] = 0 and
    // _StationIndex [sti.ID=_maxStation] = _numStations.
    OneStation sti;

    _StationIndex.resize (_maxStation + 1);
    for (std::vector <int>::iterator pos=_StationIndex.begin();
            pos != _StationIndex.end(); pos++)
        *pos = INT_MIN;
    for (int i=0; i<StationList.size (); i++) 
    {
        sti = StationList [i];
        _StationIndex [sti.ID] = i;
    }
} // end StationData::MakeStationIndex


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                             COUNTTRIPS                             **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

double StationData::CountTrips ()
{
    double count = 0.0;

    for (int i=0; i<ntrips.size1(); i++)
        for (int j=0; j<ntrips.size2(); j++)
            count += ntrips (i, j);

    return (count);
}

/************************************************************************
 ************************************************************************
 **                                                                    **
 **                           WRITENUMTRIPS                            **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

int StationData::writeNumTrips (std::string fname)
{
    int numStations = ntrips.size1 (); // < _numStations for rail data

    std::ofstream out_file;
    out_file.open (fname.c_str (), std::ofstream::out);
    for (int i=0; i<numStations; i++)
    {
        for (int j=0; j<numStations; j++)
        {
            out_file << ntrips (i, j);
            if (j == (numStations - 1))
                out_file << std::endl;
            else
                out_file << ", ";
        }
    }
    out_file.close ();
    //std::cout << "Numbers of trips written to " << fname.c_str () << std::endl;

    return 0;
}

