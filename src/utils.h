#pragma once

#include <string>
#include <vector>
#include <map>
#include <stdio.h>

// [[Rcpp::depends(BH)]]
#include <Rcpp.h>

#include <boost/algorithm/string/replace.hpp>

char *strtokm(char *str, const char *delim);
std::string str_token (std::string * line, const char * delim);
void rm_dos_end (char *str);
bool line_has_quotes (char * line);
std::string convert_datetime_ny (std::string str);
std::string convert_datetime_ch (std::string str);
std::string convert_datetime_nabsa (std::string str);
std::string convert_datetime_dc (std::string str);
std::string convert_datetime_lo (std::string str);
