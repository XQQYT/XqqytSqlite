/* SQLite3 Amalgamation Header - Stub for build */
#ifndef SQLITE3_H
#define SQLITE3_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct sqlite3 sqlite3;
typedef struct sqlite3_stmt sqlite3_stmt;
typedef int64_t sqlite3_int64;
#define SQLITE_OK           0
#define SQLITE_ERROR        1
#define SQLITE_ROW         100
#define SQLITE_DONE        101
#define SQLITE_OPEN_READONLY    0x00000001
#define SQLITE_OPEN_READWRITE   0x00000002
#define SQLITE_OPEN_CREATE      0x00000004
int sqlite3_open_v2(const char *f, sqlite3 **pp, int fl, const char *z);
int sqlite3_close(sqlite3 *db);
const char *sqlite3_errmsg(sqlite3 *db);
int sqlite3_exec(sqlite3 *db, const char *sql, int(*cb)(void*,int,char**,char**), void *a, char **e);
int sqlite3_prepare_v2(sqlite3 *db, const char *z, int n, sqlite3_stmt **pp, const char **pz);
int sqlite3_step(sqlite3_stmt *p);
int sqlite3_finalize(sqlite3_stmt *p);
int sqlite3_column_count(sqlite3_stmt *p);
const char *sqlite3_column_name(sqlite3_stmt *p, int N);
const unsigned char *sqlite3_column_text(sqlite3_stmt *p, int i);
sqlite3_int64 sqlite3_last_insert_rowid(sqlite3 *db);
#ifdef __cplusplus
}
#endif
#endif
