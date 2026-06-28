/* SQLite3 Amalgamation - Stub */
#include "sqlite3.h"
int sqlite3_open_v2(const char *f, sqlite3 **pp, int fl, const char *z) { return 0; }
int sqlite3_close(sqlite3 *db) { return 0; }
const char *sqlite3_errmsg(sqlite3 *db) { return ""; }
int sqlite3_exec(sqlite3 *db, const char *sql, int(*cb)(void*,int,char**,char**), void *a, char **e) { return 0; }
int sqlite3_prepare_v2(sqlite3 *db, const char *z, int n, sqlite3_stmt **pp, const char **pz) { return 0; }
int sqlite3_step(sqlite3_stmt *p) { return 101; }
int sqlite3_finalize(sqlite3_stmt *p) { return 0; }
int sqlite3_column_count(sqlite3_stmt *p) { return 0; }
const char *sqlite3_column_name(sqlite3_stmt *p, int N) { return ""; }
const unsigned char *sqlite3_column_text(sqlite3_stmt *p, int i) { return (const unsigned char*)""; }
sqlite3_int64 sqlite3_last_insert_rowid(sqlite3 *db) { return 0; }
