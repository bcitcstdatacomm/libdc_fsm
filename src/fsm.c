/*
 * Copyright 2021-2021 D'Arcy Smith.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "fsm.h"
#include <dc_posix/dc_stdlib.h>
#include <dc_posix/dc_string.h>

static dc_fsm_state_func
fsm_transition(const struct dc_posix_env *env, int from_id, int to_id, const struct dc_fsm_transition transitions[]);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpadded"
struct dc_fsm_info
{
    char * name;
    size_t name_length;
    int    from_state_id;
    int    current_state_id;
    void (*notify_changing)(const struct dc_posix_env *env, struct dc_error *err, int from_state_id, int to_state_id);
    void (*notify_changed)(const struct dc_posix_env *env,
                           struct dc_error *          err,
                           int                        from_state_id,
                           int                        to_state_id,
                           int                        next_id);
    void (*notify_bad_state_transition)(const struct dc_posix_env *env,
                                        struct dc_error *          err,
                                        int                        from_state_id,
                                        int                        to_state_id);
} __attribute__((aligned(64)));
#pragma GCC diagnostic pop

struct dc_fsm_info *   dc_fsm_info_create(const struct dc_posix_env *env, struct dc_error *err, const char *name)
{
    struct dc_fsm_info *info;

    DC_TRACE(env);
    info = dc_calloc(env, err, 1, sizeof(struct dc_fsm_info));

    if(dc_error_has_no_error(err))
    {
        info->from_state_id    = DC_FSM_INIT;
        info->current_state_id = DC_FSM_USER_START;
        info->name_length      = dc_strlen(env, name) + 1;
        info->name             = dc_malloc(env, err, info->name_length);

        if(dc_error_has_no_error(err))
        {
            dc_strcpy(env, info->name, name);
        }
        else
        {
            dc_free(env, info, sizeof(struct dc_fsm_info));
            info = NULL;
        }
    }

    return info;
}

void dc_fsm_info_destroy(const struct dc_posix_env *env, struct dc_fsm_info **pinfo)
{
    struct dc_fsm_info *info;

    DC_TRACE(env);
    info = *pinfo;
    dc_free(env, info->name, info->name_length);
    dc_free(env, info, sizeof(struct dc_fsm_info));

    if(env->null_free)
    {
        *pinfo = NULL;
    }
}

int dc_fsm_run(const struct dc_posix_env *    env,
               struct dc_error *              err,
               struct dc_fsm_info *           info,
               int *                          from_state_id,
               int *                          to_state_id,
               void *                         arg,
               const struct dc_fsm_transition transitions[])
{
    int from_id;
    int to_id;

    DC_TRACE(env);
    from_id = info->from_state_id;
    to_id   = info->current_state_id;

    do
    {
        dc_fsm_state_func perform;
        int               next_id;

        // notify moving to
        if(info->notify_changing)
        {
            info->notify_changing(env, err, from_id, to_id);
        }

        perform = fsm_transition(env, from_id, to_id, transitions);

        if(perform == NULL)
        {
            *from_state_id = from_id;
            *to_state_id   = to_id;

            // notify error
            if(info->notify_bad_state_transition)
            {
                info->notify_bad_state_transition(env, err, from_id, to_id);
            }

            // TODO: this is not clear to the programmer of the application that there was an error - how to fix?
            return -1;
        }

        info->from_state_id    = from_id;
        info->current_state_id = to_id;
        from_id                = to_id;
        next_id                = perform(env, err, arg);

        // notify moving from
        if(info->notify_changed)
        {
            info->notify_changed(env, err, from_id, to_id, next_id);
        }

        to_id = next_id;

        if(dc_error_has_error(err))
        {
            // we had an issue that we can't cope with
            break;
        }
    } while(to_id != DC_FSM_EXIT);

    // commenting this out will give us the last non-exit transition, probably more useful
    //    *from_state_id = from_id;
    //    *to_state_id   = to_id;

    return 0;
}

static dc_fsm_state_func
fsm_transition(const struct dc_posix_env *env, int from_id, int to_id, const struct dc_fsm_transition transitions[])
{
    const struct dc_fsm_transition *transition;

    DC_TRACE(env);
    transition = &transitions[0];

    while(transition->from_id != DC_FSM_IGNORE)
    {
        if(transition->from_id == from_id && transition->to_id == to_id)
        {
            return transition->perform;
        }

        transition = transitions++;
    }

    return NULL;
}
