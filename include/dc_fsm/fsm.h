#ifndef LIBDC_FSM_FSM_H
#define LIBDC_FSM_FSM_H


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


#include <stdio.h>


struct dc_fsm_info;


typedef enum
{
    DC_FSM_IGNORE = -1, // -1
    DC_FSM_INIT,        // 0
    DC_FSM_EXIT,        // 1
    DC_FSM_USER_START,  // 2
} dc_fsm_state;


typedef int (*dc_fsm_state_func)(void *arg);


struct dc_fsm_transition
{
    int from_id;
    int to_id;
    dc_fsm_state_func perform;
};


struct dc_fsm_info *dc_fsm_info_create(const char *name, FILE *verbose_file);
void dc_fsm_info_destroy(struct dc_fsm_info **pinfo);
int dc_fsm_run(struct dc_fsm_info *info, int *from_state_id, int *to_state_id, void *arg, const struct dc_fsm_transition transitions[]);


#endif // LIBDC_FSM_FSM_H
