#ifndef __SCHEDULER_TYPES_H
#define __SCHEDULER_TYPES_H

/**
\addtogroup kernel
\{
\addtogroup Scheduler
\{
*/

#include "opendefs.h"
#include "scheduler.h"
#include "event_task.h"

typedef struct {
   event_task_t task_buff[TASK_LIST_DEPTH];
   uint8_t numTasksCur;
   uint8_t numTasksMax;
} scheduler_vars_t;

typedef struct {
   uint8_t numTasksCur;
   uint8_t numTasksMax;
} scheduler_dbg_t;

/**
\}
\}
*/
#endif
