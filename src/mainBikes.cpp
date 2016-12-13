/***************************************************************************
 *  Project:    bike-data
 *  File:       mainBikes.c++
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


#include "mainBikes.h"

#include <Rcpp.h>

/************************************************************************
 ************************************************************************
 **                                                                    **
 **                         MAIN FUNCTION                              **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

//' rcpp_get_bikedata
//'
//' Extracts bike data for NYC citibike
//'
//' @return nothing
// [[Rcpp::export]]
int rcpp_get_bikedata ()
{
    int nfiles, count, tempi [2], age;
    std::string city, subscriber, gender, r2name, covname, MIname;

    city = "nyc";
    subscriber = "all";
    gender = "all";
    age = -1;


    tempi [0] = 0;
    tempi [1] = 0;

    RideData rideData (city, tempi [0], tempi [1]);

    int numStations = rideData.returnNumStations();
    Rcpp::Rcout << "There are " << numStations << 
        " stations [max#=" << rideData.getStnIndxLen() << "] and " << 
        rideData.getNumFiles() << " trip files." << std::endl;

    readNYC (rideData);

    return 0;
}


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                            READNYC                                 **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

void readNYC (RideData rideData)
{
    int tempi, count = 0;

    for (int i=0; i<rideData.getNumFiles(); i++)
    {
        tempi = rideData.getZipFileNameNYC (i);
        if (rideData.fileName != "") {
            //count += rideData.readOneFileNYC (i);
            tempi = rideData.readStationsNYC (i);
            tempi = rideData.removeFile ();
        }
    } // end for i
    Rcpp::Rcout << "Total number of trips = " << count << std::endl;
    //rideData.summaryStats ();
    tempi = rideData.aggregateTrips ();
    std::string fname = "NumTrips_nyc_" + std::to_string (rideData.getSubscriber()) +
        std::to_string (rideData.getGender ()) + ".csv";
    rideData.writeNumTrips (fname);

    //std::cout << "---There are " << rideData.StnXY.size () << 
    //    " stations---" << std::endl;
}


