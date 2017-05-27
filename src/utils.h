#pragma once

#include <string>
#include <vector>
#include <map>
#include <stdio.h>

// [[Rcpp::depends(BH)]]
#include <Rcpp.h>

#include <boost/algorithm/string/replace.hpp>

// https://github.com/RcppCore/Rcpp/issues/636
void R_init_bikedata(DllInfo* info) {
    R_registerRoutines(info, NULL, NULL, NULL, NULL);
    R_useDynamicSymbols(info, TRUE);
}

//' strtokm
//'
//' A string delimiter function based on strtok
//' Accessed from StackOverflow (using M Oehm):
//' http://stackoverflow.com/questions/29847915/implementing-strtok-whose-delimiter-has-more-than-one-character
//'
//' @noRd
char *strtokm(char *str, const char *delim)
{
    static char *tok;
    static char *next;
    char *m;

    if (delim == NULL) return NULL;

    tok = (str) ? str : next;
    if (tok == NULL) return NULL;

    m = strstr(tok, delim);

    if (m) {
        next = m + strlen(delim);
        *m = '\0';
    } else {
        next = NULL;
    }

    return tok;
}

//' str_token
//'
//' A delimiter function for comma-separated std::string
//'
//' @param line The line of text to be tokenised
//' @param delim The desired delimiter
//'
//' @return Next token
//'
//' @noRd
std::string str_token (std::string * line, const char * delim)
{
    unsigned ipos = line->find (delim, 0);
    std::string res = line->substr (0, ipos);
    (*line) = line->substr (ipos + 1, line->length () - ipos - 1);
    return res;
}


//' rm_dos_end
//'
//' Remove dos line ending from a character string
//'
//' @noRd
void rm_dos_end (char *str)
{
    char *p = strrchr (str, '\r');
    if (p && p[1]=='\n' && p[2]=='\0') 
        p[0] = '\0';
}

//' line_has_quotes
//'
//' Determine whether or not fields within a line are separated by double quotes
//' and a comma, or just comma separated.
//'
//' @param line Character string with or without double-quote-comma separation
//'
//' @return true if line is delimited by double quotes and commas, false if
//' commas only.
//'
//' @noRd
bool line_has_quotes (char * line)
{
    bool has_quotes = false;
    unsigned slen = strlen (line);
    char * b;
    b = (char*) line;
    char ch;
    for (unsigned i = 0; i<slen; ++i)
    {
        strncpy (&ch, b+i, 1);
        if (ch == '\"')
        {
            has_quotes = true;
            break;
        }
    }
    return has_quotes;
}

//' convert_datetime_ny
//'
//' Datetime strings for NYC change between 08/2014 and 09/2014 from
//' yyyy-mm-dd HH:MM:SS to either m/d/yyyy HH:MM:SS or m/d/yyyy H:M
//'
//' @noRd
std::string convert_datetime_ny (std::string str)
{
    // NOTE that the following does not work for some reason?
    //if (size_t ipos = str.find ("/") != std::string::npos)
    if (str.find ("/") != std::string::npos)
    {
        std::string mon = str_token (&str, "/");
        if (mon.length () == 1)
            mon = "0" + mon;
        std::string dd = str_token (&str, "/");
        if (dd.length () == 1)
            dd = "0" + dd;
        std::string yy = str_token (&str, " ");
        std::string hh = str_token (&str, ":");
        if (hh.length () == 1)
            hh = "0" + hh;
        std::string mm, ss = "00";
        if (str.find (":") != std::string::npos)
        {
            mm = str_token (&str, ":");
            ss = str;
        } else
            mm = str;
            
        str = yy + "-" + mon + "-" + dd + " " + hh + ":" + str;
    }

    return str;
}

//' convert_datetime_ch
//'
//' Datetime strings for Chicago vary between the following formats:
//' YYYY-MM-DD hh:mm
//' M/D/YYYY hh:mm
//' M/D/YYYY hh:mm:ss
//'
//' @noRd
std::string convert_datetime_ch (std::string str)
{
    // NOTE that the following does not work for some reason?
    //if (size_t ipos = str.find ("/") != std::string::npos)
    if (str.find ("-") != std::string::npos)
    {
        std::string yy = str_token (&str, "-");
        std::string mon = str_token (&str, "-");
        std::string dd = str_token (&str, " ");
        std::string hh = str_token (&str, ":");
        str = yy + "-" + mon + "-" + dd + " " + hh + ":" + str + ":00";
    } else if (str.find ("/") != std::string::npos)
    {
        std::string mon = str_token (&str, "/");
        if (mon.length () == 1)
            mon = "0" + mon;
        std::string dd = str_token (&str, "/");
        if (dd.length () == 1)
            dd = "0" + dd;
        std::string yy = str_token (&str, " ");
        std::string hh = str_token (&str, ":");
        if (hh.length () == 1)
            hh = "0" + hh;
        std::string mm, ss = "00";
        if (str.find (":") == std::string::npos)
            mm = str;
        else
        {
            mm = str_token (&str, ":");
            ss = str;
        }
        str = yy + "-" + mon + "-" + dd + " " + hh + ":" + mm + ":" + ss;
    }

    return str;
}

//' convert_datetime_la
//'
//' @noRd
std::string convert_datetime_la (std::string str)
{
    std::string mon = str_token (&str, "/");
    if (mon.length () == 1)
        mon = "0" + mon;
    std::string dd = str_token (&str, "/");
    if (dd.length () == 1)
        dd = "0" + dd;
    std::string yy = str_token (&str, " ");
    std::string hh = str_token (&str, ":");
    if (hh.length () == 1)
        hh = "0" + hh;
    std::string mm = str;
    str = yy + "-" + mon + "-" + dd + " " + hh + ":" + mm + ":00";

    return str;
}

//' convert_datetime_dc
//'
//' Datetime strings for DC are either M/D/YYYY h:mm, where "mm" is always
//' 0-padded, or YYYY-MM-DD hh:mm, where "hh" is always 0-padded
//'
//' @noRd
std::string convert_datetime_dc (std::string str)
{
    std::string yy, mon, dd, hh;
    if (str.find ("-") != std::string::npos)
    {
        yy = str_token (&str, "-");
        mon = str_token (&str, "-");
        dd = str_token (&str, " ");
        hh = str_token (&str, ":");
    } else if (str.find ("/") != std::string::npos)
    {
        mon = str_token (&str, "/");
        if (mon.length () == 1)
            mon = "0" + mon;
        dd = str_token (&str, "/");
        if (dd.length () == 1)
            dd = "0" + dd;
        yy = str_token (&str, " ");
        hh = str_token (&str, ":");
        if (hh.length () == 1)
            hh = "0" + hh;
    }
    str = yy + "-" + mon + "-" + dd + " " + hh + ":" + str + ":00";

    return str;
}

//' convert_datetime_lo
//'
//' Only issue with London is sometimes seconds are present; sometimes not
//'
//' @noRd
std::string convert_datetime_lo (std::string str)
{
    std::string dd = str_token (&str, "/");
    std::string mon = str_token (&str, "/");
    std::string yy = str_token (&str, " ");
    std::string hh = str_token (&str, ":");
    std::string mm = str, ss = "00";
    if (str.find (":") != std::string::npos)
    {
        mm = str_token (&str, ":");
        ss = str;
    }
    str = yy + "-" + mon + "-" + dd + " " + hh + ":" + mm + ":" + ss;
    // Years 1900-1901 are for test stations - see 
    // "6. Journey Data Extract_27May-23Jun12.csv" and some files also have
    // masses of empty records ("10b... 28Sep14-11Oct14", for example)
    if (yy == "" || yy == "1900" || yy == "1901") 
        str = "";

    return str;
}
