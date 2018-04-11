#pragma once

// [[Rcpp::depends(BH)]]
#include <Rcpp.h>

char *strtokm(char *str, const char *delim);
std::string str_token (std::string * line, const char * delim);
void rm_dos_end (char *str);
bool line_has_quotes (char * line);
std::string convert_datetime_ny (std::string str);
std::string convert_datetime_ch (std::string str);
std::string convert_datetime_nabsa (std::string str);
std::string convert_datetime_dc (std::string str);
std::string convert_datetime_lo (std::string str);

std::string convert_datetime_generic (std::string str);
bool date_is_standard (const std::string ymd);
bool time_is_standard (const std::string hms);
std::string convert_date (std::string ymd);
std::string convert_time (std::string hms);
void zero_pad (std::string &t);

int timediff (std::string t1, std::string t2);
int daynum (int y, int m, int d);
