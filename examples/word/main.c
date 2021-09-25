#include <dc_fsm/fsm.h>
#include <dc_posix/dc_time.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


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
static int process(const struct dc_posix_env *env, struct dc_error *err, void *arg);
static int upper(const struct dc_posix_env *env, struct dc_error *err, void *arg);
static int lower(const struct dc_posix_env *env, struct dc_error *err, void *arg);
static int nothing(const struct dc_posix_env *env, struct dc_error *err, void *arg);
static int do_nothing(int c);
static int convert(const struct dc_posix_env *env, struct dc_error *err, const char *str, int (*converter)(int));


enum application_states
{
    	PROCESS = DC_FSM_USER_START,    // 2
	UPPER,
	LOWER,
	NOTHING,
};


int main(int argc, char *argv[])
{
    struct dc_error err;
    struct dc_posix_env env;
    int ret_val;
    struct dc_fsm_info *fsm_info;
    static struct dc_fsm_transition transitions[] = {
            {DC_FSM_INIT, PROCESS,     process},
            {PROCESS,     UPPER, upper},
            {PROCESS,     LOWER, lower},
            {PROCESS,     NOTHING, nothing},
            {UPPER,       DC_FSM_EXIT, NULL},
            {LOWER,       DC_FSM_EXIT, NULL},
            {NOTHING,     DC_FSM_EXIT, NULL},
    };

    dc_error_init(&err, error_reporter);
    dc_posix_env_init(&env, NULL /*trace_reporter*/);
    ret_val = EXIT_SUCCESS;
    fsm_info = dc_fsm_info_create(&env, &err, "word");
//    dc_fsm_info_set_will_change_state(fsm_info, will_change_state);
//    dc_fsm_info_set_did_change_state(fsm_info, did_change_state);
    dc_fsm_info_set_bad_change_state(fsm_info, bad_change_state);

    if(dc_error_has_no_error(&err))
    {
        int from_state;
        int to_state;

        ret_val = dc_fsm_run(&env, &err, fsm_info, &from_state, &to_state, argv[1], transitions);
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

static int process(const struct dc_posix_env *env, struct dc_error *err, void *arg)
{
	const char *str;
	char c;
	int next_state;

	str = (const char *)arg;
	c = str[0];

	if(isupper(c))
	{
		next_state = UPPER;
	}	
	else if(islower(c))
	{
		next_state = LOWER;
	}	
	else
	{
		next_state = NOTHING;
	}	

	return next_state;
}

static int upper(const struct dc_posix_env *env, struct dc_error *err, void *arg)
{
	const char *str;
	
	str = (const char *)arg;
	convert(env, err, str, toupper);

	return DC_FSM_EXIT;
}

static int lower(const struct dc_posix_env *env, struct dc_error *err, void *arg)
{
	const char *str;
	
	str = (const char *)arg;
	convert(env, err, str, tolower);

	return DC_FSM_EXIT;
}

static int nothing(const struct dc_posix_env *env, struct dc_error *err, void *arg)
{
	const char *str;
	
	str = (const char *)arg;
	convert(env, err, str, do_nothing);

	return DC_FSM_EXIT;
}

static int do_nothing(int c)
{
	return c;
}


static int convert(const struct dc_posix_env *env, struct dc_error *err, const char *str, int (*converter)(int))
{
	size_t len;
	
	len = strlen(str);

	for(size_t i = 0; i < len; i++)
	{
		printf("%c", converter(str[i]));
	}

	printf("\n");
	return DC_FSM_EXIT;
}
