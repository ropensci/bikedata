#include "common.h"
#include "utils.h"

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

    if (delim == nullptr) return nullptr;

    tok = (str) ? str : next;
    if (tok == nullptr) return nullptr;

    m = strstr(tok, delim);

    if (m) {
        next = m + strlen(delim);
        *m = '\0';
    } else {
        next = nullptr;
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
    size_t ipos = line->find (delim, 0);
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
    size_t slen = strlen (line);
    char * b;
    b = static_cast <char*> (line);
    char ch;
    for (size_t i = 0; i < slen; ++i)
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

//' convert_datetime_nabsa
//'
//' North American Bike Share Association (LA and Philadelphia) have identical
//' formats, but they still change from M/D/YYYY H:MM to a more regular
//' YYYY-MM-DD HH:MM:SS.
//'
//' @noRd
std::string convert_datetime_nabsa (std::string str)
{
    if (str.find ("-") != std::string::npos)
    {
        std::string yy = str_token (&str, "-");
        std::string mon = str_token (&str, "-");
        std::string dd = str_token (&str, " ");
        std::string hh = str_token (&str, ":");
        std::string mm = str_token (&str, ":");
        str = yy + "-" + mon + "-" + dd + " " + hh + ":" + mm + ":" + str;
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
        std::string mm = str;
        str = yy + "-" + mon + "-" + dd + " " + hh + ":" + mm + ":00";
    }

    // Later format YYYY-MM-DD ... are double-quoted, so remove these:
    if (str.front () == '"')
    {
        str.erase (0, 1);
        str.erase (str.size () - 1);
    }

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

//' Difference between two time strings formatted as
//' YYYY-MM-DD hh:mm:ss
//' @param t1 start date-time string
//' @param t2 end date-time string
//' @noRd
int timediff (std::string t1, std::string t2)
{
    if (t1.length () < 19)
    {
        Rcpp::Rcout << "---[" << t1 << "]---" << std::endl;
        Rcpp::stop ("nope");
    }
    int Y1 = atoi (t1.substr (0, 4).c_str ()),
        M1 = atoi (t1.substr (5, 2).c_str ()),
        D1 = atoi (t1.substr (8, 2).c_str ()),
        h1 = atoi (t1.substr (11, 2).c_str ()),
        m1 = atoi (t1.substr (14, 2).c_str ()),
        s1 = atoi (t1.substr (17, 2).c_str ()),
        Y2 = atoi (t2.substr (0, 4).c_str ()),
        M2 = atoi (t2.substr (5, 2).c_str ()),
        D2 = atoi (t2.substr (8, 2).c_str ()),
        h2 = atoi (t2.substr (11, 2).c_str ()),
        m2 = atoi (t2.substr (14, 2).c_str ()),
        s2 = atoi (t2.substr (17, 2).c_str ());

    int y1 = daynum (Y1, M1, D1), y2 = daynum (Y2, M2, D2);
    int d1 = y1 * 3600 * 24 + h1 * 3600 + m1 * 60 + s1,
        d2 = y2 * 3600 * 24 + h2 * 3600 + m2 * 60 + s2;

    return d2 - d1;
}

// Julian day number calculation from
// http://www.cs.utsa.edu/~cs1063/projects/Spring2011/Project1/jdn-explanation.html
int daynum (int y, int m, int d)
{
    int a = floor ((14 - m) / 12);
    y = y + 4800 - a;
    m = m + 12 * a - 3;
    int res = d + floor ((153 * m + 2) / 5) + 365 * y +
        floor (y / 4) - floor (y / 100) + floor (y / 400) - 32045;
    return res;
}
