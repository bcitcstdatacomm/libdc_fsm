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

#include <dc_posix/dc_posix_env.h>
#include <stdio.h>

struct dc_fsm_info;

typedef enum {
  DC_FSM_IGNORE = -1, // -1
  DC_FSM_INIT,        // 0
  DC_FSM_EXIT,        // 1
  DC_FSM_USER_START,  // 2
} dc_fsm_state;

typedef int (*dc_fsm_state_func)(const struct dc_posix_env *env,
                                 struct dc_error *err, void *arg);

struct dc_fsm_transition {
  int from_id;
  int to_id;
  dc_fsm_state_func perform;
};

/**
 *
 * @param env
 * @param name
 * @param verbose_file
 * @return
 */
struct dc_fsm_info *dc_fsm_info_create(const struct dc_posix_env *env,
                                       struct dc_error *err, const char *name);

/**
 *
 * @param env
 * @param pinfo
 */
void dc_fsm_info_destroy(const struct dc_posix_env *env,
                         struct dc_fsm_info **pinfo);

/**
 *
 * @param env
 * @param info
 * @param from_state_id
 * @param to_state_id
 * @param arg
 * @param transitions
 * @return
 */
int dc_fsm_run(const struct dc_posix_env *env, struct dc_error *err,
               struct dc_fsm_info *info, int *from_state_id, int *to_state_id,
               void *arg, const struct dc_fsm_transition transitions[]);

#endif // LIBDC_FSM_FSM_H
