#include <R.h>
#include <Rinternals.h>
#include <stdlib.h> // for NULL
#include <R_ext/Rdynload.h>

/* .Call calls */
extern SEXP bikedata_rcpp_create_city_index(SEXP, SEXP);
extern SEXP bikedata_rcpp_create_db_indexes(SEXP, SEXP, SEXP, SEXP);
extern SEXP bikedata_rcpp_create_sqlite3_db(SEXP);
extern SEXP bikedata_rcpp_import_stn_df(SEXP, SEXP, SEXP);
extern SEXP bikedata_rcpp_import_to_file_table(SEXP, SEXP, SEXP, SEXP);
extern SEXP bikedata_rcpp_import_to_trip_table(SEXP, SEXP, SEXP, SEXP);

static const R_CallMethodDef CallEntries[] = {
    {"bikedata_rcpp_create_city_index",    (DL_FUNC) &bikedata_rcpp_create_city_index,    2},
    {"bikedata_rcpp_create_db_indexes",    (DL_FUNC) &bikedata_rcpp_create_db_indexes,    4},
    {"bikedata_rcpp_create_sqlite3_db",    (DL_FUNC) &bikedata_rcpp_create_sqlite3_db,    1},
    {"bikedata_rcpp_import_stn_df",        (DL_FUNC) &bikedata_rcpp_import_stn_df,        3},
    {"bikedata_rcpp_import_to_file_table", (DL_FUNC) &bikedata_rcpp_import_to_file_table, 4},
    {"bikedata_rcpp_import_to_trip_table", (DL_FUNC) &bikedata_rcpp_import_to_trip_table, 4},
    {NULL, NULL, 0}
};

void R_init_bikedata(DllInfo *dll)
{
    R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
    R_useDynamicSymbols(dll, FALSE);
}
