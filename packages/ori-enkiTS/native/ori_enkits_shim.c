/* enkiTS C API shim for Ori — parallel_for, N independent tasks, wait, shutdown. */
#include "../vendor/TaskScheduler_c.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static enkiTaskScheduler *g_ets = NULL;
static enkiTaskSet *g_range_task = NULL;
static enkiTaskSet *g_unit_task = NULL;

/* Range parallel_for accumulators (ranges do not overlap). */
static int64_t g_sum = 0;
static int64_t g_runs = 0;

/* N independent unit tasks: each index contributes once. */
static int64_t g_task_sum = 0;
static int64_t g_tasks_completed = 0;

static void range_sum(uint32_t start, uint32_t end, uint32_t threadnum, void *args) {
    (void)threadnum;
    (void)args;
    int64_t local = 0;
    for (uint32_t i = start; i < end; ++i) {
        local += (int64_t)i;
    }
    __atomic_add_fetch(&g_sum, local, __ATOMIC_RELAXED);
    __atomic_add_fetch(&g_runs, 1, __ATOMIC_RELAXED);
}

static void unit_work(uint32_t start, uint32_t end, uint32_t threadnum, void *args) {
    (void)threadnum;
    (void)args;
    for (uint32_t i = start; i < end; ++i) {
        __atomic_add_fetch(&g_task_sum, (int64_t)i, __ATOMIC_RELAXED);
        __atomic_add_fetch(&g_tasks_completed, 1, __ATOMIC_RELAXED);
    }
}

static void destroy_tasks(void) {
    if (!g_ets) return;
    if (g_range_task) {
        enkiDeleteTaskSet(g_ets, g_range_task);
        g_range_task = NULL;
    }
    if (g_unit_task) {
        enkiDeleteTaskSet(g_ets, g_unit_task);
        g_unit_task = NULL;
    }
}

static void full_shutdown(void) {
    if (!g_ets) return;
    enkiWaitforAllAndShutdown(g_ets);
    destroy_tasks();
    enkiDeleteTaskScheduler(g_ets);
    g_ets = NULL;
}

int64_t ori_enki_init(int64_t num_threads) {
    full_shutdown();
    g_ets = enkiNewTaskScheduler();
    if (!g_ets) return 1;
    if (num_threads > 0) {
        enkiInitTaskSchedulerNumThreads(g_ets, (uint32_t)num_threads);
    } else {
        enkiInitTaskScheduler(g_ets);
    }
    g_range_task = enkiCreateTaskSet(g_ets, range_sum);
    g_unit_task = enkiCreateTaskSet(g_ets, unit_work);
    if (!g_range_task || !g_unit_task) {
        full_shutdown();
        return 1;
    }
    g_sum = 0;
    g_runs = 0;
    g_task_sum = 0;
    g_tasks_completed = 0;
    return 0;
}

void ori_enki_shutdown(void) {
    full_shutdown();
}

int64_t ori_enki_is_initialized(void) {
    return g_ets != NULL ? 1 : 0;
}

int64_t ori_enki_num_threads(void) {
    if (!g_ets) return 0;
    return (int64_t)enkiGetNumTaskThreads(g_ets);
}

/* Wait for all scheduled work (no continuous producers). */
void ori_enki_wait_all(void) {
    if (!g_ets) return;
    enkiWaitForAll(g_ets);
}

/* parallel_for [0,n) with grain min_range; returns sum(i). */
int64_t ori_enki_parallel_for_sum(int64_t n, int64_t min_range) {
    if (!g_ets || !g_range_task || n <= 0) return 0;
    if (min_range < 1) min_range = 1;
    g_sum = 0;
    g_runs = 0;
    enkiAddTaskSetMinRange(g_ets, g_range_task, NULL, (uint32_t)n, (uint32_t)min_range);
    enkiWaitForTaskSet(g_ets, g_range_task);
    return g_sum;
}

/* Convenience: grain 1. */
int64_t ori_enki_parallel_sum(int64_t n) {
    return ori_enki_parallel_for_sum(n, 1);
}

int64_t ori_enki_last_range_runs(void) {
    return g_runs;
}

int64_t ori_enki_last_sum(void) {
    return g_sum;
}

/*
 * N independent unit tasks: TaskSet partitions of size 1, each index runs once.
 * Returns sum of task indices 0..n-1 after wait.
 */
int64_t ori_enki_run_n_tasks(int64_t n) {
    if (!g_ets || !g_unit_task || n <= 0) return 0;
    g_task_sum = 0;
    g_tasks_completed = 0;
    enkiAddTaskSetMinRange(g_ets, g_unit_task, NULL, (uint32_t)n, 1);
    enkiWaitForTaskSet(g_ets, g_unit_task);
    return g_task_sum;
}

int64_t ori_enki_tasks_completed(void) {
    return g_tasks_completed;
}

/* Schedule range sum without waiting (pair with wait_scheduled_range). */
int64_t ori_enki_add_range_sum(int64_t n, int64_t min_range) {
    if (!g_ets || !g_range_task || n <= 0) return 1;
    if (min_range < 1) min_range = 1;
    g_sum = 0;
    g_runs = 0;
    enkiAddTaskSetMinRange(g_ets, g_range_task, NULL, (uint32_t)n, (uint32_t)min_range);
    return 0;
}

void ori_enki_wait_scheduled_range(void) {
    if (!g_ets || !g_range_task) return;
    enkiWaitForTaskSet(g_ets, g_range_task);
}

int64_t ori_enki_is_range_complete(void) {
    if (!g_ets || !g_range_task) return 1;
    return enkiIsTaskSetComplete(g_ets, g_range_task) ? 1 : 0;
}

int64_t ori_enki_serial_sum(int64_t n) {
    if (n <= 0) return 0;
    int64_t s = 0;
    for (int64_t i = 0; i < n; ++i) s += i;
    return s;
}
