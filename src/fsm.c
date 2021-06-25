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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static dc_fsm_state_func fsm_transition(int from_id, int to_id, const struct dc_fsm_transition transitions[]);


struct dc_fsm_info
{
    char *name;
    FILE *verbose_file;
    int from_state_id;
    int current_state_id;
};

struct dc_fsm_info *dc_fsm_info_create(const char *name, FILE *verbose_file)
{
    struct dc_fsm_info *info;

    info = calloc(1, sizeof(struct dc_fsm_info));
    info->name = calloc(1, strlen(name) + 1);
    strcpy(info->name, name);
    info->verbose_file     = verbose_file;
    info->from_state_id    = DC_FSM_INIT;
    info->current_state_id = DC_FSM_USER_START;

    return info;
}


void dc_fsm_info_destroy(struct dc_fsm_info **pinfo)
{
    memset((*pinfo)->name, '\0', strlen((*pinfo)->name));
    free((*pinfo)->name);
    memset(*pinfo, 0, sizeof(struct dc_fsm_info));
    free(*pinfo);
    *pinfo = NULL;
}

int dc_fsm_run(struct dc_fsm_info *info,
               int *from_state_id,
               int *to_state_id,
               void *arg,
               const struct dc_fsm_transition transitions[])
{
    int from_id;
    int to_id;

    from_id = info->from_state_id;
    to_id   = info->current_state_id;

    do
    {
        dc_fsm_state_func perform;

        if(info->verbose_file)
        {
            fprintf(info->verbose_file, "FSM: %s - moving from state: %d to state: %d\n", info->name, from_id, to_id);
        }

        perform = fsm_transition(from_id, to_id, transitions);

        if(perform == NULL)
        {
            *from_state_id = from_id;
            *to_state_id   = to_id;

            fprintf(stderr, "FSM: %s - no perform method found moving from %d to %d\n", info->name, from_id, to_id);
            // TODO: this is not clear to the programmer of the application that there was an error - how to fix?
            return -1;
        }

        info->from_state_id    = from_id;
        info->current_state_id = to_id;
        from_id                = to_id;
        to_id                  = perform(arg);
    }
    while(to_id != DC_FSM_EXIT);

// commenting this out will give us the last non-exit transition, probably more useful
//    *from_state_id = from_id;
//    *to_state_id   = to_id;

    return 0;
}

static dc_fsm_state_func fsm_transition(int from_id, int to_id, const struct dc_fsm_transition transitions[])
{
    const struct dc_fsm_transition *transition;

    transition = &transitions[0];

    while(transition->from_id != DC_FSM_IGNORE)
    {
        if(transition->from_id == from_id &&
           transition->to_id == to_id)
        {
            return transition->perform;
        }

        transition = transitions++;
    }

    return NULL;
}
