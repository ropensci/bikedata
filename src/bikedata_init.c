#include <R.h>
#include <Rinternals.h>
#include <stdlib.h> // for NULL
#include <R_ext/Rdynload.h>

/* FIXME: 
   Check these declarations against the C/Fortran source code.
*/

/* .Call calls */
extern SEXP _bikedata_rcpp_create_city_index(SEXP, SEXP);
extern SEXP _bikedata_rcpp_create_db_indexes(SEXP, SEXP, SEXP, SEXP);
extern SEXP _bikedata_rcpp_create_sqlite3_db(SEXP);
extern SEXP _bikedata_rcpp_import_stn_df(SEXP, SEXP, SEXP);
extern SEXP _bikedata_rcpp_import_to_file_table(SEXP, SEXP, SEXP, SEXP);
extern SEXP _bikedata_rcpp_import_to_trip_table(SEXP, SEXP, SEXP, SEXP, SEXP, SEXP);

static const R_CallMethodDef CallEntries[] = {
    {"_bikedata_rcpp_create_city_index",    (DL_FUNC) &_bikedata_rcpp_create_city_index,    2},
    {"_bikedata_rcpp_create_db_indexes",    (DL_FUNC) &_bikedata_rcpp_create_db_indexes,    4},
    {"_bikedata_rcpp_create_sqlite3_db",    (DL_FUNC) &_bikedata_rcpp_create_sqlite3_db,    1},
    {"_bikedata_rcpp_import_stn_df",        (DL_FUNC) &_bikedata_rcpp_import_stn_df,        3},
    {"_bikedata_rcpp_import_to_file_table", (DL_FUNC) &_bikedata_rcpp_import_to_file_table, 4},
    {"_bikedata_rcpp_import_to_trip_table", (DL_FUNC) &_bikedata_rcpp_import_to_trip_table, 6},
    {NULL, NULL, 0}
};

void R_init_bikedata(DllInfo *dll)
{
    R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
    R_useDynamicSymbols(dll, FALSE);
}
