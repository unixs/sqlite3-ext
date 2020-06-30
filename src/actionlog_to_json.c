#include <string.h>
#include <jansson.h>
#include <omp.h>
#include <stdlib.h>
#include <jemalloc/jemalloc.h>

#include "actionlog_to_json.h"

SQLITE_EXTENSION_INIT1

#define cell(i) (result[offset + i])
#define to_i(i) (strtoll(result[offset + i], NULL, 10))

static char *all_rows_sql = "SELECT * FROM action_log_rows;";

static void ext_test(sqlite3_context *context, int argc, sqlite3_value **argv)
{
    size_t num_threads = omp_get_num_procs();
    sqlite3 *db = sqlite3_user_data(context);
    char *sql_err_msg = NULL;
    char **result;
    char *json;
    int nrows, ncols;

    int rc = sqlite3_get_table(db, all_rows_sql, &result, &nrows, &ncols, &sql_err_msg);

    if (rc != SQLITE_OK) {
        sqlite3_result_error(context, sql_err_msg, strlen(sql_err_msg));
        sqlite3_free(sql_err_msg);

        return;
    }

    json_t **arr = malloc(sizeof(json_t *) * num_threads);

    for (size_t i = 0; i < num_threads; i++) {
        arr[i] = json_array();
    }

    #pragma omp parallel for
    for (int row = 1; row < nrows; row++) {
        int tid = omp_get_thread_num();
        int offset = row * ncols;

        json_t *obj = json_object();

        json_object_set(obj, "id", json_integer(to_i(0)));
        json_object_set(obj, "userClass", json_string(cell(1)));
        json_object_set(obj, "userId", json_integer(to_i(2)));
        json_object_set(obj, "timestamp", json_string(cell(3)));
        json_object_set(obj, "data", json_string(cell(4)));

        json_t *exercise_id;

        if (cell(5) == NULL) {
            exercise_id = json_null();
        }
        else {
            exercise_id = json_integer(to_i(5));
        }
        json_object_set(obj, "gameExerciseId", exercise_id);

        json_array_append_new(arr[tid], obj);
    }

    for (size_t i = 1; i < num_threads; i++) {
        json_array_extend(arr[0], arr[i]);
        json_decref(arr[i]);
    }

    json = json_dumps(arr[0], 0);

    json_decref(arr[0]);
    free(arr);

    // malloc_stats_print(NULL, NULL, NULL);

    sqlite3_free_table(result);
    sqlite3_result_text(context, json, strlen(json), free);
}

int sqlite3_actionlogtojson_init(sqlite3 *db, char **pzErrMsg, const sqlite3_api_routines *pApi)
{
    omp_set_num_threads(omp_get_num_procs());
    json_set_alloc_funcs(malloc, free);

    int rc = SQLITE_OK;

    SQLITE_EXTENSION_INIT2(pApi);

    rc = sqlite3_create_function(
            db,
            "ext_test",
            0,
            SQLITE_UTF8 | SQLITE_INNOCUOUS,
            db,
            ext_test,
            NULL,
            NULL);

    return rc;
}
