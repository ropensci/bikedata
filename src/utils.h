#include <string>
#include <vector>
#include <map>
#include <sqlite3.h>
#include <spatialite.h>

// [[Rcpp::depends(BH)]]
#include <Rcpp.h>

#include <boost/algorithm/string/replace.hpp>

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

//' compare_version_numbers
//'
//' Function to compare version numbers
//' First argument is compared to the second argument
//' Return value:
//' -1 = Argument one version lower than Argument two version
//' 0 = Argument one version equal to Argument two version
//' 1 = Argument one version higher than Argument two version
//'
//' @noRd
int compare_version_numbers (std::string vstro, std::string compvstro) {
  
  int versiondiff = 0;
  
  char *vstr = (char *)vstro.c_str();
  char *compvstr = (char *)compvstro.c_str();
  
  char *vstrtok, *compvstrtok;
  char *vstrtokptr, *compvstrtokptr;
  
  vstrtok = strtok_r(vstr, ".", &vstrtokptr);
  compvstrtok = strtok_r (compvstr, ".", &compvstrtokptr);

  if (atoi(vstrtok) < atoi(compvstrtok)) {
    versiondiff = -1;
  }
  else if (atoi(vstrtok) > atoi(compvstrtok)) {
    versiondiff = 1;
  }
  else {
    while (vstrtok != NULL && compvstrtok != NULL && versiondiff == 0) {

      vstrtok = strtok_r (NULL, ".", &vstrtokptr);
      compvstrtok = strtok_r (NULL, ".", &compvstrtokptr);

      if (vstrtok == NULL && compvstrtok == NULL) {
        versiondiff = 0;
      }
      else if (vstrtok == NULL && compvstrtok != NULL) {
        if (atoi(compvstrtok) == 0) {
          versiondiff = 0;
        }
        else {
          versiondiff = -1;
        }
      }
      else if (vstrtok != NULL && compvstrtok == NULL) {
        if (atoi(vstrtok) == 0) {
          versiondiff = 0;
        }
        else {
          versiondiff = 1;
        }
      }
      else if (atoi(vstrtok) < atoi(compvstrtok)) {
        versiondiff = -1;
      }
      else if (atoi(vstrtok) > atoi(compvstrtok)) {
        versiondiff = 1;
      }

    }

  }
  
  return versiondiff;

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

//' convert_datetime
//'
//' Datetime strings for NYC change between 08/2014 and 09/2014 from
//' yyyy-mm-dd HH:MM:SS to m/d/yyyy HH:MM:SS. sqlite3 can't combine dates in
//' different formats, so this converts the latter to former formats.
//'
//' @noRd
std::string convert_datetime (std::string str)
{
    // NOTE that the following does not work for some reason?
    //if (size_t ipos = str.find ("/") != std::string::npos)
    if (str.find ("/") != std::string::npos)
    {
        size_t ipos = str.find ("/");
        std::string mm = str.substr (0, ipos);
        if (ipos == 1)
            mm = std::string ("0") + mm;
        str = str.substr (ipos + 1, str.length () - ipos - 1);
        ipos = str.find ("/");
        std::string dd = str.substr (0, ipos);
        if (ipos == 1)
            dd = std::string ("0") + dd;
        str = str.substr (ipos + 1, str.length () - ipos - 1);
        ipos = str.find (" ");
        std::string yy = str.substr (0, ipos);
        str = str.substr (ipos + 1, str.length () - ipos - 1);
        str = yy + "-" + mm + "-" + dd + " " + str;
    }

    return str;
}

