#include <dc_fsm/fsm.h>
#include <dc_posix/dc_time.h>
#include <stdio.h>
#include <stdlib.h>


static void error_reporter(const struct dc_error *err);
static void trace_reporter(const struct dc_posix_env *env, const char *file_name,
                           const char *function_name, size_t line_number);
static void will_change_state(const struct dc_posix_env *env,
                struct dc_error *err,
                const struct dc_fsm_info *info,
                int from_state_id,
                int to_state_id);
static void did_change_state(const struct dc_posix_env *env,
                         struct dc_error *err,
                         const struct dc_fsm_info *info,
                         int from_state_id,
                         int to_state_id,
                         int next_id);
static void bad_change_state(const struct dc_posix_env *env,
                         struct dc_error *err,
                         const struct dc_fsm_info *info,
                         int from_state_id,
                         int to_state_id);
static int red(const struct dc_posix_env *env, struct dc_error *err, void *arg);
static int green(const struct dc_posix_env *env, struct dc_error *err, void *arg);
static int yellow(const struct dc_posix_env *env, struct dc_error *err, void *arg);
static int change_colour(const struct dc_posix_env *env, struct dc_error *err, const char *name, const struct timespec *time, int next_state);
static int state_error(const struct dc_posix_env *env, struct dc_error *err, void *arg);


enum application_states
{
    RED = DC_FSM_USER_START,    // 2
    GREEN,
    YELLOW,
    ERROR,
};


struct times
{
    struct timespec red_time;
    struct timespec green_time;
    struct timespec yellow_time;
};


int main(void)
{
    struct dc_error err;
    struct dc_posix_env env;
    int ret_val;
    struct dc_fsm_info *fsm_info;
    static struct dc_fsm_transition transitions[] = {
            {DC_FSM_INIT, RED,    red},
            {RED,         GREEN,  green},
            {GREEN,       YELLOW, yellow},
            {YELLOW,      RED,    red},
            {RED,         ERROR,  state_error},
            {GREEN,       ERROR,  state_error},
            {YELLOW,      ERROR,  state_error},
            {ERROR,       DC_FSM_EXIT, NULL}
    };

    dc_error_init(&err, error_reporter);
    dc_posix_env_init(&env, NULL /*trace_reporter*/);
    ret_val = EXIT_SUCCESS;
    fsm_info = dc_fsm_info_create(&env, &err, "traffic");
//    dc_fsm_info_set_will_change_state(fsm_info, will_change_state);
//    dc_fsm_info_set_did_change_state(fsm_info, did_change_state);
//    dc_fsm_info_set_bad_change_state(fsm_info, bad_change_state);

    if(dc_error_has_no_error(&err))
    {
        struct times timing;
        int from_state;
        int to_state;

        timing.red_time.tv_sec = 2;
        timing.red_time.tv_nsec = 500000000L;
        timing.green_time.tv_sec = 2;
        timing.green_time.tv_nsec = 500000000L;
        timing.yellow_time.tv_sec = 0;
        timing.yellow_time.tv_nsec = 500000000L;
        ret_val = dc_fsm_run(&env, &err, fsm_info, &from_state, &to_state, &timing, transitions);
        dc_fsm_info_destroy(&env, &fsm_info);
    }

    return ret_val;
}

static void error_reporter(const struct dc_error *err)
{
    fprintf(stderr, "Error: \"%s\" - %s : %s @ %zu\n", err->message, err->file_name, err->function_name, err->line_number);
}

static void trace_reporter(const struct dc_posix_env *env, const char *file_name,
                           const char *function_name, size_t line_number)
{
    fprintf(stderr, "Entering: %s : %s @ %zu\n", file_name, function_name, line_number);
}

static void will_change_state(const struct dc_posix_env *env,
                              struct dc_error *err,
                              const struct dc_fsm_info *info,
                              int from_state_id,
                              int to_state_id)
{
    printf("%s: will change %d -> %d\n", dc_fsm_info_get_name(info), from_state_id, to_state_id);
}

static void did_change_state(const struct dc_posix_env *env,
                             struct dc_error *err,
                             const struct dc_fsm_info *info,
                             int from_state_id,
                             int to_state_id,
                             int next_id)
{
    printf("%s: did change %d -> %d moving to %d\n", dc_fsm_info_get_name(info), from_state_id, to_state_id, next_id);
}

static void bad_change_state(const struct dc_posix_env *env,
                             struct dc_error *err,
                             const struct dc_fsm_info *info,
                             int from_state_id,
                             int to_state_id)
{
    printf("%s: bad change %d -> %d\n", dc_fsm_info_get_name(info), from_state_id, to_state_id);
}

static int red(const struct dc_posix_env *env, struct dc_error *err, void *arg)
{
    int next_state;
    struct times *timing;

    timing = (struct times *)arg;
    next_state = change_colour(env, err, "RED", &timing->red_time, GREEN);

    return next_state;
}

static int green(const struct dc_posix_env *env, struct dc_error *err, void *arg)
{
    int next_state;
    struct times *timing;

    timing = (struct times *)arg;
    next_state = change_colour(env, err, "GREEN", &timing->green_time, YELLOW);

    return next_state;
}

static int yellow(const struct dc_posix_env *env, struct dc_error *err, void *arg)
{
    int next_state;
    struct times *timing;

    timing = (struct times *)arg;
    next_state = change_colour(env, err, "YELLOW", &timing->yellow_time, RED);

    return next_state;
}

static int change_colour(const struct dc_posix_env *env, struct dc_error *err, const char *name, const struct timespec *time, int next_state)
{
    int ret_val;

    printf("%s\n", name);
    dc_nanosleep(env, err, time, NULL);

    if(dc_error_has_no_error(err))
    {
        ret_val = next_state;
    }
    else
    {
        ret_val = ERROR;
    }

    return ret_val;
}

static int state_error(const struct dc_posix_env *env, struct dc_error *err, void *arg)
{
    printf("ERROR\n");

    return DC_FSM_EXIT;
}

