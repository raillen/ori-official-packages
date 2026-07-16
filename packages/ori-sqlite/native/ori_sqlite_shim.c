#include "sqlite3.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_DBS 32
#define MAX_STMTS 64
#define TEXT_BUF 8192

static sqlite3 *g_dbs[MAX_DBS];
static sqlite3_stmt *g_stmts[MAX_STMTS];
static char g_last_err[512];
static char g_text[TEXT_BUF];

static int alloc_db(void) {
    for (int i = 0; i < MAX_DBS; ++i) {
        if (g_dbs[i] == NULL) return i;
    }
    return -1;
}

static int alloc_stmt(void) {
    for (int i = 0; i < MAX_STMTS; ++i) {
        if (g_stmts[i] == NULL) return i;
    }
    return -1;
}

static void set_err(const char *msg) {
    if (!msg) {
        g_last_err[0] = '\0';
        return;
    }
    strncpy(g_last_err, msg, sizeof(g_last_err) - 1);
    g_last_err[sizeof(g_last_err) - 1] = '\0';
}

static sqlite3 *db_at(int db) {
    if (db < 0 || db >= MAX_DBS) return NULL;
    return g_dbs[db];
}

static sqlite3_stmt *stmt_at(int st) {
    if (st < 0 || st >= MAX_STMTS) return NULL;
    return g_stmts[st];
}

int ori_sqlite_open(int path_ptr) {
    const char *path = (const char *)(intptr_t)path_ptr;
    if (!path) {
        set_err("null path");
        return -1;
    }
    int slot = alloc_db();
    if (slot < 0) {
        set_err("too many open databases");
        return -1;
    }
    sqlite3 *db = NULL;
    int rc = sqlite3_open(path, &db);
    if (rc != SQLITE_OK) {
        set_err(db ? sqlite3_errmsg(db) : "open failed");
        if (db) sqlite3_close(db);
        return -1;
    }
    g_dbs[slot] = db;
    set_err(NULL);
    return slot;
}

void ori_sqlite_close(int db) {
    if (db < 0 || db >= MAX_DBS || !g_dbs[db]) return;
    /* Finalize any stmts that belong to this connection (best-effort). */
    for (int i = 0; i < MAX_STMTS; ++i) {
        if (g_stmts[i] && sqlite3_db_handle(g_stmts[i]) == g_dbs[db]) {
            sqlite3_finalize(g_stmts[i]);
            g_stmts[i] = NULL;
        }
    }
    sqlite3_close(g_dbs[db]);
    g_dbs[db] = NULL;
}

int ori_sqlite_exec(int db, int sql_ptr) {
    sqlite3 *conn = db_at(db);
    if (!conn) {
        set_err("invalid db");
        return 1;
    }
    const char *sql = (const char *)(intptr_t)sql_ptr;
    if (!sql) {
        set_err("null sql");
        return 1;
    }
    char *errmsg = NULL;
    int rc = sqlite3_exec(conn, sql, NULL, NULL, &errmsg);
    if (rc != SQLITE_OK) {
        set_err(errmsg ? errmsg : sqlite3_errmsg(conn));
    } else {
        set_err(NULL);
    }
    if (errmsg) sqlite3_free(errmsg);
    return rc == SQLITE_OK ? 0 : rc;
}

int ori_sqlite_query_int(int db, int sql_ptr) {
    sqlite3 *conn = db_at(db);
    if (!conn) {
        set_err("invalid db");
        return 0;
    }
    const char *sql = (const char *)(intptr_t)sql_ptr;
    sqlite3_stmt *st = NULL;
    if (sqlite3_prepare_v2(conn, sql, -1, &st, NULL) != SQLITE_OK) {
        set_err(sqlite3_errmsg(conn));
        return 0;
    }
    int value = 0;
    int step = sqlite3_step(st);
    if (step == SQLITE_ROW) {
        value = sqlite3_column_int(st, 0);
        set_err(NULL);
    } else if (step != SQLITE_DONE) {
        set_err(sqlite3_errmsg(conn));
    } else {
        set_err("no row");
    }
    sqlite3_finalize(st);
    return value;
}

int ori_sqlite_query_text_to_path(int db, int sql_ptr, int path_ptr) {
    sqlite3 *conn = db_at(db);
    if (!conn) {
        set_err("invalid db");
        return 1;
    }
    const char *sql = (const char *)(intptr_t)sql_ptr;
    const char *path = (const char *)(intptr_t)path_ptr;
    if (!sql || !path) {
        set_err("null arg");
        return 1;
    }
    sqlite3_stmt *st = NULL;
    if (sqlite3_prepare_v2(conn, sql, -1, &st, NULL) != SQLITE_OK) {
        set_err(sqlite3_errmsg(conn));
        return 1;
    }
    int rc = 1;
    if (sqlite3_step(st) == SQLITE_ROW) {
        const unsigned char *t = sqlite3_column_text(st, 0);
        FILE *f = fopen(path, "wb");
        if (f) {
            if (t) fputs((const char *)t, f);
            fclose(f);
            rc = 0;
            set_err(NULL);
        } else {
            set_err("cannot write text path");
        }
    } else {
        set_err("no row");
    }
    sqlite3_finalize(st);
    return rc;
}

int ori_sqlite_last_insert_rowid(int db) {
    sqlite3 *conn = db_at(db);
    if (!conn) return 0;
    return (int)sqlite3_last_insert_rowid(conn);
}

int ori_sqlite_changes(int db) {
    sqlite3 *conn = db_at(db);
    if (!conn) return 0;
    return sqlite3_changes(conn);
}

int ori_sqlite_errmsg_to_path(int path_ptr) {
    const char *path = (const char *)(intptr_t)path_ptr;
    if (!path) return 1;
    FILE *f = fopen(path, "wb");
    if (!f) return 1;
    fputs(g_last_err, f);
    fclose(f);
    return 0;
}

int ori_sqlite_has_error(void) {
    return g_last_err[0] != '\0' ? 1 : 0;
}

/* ---- prepared statements ---- */

int ori_sqlite_prepare(int db, int sql_ptr) {
    sqlite3 *conn = db_at(db);
    if (!conn) {
        set_err("invalid db");
        return -1;
    }
    const char *sql = (const char *)(intptr_t)sql_ptr;
    if (!sql) {
        set_err("null sql");
        return -1;
    }
    int slot = alloc_stmt();
    if (slot < 0) {
        set_err("too many prepared statements");
        return -1;
    }
    sqlite3_stmt *st = NULL;
    int rc = sqlite3_prepare_v2(conn, sql, -1, &st, NULL);
    if (rc != SQLITE_OK) {
        set_err(sqlite3_errmsg(conn));
        return -1;
    }
    g_stmts[slot] = st;
    set_err(NULL);
    return slot;
}

int ori_sqlite_bind_int(int stmt, int idx, int value) {
    sqlite3_stmt *st = stmt_at(stmt);
    if (!st) {
        set_err("invalid stmt");
        return 1;
    }
    int rc = sqlite3_bind_int(st, idx, value);
    if (rc != SQLITE_OK) {
        set_err(sqlite3_errmsg(sqlite3_db_handle(st)));
        return rc;
    }
    set_err(NULL);
    return 0;
}

int ori_sqlite_bind_text(int stmt, int idx, int text_ptr) {
    sqlite3_stmt *st = stmt_at(stmt);
    if (!st) {
        set_err("invalid stmt");
        return 1;
    }
    const char *text = (const char *)(intptr_t)text_ptr;
    if (!text) text = "";
    /* SQLITE_TRANSIENT: copy; Ori string may not outlive bind. */
    int rc = sqlite3_bind_text(st, idx, text, -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        set_err(sqlite3_errmsg(sqlite3_db_handle(st)));
        return rc;
    }
    set_err(NULL);
    return 0;
}

/* Returns SQLITE_ROW (100), SQLITE_DONE (101), or error code. */
int ori_sqlite_step(int stmt) {
    sqlite3_stmt *st = stmt_at(stmt);
    if (!st) {
        set_err("invalid stmt");
        return 1;
    }
    int rc = sqlite3_step(st);
    if (rc == SQLITE_ROW || rc == SQLITE_DONE) {
        set_err(NULL);
    } else {
        set_err(sqlite3_errmsg(sqlite3_db_handle(st)));
    }
    return rc;
}

int ori_sqlite_column_int(int stmt, int col) {
    sqlite3_stmt *st = stmt_at(stmt);
    if (!st) return 0;
    return sqlite3_column_int(st, col);
}

int ori_sqlite_column_text_to_path(int stmt, int col, int path_ptr) {
    sqlite3_stmt *st = stmt_at(stmt);
    if (!st) {
        set_err("invalid stmt");
        return 1;
    }
    const char *path = (const char *)(intptr_t)path_ptr;
    if (!path) {
        set_err("null path");
        return 1;
    }
    const unsigned char *t = sqlite3_column_text(st, col);
    FILE *f = fopen(path, "wb");
    if (!f) {
        set_err("cannot write column path");
        return 1;
    }
    if (t) fputs((const char *)t, f);
    fclose(f);
    set_err(NULL);
    return 0;
}

int ori_sqlite_column_count(int stmt) {
    sqlite3_stmt *st = stmt_at(stmt);
    if (!st) return 0;
    return sqlite3_column_count(st);
}

int ori_sqlite_reset(int stmt) {
    sqlite3_stmt *st = stmt_at(stmt);
    if (!st) {
        set_err("invalid stmt");
        return 1;
    }
    int rc = sqlite3_reset(st);
    if (rc != SQLITE_OK) set_err(sqlite3_errmsg(sqlite3_db_handle(st)));
    else set_err(NULL);
    return rc == SQLITE_OK ? 0 : rc;
}

int ori_sqlite_clear_bindings(int stmt) {
    sqlite3_stmt *st = stmt_at(stmt);
    if (!st) {
        set_err("invalid stmt");
        return 1;
    }
    int rc = sqlite3_clear_bindings(st);
    if (rc != SQLITE_OK) set_err(sqlite3_errmsg(sqlite3_db_handle(st)));
    else set_err(NULL);
    return rc == SQLITE_OK ? 0 : rc;
}

void ori_sqlite_finalize(int stmt) {
    if (stmt < 0 || stmt >= MAX_STMTS || !g_stmts[stmt]) return;
    sqlite3_finalize(g_stmts[stmt]);
    g_stmts[stmt] = NULL;
}

/* Multi-row helper: write JSON array of objects from first N text/int columns.
 * Simple format: [{"c0":..,"c1":"..."}, ...] — ints unquoted, text escaped lightly.
 * Returns 0 on success. Max rows: 256. */
int ori_sqlite_query_all_to_json_path(int db, int sql_ptr, int path_ptr) {
    sqlite3 *conn = db_at(db);
    if (!conn) {
        set_err("invalid db");
        return 1;
    }
    const char *sql = (const char *)(intptr_t)sql_ptr;
    const char *path = (const char *)(intptr_t)path_ptr;
    if (!sql || !path) {
        set_err("null arg");
        return 1;
    }
    sqlite3_stmt *st = NULL;
    if (sqlite3_prepare_v2(conn, sql, -1, &st, NULL) != SQLITE_OK) {
        set_err(sqlite3_errmsg(conn));
        return 1;
    }
    FILE *f = fopen(path, "wb");
    if (!f) {
        set_err("cannot write json path");
        sqlite3_finalize(st);
        return 1;
    }
    fputc('[', f);
    int cols = sqlite3_column_count(st);
    int first = 1;
    int rows = 0;
    while (sqlite3_step(st) == SQLITE_ROW && rows < 256) {
        if (!first) fputc(',', f);
        first = 0;
        fputc('{', f);
        for (int c = 0; c < cols; ++c) {
            if (c) fputc(',', f);
            fprintf(f, "\"c%d\":", c);
            int type = sqlite3_column_type(st, c);
            if (type == SQLITE_INTEGER) {
                fprintf(f, "%d", sqlite3_column_int(st, c));
            } else if (type == SQLITE_NULL) {
                fputs("null", f);
            } else {
                const unsigned char *t = sqlite3_column_text(st, c);
                fputc('"', f);
                if (t) {
                    for (const unsigned char *p = t; *p; ++p) {
                        if (*p == '"' || *p == '\\') fputc('\\', f);
                        if (*p == '\n') { fputs("\\n", f); continue; }
                        fputc((char)*p, f);
                    }
                }
                fputc('"', f);
            }
        }
        fputc('}', f);
        rows++;
    }
    fputc(']', f);
    fclose(f);
    sqlite3_finalize(st);
    set_err(NULL);
    return 0;
}
