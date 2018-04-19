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

bool strfound (const std::string str, const std::string target)
{
    bool found = false;
    if (str.find (target) != std::string::npos)
        found = true;
    return found;
}

//' convert_datetime
//'
//' Possible formats are:
//' YYYY-mm-dd HH:MM:SS
//' m/d/YYYY HH:MM:SS
//' m/d/YYYY H:M
//' YYYY-mm-dd HH:MM
//' m/d/YYYY HH:MM
//' m/d/YYYY HH:MM:ss
//' m/d/YYYY H:MM
//' YYYY-MM-DD HH:MM:SS.
//' M/D/YYYY H:MM
//' YYYY-MM-DD HH:M
//'
//' @noRd
std::string convert_datetime (std::string str)
{
    std::string ret;
    if (str.find (" ") == std::string::npos)
    {
        ret = "NA";
    } else
    {
        size_t ipos = str.find (" ");
        std::string ymd = str.substr (0, ipos);
        str = str.substr (ipos + 1, str.length () - ipos - 1);

        if (!date_is_standard (ymd))
            ymd = convert_date (ymd);
        if (!time_is_standard (str))
            str = convert_time (str);

        ret = ymd + " " + str;
    }
    return ret;
}

bool date_is_standard (const std::string ymd)
{
    // stardard is yyyy-mm-dd
    bool check = false;
    // std::count counts char not string, so '-', not "-"
    if (ymd.size () == 10 && std::count (ymd.begin(), ymd.end(), '-') == 2)
        check = true;
    return check;
}

bool time_is_standard (const std::string hms)
{
    // stardard is HH:MM:SS
    bool check = false;
    if (hms.size () == 8 && std::count (hms.begin(), hms.end(), ':') == 2)
        check = true;
    return check;
}

std::string convert_date (std::string ymd)
{
    std::string delim = "-";
    if (ymd.find ("/") != std::string::npos)
        delim = "/";

    size_t ipos = ymd.find (delim.c_str ());
    std::string y = ymd.substr (0, ipos);
    ymd = ymd.substr (ipos + 1, ymd.length () - ipos - 1);
    ipos = ymd.find (delim.c_str ());
    std::string m = ymd.substr (0, ipos);
    std::string d = ymd.substr (ipos + 1, ymd.length () - ipos - 1);

    if (d.size () == 4) // change (y,m,d) = m/d/yyyy -> yyyy/m/d
    {
        std::string s = y;
        y = d;
        d = m;
        m = s;
    }
    if (y.size () == 2)
        y = "20" + y;
    zero_pad (m);
    zero_pad (d);

    return y + "-" + m + "-" + d;
}

std::string convert_time (std::string hms)
{
    // Some systems have decimal seconds which are discarded here:
    if (hms.length () > 8 && hms.find (".") != std::string::npos)
        hms = hms.substr (0, hms.find ("."));

    const std::string delim = ":";
    size_t ipos = hms.find (delim.c_str ());
    std::string h = hms.substr (0, ipos), m, s;
    hms = hms.substr (ipos + 1, hms.length () - ipos - 1);
    if (hms.find (delim.c_str ()) != std::string::npos) // has seconds
    {
        ipos = hms.find (delim.c_str ());
        m = hms.substr (0, ipos);
        s = hms.substr (ipos + 1, hms.length () - ipos - 1);
    } else
    {
        m = hms;
        s = "00";
    }
    zero_pad (h);
    zero_pad (m);
    zero_pad (s);
    return h + delim + m + delim + s;
}

void zero_pad (std::string &t)
{
    if (t.size () == 1)
        t = "0" + t;
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
        Rcpp::Rcout << "---[" << t1 << ", " << t2 << "]---" << std::endl;
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
    int a = static_cast <int> (floor ((14 - m) / 12));
    y = y + 4800 - a;
    m = m + 12 * a - 3;
    double res = d + floor ((153 * m + 2) / 5) + 365 * y +
        floor (y / 4) - floor (y / 100) + floor (y / 400) - 32045;
    return static_cast <int> (res);
}
