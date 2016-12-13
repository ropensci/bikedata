/***************************************************************************
 *  Project:    bikedata
 *  File:       RideData.c++
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


#include "RideData.h"


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                        DUMPMISSINGSTATIONS                         **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

void RideData::dumpMissingStations ()
{
    if (missingStations.size () > 0)
    {
        std::sort (missingStations.begin(), missingStations.end());
        Rcpp::Rcout << "The following stations are in trip files " <<
            "yet missing from station_latlons:" << std::endl << "(";
        for (std::vector <int>::iterator pos=missingStations.begin();
                pos != missingStations.end(); pos++)
            Rcpp::Rcout << *pos << ", ";
        Rcpp::Rcout << ")" << std::endl;
    }
}


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                            REMOVEFILE                              **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

int RideData::removeFile ()
{
    std::string fname_csv = StationData::GetDirName() + '/' + fileName;

    if (remove(fname_csv.c_str()) != 0)
        return 1;
    else
        return 0;
}




/************************************************************************
 ************************************************************************
 **                                                                    **
 **                         GETZIPFILENAMENYC                          **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

int RideData::getZipFileNameNYC (int filei)
{
    const char *archive;
    struct zip *za;
    struct zip_file *zf;
    struct zip_stat sb;
    char buf[100]; 
    int err;

    std::string fname_base = StationData::GetDirName() + '/' + filelist [filei];
    archive = fname_base.c_str ();
    if ((za = zip_open(archive, 0, &err)) == NULL) {
        zip_error_to_str(buf, sizeof(buf), err, errno);
        //Rcpp::Rcout << stderr << archive << "can't open size archive : " <<
        //    buf << std::endl;
        return 1;
    } 

    if (zip_get_num_entries (za, 0) == 1) {
        if (zip_stat_index(za, 0, 0, &sb) == 0) {
            fileName = sb.name;
            zf = zip_fopen_index(za, 0, 0);
            if (!zf) {
                //fprintf(stderr, "ERROR: zip can not be opened/n");
                return 1;
            }
        }
    } 
    zip_fclose(zf); 
    if (zip_close(za) == -1) {
        //Rcpp::Rcout << stderr << "can't close zip archive " << archive <<
        //    std::endl;
        //fprintf(stderr, "can't close zip archive `%s'/n", archive);
        return 1;
    }

    return 0;
}


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                          READSTATIONSNYC                           **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

int RideData::readStationsNYC (int filei)
{
    // First unzip file, for which all error checks have been done in
    // getZipFileNameNYC above
    std::string fname = StationData::GetDirName() + '/' + filelist [filei];
    const char *archive;
    struct zip *za;
    struct zip_file *zf;
    struct zip_stat sb;
    char buf[100]; 
    int err, len, fileYear, age;
    long long sum;

    archive = fname.c_str ();
    za = zip_open(archive, 0, &err);
    zf = zip_fopen_index(za, 0, 0);
    zip_stat_index(za, 0, 0, &sb);
    std::string fname_csv = StationData::GetDirName() + '/' + fileName;
    std::ofstream out_file (fname_csv.c_str(), std::ios::out);
    fileYear = atoi (fileName.substr (0, 4).c_str());

    sum = 0;
    while (sum != sb.size) {
        len = zip_fread(zf, buf, 100);
        if (len < 0) {
            // TODO: INSERT ERROR HANDLER
        }
        out_file.write (buf, len);
        sum += len;
    }
    out_file.close ();
    zip_fclose(zf); 

    // Then read unzipped .csv file
    int count = 0, ipos, tempi [4];
    int nstations = getStnIndxLen ();
    float stn_lat, stn_lon;
    std::ifstream in_file;
    std::string linetxt, stn_name, usertype;

    in_file.open (fname_csv.c_str (), std::ifstream::in);
    assert (!in_file.fail ());
    getline (in_file, linetxt, '\n');
    while (getline (in_file, linetxt, '\n')) { count++;	}

    Rcpp::Rcout << "Reading file [";
    if (filei < 10)
        Rcpp::Rcout << " ";
    Rcpp::Rcout << filei << "/" << filelist.size() <<
        "]: " << fileName.c_str() << " with " <<
        count << " records" << std::endl;

    in_file.clear ();
    in_file.seekg (0); 
    getline (in_file, linetxt, '\n');
    count = 0;

    /* Structure is:
    [1] tripduration, [2] starttime, [3] stoptime, [4] start station id, 
    [5] start station name, [6] start station latitude, 
    [7] start station longitude, [8] end station id, [9] end station name, 
    [10] end station latitude [11] end station longitude, [12] bikeid, 
    [13] usertype, [14] birth year, [15] gender (1=male, 2=female)
    Note that birthyears seem uncontrolled, so there are quite a number that are
    1899, 1900, or 1901, yet none further until 1920 or so.
    */
    while (getline (in_file, linetxt,'\n')) {
        for (int i=0; i<3; i++) {
            ipos = linetxt.find("\",\"",0);
            linetxt = linetxt.substr (ipos + 3, linetxt.length () - ipos - 1);
        }
        ipos = linetxt.find ("\",\"", 0);
        tempi [0] = atoi (linetxt.substr (0, ipos).c_str()); // Start Station ID

        linetxt = linetxt.substr (ipos + 3, linetxt.length () - ipos - 1);
        ipos = linetxt.find ("\",\"", 0);
        stn_name = linetxt.substr (0, ipos).c_str(); // Station Name

        linetxt = linetxt.substr (ipos + 3, linetxt.length () - ipos - 1);
        ipos = linetxt.find ("\",\"", 0);
        stn_lat = atof (linetxt.substr (0, ipos).c_str());
        linetxt = linetxt.substr (ipos + 3, linetxt.length () - ipos - 1);
        ipos = linetxt.find ("\",\"", 0);
        stn_lon = atof (linetxt.substr (0, ipos).c_str());

        if (StnXY.find (stn_name) == StnXY.end ())
            StnXY [stn_name] = std::make_pair (stn_lon, stn_lat);
    } // end while getline
    in_file.close();

    return count;
} // end readOneFileNYC


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                           READONEFILENYC                           **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

int RideData::readOneFileNYC (int filei)
{
    // First unzip file, for which all error checks have been done in
    // getZipFileNameNYC above
    std::string fname = StationData::GetDirName() + '/' + filelist [filei];
    const char *archive;
    struct zip *za;
    struct zip_file *zf;
    struct zip_stat sb;
    char buf[100]; 
    int err, len, fileYear, age;
    long long sum;

    archive = fname.c_str ();
    za = zip_open(archive, 0, &err);
    zf = zip_fopen_index(za, 0, 0);
    zip_stat_index(za, 0, 0, &sb);
    std::string fname_csv = StationData::GetDirName() + '/' + fileName;
    std::ofstream out_file (fname_csv.c_str(), std::ios::out);
    fileYear = atoi (fileName.substr (0, 4).c_str());

    sum = 0;
    while (sum != sb.size) {
        len = zip_fread(zf, buf, 100);
        if (len < 0) {
            // TODO: INSERT ERROR HANDLER
        }
        out_file.write (buf, len);
        sum += len;
    }
    out_file.close ();
    zip_fclose(zf); 

    // Then read unzipped .csv file
    int count = 0, ipos, tempi [4];
    int nstations = getStnIndxLen ();
    std::ifstream in_file;
    std::string linetxt, usertype;

    in_file.open (fname_csv.c_str (), std::ifstream::in);
    assert (!in_file.fail ());
    getline (in_file, linetxt, '\n');
    while (getline (in_file, linetxt, '\n')) { count++;	}

    Rcpp::Rcout << "Reading file [";
    if (filei < 10)
        Rcpp::Rcout << " ";
    Rcpp::Rcout << filei << "/" << filelist.size() <<
        "]: " << fileName.c_str() << " with " <<
        count << " records";
    Rcpp::Rcout.flush ();

    in_file.clear ();
    in_file.seekg (0); 
    getline (in_file, linetxt, '\n');
    count = 0;

    /* Structure is:
    [1] tripduration, [2] starttime, [3] stoptime, [4] start station id, 
    [5] start station name, [6] start station latitude, 
    [7] start station longitude, [8] end station id, [9] end station name, 
    [10] end station latitude [11] end station longitude, [12] bikeid, 
    [13] usertype, [14] birth year, [15] gender (1=male, 2=female)
    Note that birthyears seem uncontrolled, so there are quite a number that are
    1899, 1900, or 1901, yet none further until 1920 or so.
    */
    while (getline (in_file, linetxt,'\n')) {
        for (int i=0; i<3; i++) {
            ipos = linetxt.find("\",\"",0);
            linetxt = linetxt.substr (ipos + 3, linetxt.length () - ipos - 1);
        }
        ipos = linetxt.find ("\",\"", 0);
        tempi [0] = atoi (linetxt.substr (0, ipos).c_str()); // Start Station ID
        linetxt = linetxt.substr (ipos + 3, linetxt.length () - ipos - 1);
        for (int i=0; i<3; i++) {
            ipos = linetxt.find ("\",\"", 0);
            linetxt = linetxt.substr (ipos + 3, linetxt.length () - ipos - 1);
        }
        ipos = linetxt.find ("\",\"", 0);
        tempi [1] = atoi (linetxt.substr (0, ipos).c_str()); // End Station ID
        if (tempi [0] >= 0 && tempi [0] < RideData::getStnIndxLen() && 
                tempi [1] >= 0 && tempi [1] < RideData::getStnIndxLen() &&
                tempi [0] != tempi [1])
        {
            tempi [0] = _StationIndex [tempi[0]];
            tempi [1] = _StationIndex [tempi[1]];
            if (tempi [0] < 0 || tempi [0] > nstations || tempi [1] < 0 ||
                    tempi [1] > nstations) { // should never happen
                throw std::runtime_error ("station not in StationIndex");
            }
            linetxt = linetxt.substr (ipos + 3, linetxt.length () - ipos - 1);
            // Then extract usertype, birthyear, gender
            for (int i=0; i<4; i++) {
                ipos = linetxt.find("\",\"",0);
                linetxt = linetxt.substr (ipos + 3, linetxt.length () - ipos - 1);
            }
            ipos = linetxt.find ("\",\"", 0);
            usertype = linetxt.substr (0, ipos).c_str(); // User type
            linetxt = linetxt.substr (ipos + 3, linetxt.length () - ipos - 1);
            ipos = linetxt.find ("\",\"", 0);
            tempi [2] = atoi (linetxt.substr (0, ipos).c_str()); // Birthyear
            age = fileYear - tempi [2];
            if (age > 0 && age < 99)
                ageDistribution [age]++;
            tempi [2] = floor (tempi [2] / 10);
            if (RideData::getSubscriber () > 3)
            {
                if (age > 0 && age < 38) // Average age is 37.7
                    ntripsYoung (tempi [0], tempi [1])++;
                else if (age < 99)
                    ntripsOld (tempi [0], tempi [1])++;
                // TODO: Write this better!
                if (tempi [2] == 192)
                    ntrips1920 (tempi [0], tempi [1])++;
                else if (tempi [2] == 193)
                    ntrips1930 (tempi [0], tempi [1])++;
                else if (tempi [2] == 194)
                    ntrips1940 (tempi [0], tempi [1])++;
                else if (tempi [2] == 195)
                    ntrips1950 (tempi [0], tempi [1])++;
                else if (tempi [2] == 196)
                    ntrips1960 (tempi [0], tempi [1])++;
                else if (tempi [2] == 197)
                    ntrips1970 (tempi [0], tempi [1])++;
                else if (tempi [2] == 198)
                    ntrips1980 (tempi [0], tempi [1])++;
                else if (tempi [2] == 199)
                    ntrips1990 (tempi [0], tempi [1])++;
                else if (tempi [2] == 200)
                    ntrips2000 (tempi [0], tempi [1])++;
            }
            else
            {
                linetxt = linetxt.substr (ipos + 3, linetxt.length () - ipos - 1);
                tempi [3] = atoi (linetxt.substr (0, 1).c_str()); // Gender
                if (usertype != "Subscriber")
                    ntrips_cust (tempi [0], tempi [1])++;
                else
                    if (tempi [3] == 1)
                        ntrips_sub_m (tempi [0], tempi [1])++;
                    else if (tempi [3] == 2)
                        ntrips_sub_f (tempi [0], tempi [1])++;
                    else
                        ntrips_sub_n (tempi [0], tempi [1])++;
            }
            count++; 
        } // end if stations in StnIndxLen
    } // end while getline
    in_file.close();
    Rcpp::Rcout << " and " << count << " valid trips." << std::endl;

    return count;
} // end readOneFileNYC


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                           SUMMARYSTATS                             **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

void RideData::summaryStats ()
{
    // Average Age
    int tempi [2] = {0,0};
    for (int i=0; i<99; i++)
    {
        tempi [0] += i * ageDistribution [i];
        tempi [1] += ageDistribution [i];
    }
    Rcpp::Rcout << "Average Age = " << (double) tempi [0] / (double) tempi [1] << 
        std::endl;

    if (RideData::getSubscriber () < 4)
    {
        // Male-Female ratio
        tempi [0] = tempi [1] = 0;
        int numStations = RideData::returnNumStations ();
        for (int i=0; i<numStations; i++)
            for (int j=0; j<numStations; j++)
            {
                tempi [0] += ntrips_sub_f (i, j);
                tempi [1] += ntrips_sub_m (i, j);
            }
        Rcpp::Rcout << "Female/Male ratio = " << (double) tempi [0] /
            (double) tempi [1] << std::endl;
    }
} // end summaryStats



/************************************************************************
 ************************************************************************
 **                                                                    **
 **                          AGGREGATETRIPS                            **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

int RideData::aggregateTrips ()
{
    int numStations = RideData::returnNumStations (),
        subscriber = RideData::getSubscriber (),
        gender = RideData::getGender ();
    // subscriber = (0, 1, 2) for (all, subscriber, customer)
    // gender = (0, 1, 2) for (all, male, female)
    // (male, female) only make sense for subscriber = 1, and are ignored
    // otherwise.
    if (subscriber == 0 || subscriber == 3)
    {
        for (int i=0; i<numStations; i++)
            for (int j=0; j<numStations; j++)
                ntrips (i, j) += (double) ntrips_sub_f (i, j) +
                    (double) ntrips_sub_m (i, j) +
                    (double) ntrips_sub_n (i, j) +
                    (double) ntrips_cust (i, j);

    }
    else if (subscriber == 1)
    {
        if (gender == 0)
            for (int i=0; i<numStations; i++)
                for (int j=0; j<numStations; j++)
                    ntrips (i, j) += (double) ntrips_sub_f (i, j) +
                        (double) ntrips_sub_m (i, j);
        else if (gender == 1)
            for (int i=0; i<numStations; i++)
                for (int j=0; j<numStations; j++)
                    ntrips (i, j) += (double) ntrips_sub_m (i, j);
        else if (gender == 2)
            for (int i=0; i<numStations; i++)
                for (int j=0; j<numStations; j++)
                    ntrips (i, j) += (double) ntrips_sub_f (i, j);
    }
    else if (subscriber == 2)
    {
        for (int i=0; i<numStations; i++)
            for (int j=0; j<numStations; j++)
                ntrips (i, j) += (double) ntrips_cust (i, j);
    }
    else
    {
        if (gender == 0) // "young"
            for (int i=0; i<numStations; i++)
                for (int j=0; j<numStations; j++)
                    ntrips (i, j) += (double) ntripsYoung (i, j);
        else if (gender == 1) // "old"
            for (int i=0; i<numStations; i++)
                for (int j=0; j<numStations; j++)
                    ntrips (i, j) += (double) ntripsOld (i, j);
        else if (gender == 1920)
            for (int i=0; i<numStations; i++)
                for (int j=0; j<numStations; j++)
                    ntrips (i, j) += (double) ntrips1920 (i, j);
        else if (gender == 1930)
            for (int i=0; i<numStations; i++)
                for (int j=0; j<numStations; j++)
                    ntrips (i, j) += (double) ntrips1930 (i, j);
        else if (gender == 1940)
            for (int i=0; i<numStations; i++)
                for (int j=0; j<numStations; j++)
                    ntrips (i, j) += (double) ntrips1940 (i, j);
        else if (gender == 1950)
            for (int i=0; i<numStations; i++)
                for (int j=0; j<numStations; j++)
                    ntrips (i, j) += (double) ntrips1950 (i, j);
        else if (gender == 1960)
            for (int i=0; i<numStations; i++)
                for (int j=0; j<numStations; j++)
                    ntrips (i, j) += (double) ntrips1960 (i, j);
        else if (gender == 1970)
            for (int i=0; i<numStations; i++)
                for (int j=0; j<numStations; j++)
                    ntrips (i, j) += (double) ntrips1970 (i, j);
        else if (gender == 1980)
            for (int i=0; i<numStations; i++)
                for (int j=0; j<numStations; j++)
                    ntrips (i, j) += (double) ntrips1980 (i, j);
        else if (gender == 1990)
            for (int i=0; i<numStations; i++)
                for (int j=0; j<numStations; j++)
                    ntrips (i, j) += (double) ntrips1990 (i, j);
        else if (gender == 2000)
            for (int i=0; i<numStations; i++)
                for (int j=0; j<numStations; j++)
                    ntrips (i, j) += (double) ntrips2000 (i, j);
        else // default should not happen!
            for (int i=0; i<numStations; i++)
                for (int j=0; j<numStations; j++)
                    ntrips (i, j) += (double) ntrips1920 (i, j) +
                    (double) ntrips1930 (i, j) + (double) ntrips1940 (i, j) +
                    (double) ntrips1950 (i, j) + (double) ntrips1960 (i, j) +
                    (double) ntrips1970 (i, j) + (double) ntrips1980 (i, j) +
                    (double) ntrips1990 (i, j) + (double) ntrips2000 (i, j);
    }

    return 0;
} // end aggregateTrips


