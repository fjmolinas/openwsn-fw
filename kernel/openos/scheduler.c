/**
\brief OpenOS scheduler.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "opendefs.h"
#include "scheduler.h"
#include "openwsn_board.h"
#include "debugpins.h"
#include "leds.h"

#include "openwsn.h"
#include "thread.h"

#include "openwsn_log.h"

#define OPENWSN_SCHEDULER_FLAG    (1u << 8)

//=========================== variables =======================================

scheduler_vars_t scheduler_vars;

#if SCHEDULER_DEBUG_ENABLE
scheduler_dbg_t scheduler_dbg;
#endif

//=========================== prototypes ======================================

void consumeTask(uint8_t taskId);

//=========================== public ==========================================

void scheduler_init(void) {

    // initialization module variables
    memset(&scheduler_vars,0,sizeof(scheduler_vars_t));
#if SCHEDULER_DEBUG_ENABLE
    memset(&scheduler_dbg,0,sizeof(scheduler_dbg_t));
#endif

    // enable the scheduler's interrupt so SW can wake up the scheduler
    SCHEDULER_ENABLE_INTERRUPT();
}

void scheduler_start(unsigned state) {
    taskList_item_t* pThisTask;

   irq_restore(state);

    while (1) {
        while(scheduler_vars.task_list!=NULL) {
         // there is still at least one task in the linked-list of tasks

         INTERRUPT_DECLARATION();
         DISABLE_INTERRUPTS();

         // the task to execute is the one at the head of the queue
         pThisTask                = scheduler_vars.task_list;

         // shift the queue by one task
         scheduler_vars.task_list = pThisTask->next;

         ENABLE_INTERRUPTS();

         // execute the current task
         pThisTask->cb();

         // free up this task container
         pThisTask->cb            = NULL;
         pThisTask->prio          = TASKPRIO_NONE;
         pThisTask->next          = NULL;
#if SCHEDULER_DEBUG_ENABLE
         scheduler_dbg.numTasksCur--;
#endif
      }
      debugpins_task_clr();
      board_sleep();
      thread_flags_wait_any(OPENWSN_SCHEDULER_FLAG);
      debugpins_task_set();                      // IAR should halt here if nothing to do
   }
}

void scheduler_push_task(task_cbt cb, task_prio_t prio) {
    taskList_item_t*  taskContainer;
    taskList_item_t** taskListWalker;
    INTERRUPT_DECLARATION();

    DISABLE_INTERRUPTS();

    // find an empty task container
    taskContainer = &scheduler_vars.taskBuf[0];
    while (taskContainer->cb!=NULL &&
          taskContainer<=&scheduler_vars.taskBuf[TASK_LIST_DEPTH-1]) {
       taskContainer++;
    }
    if (taskContainer>&scheduler_vars.taskBuf[TASK_LIST_DEPTH-1]) {
       // task list has overflown. This should never happpen!
       LOG_RIOT_ERROR("[openos/scheduler]: critical, task list overflow\n");
       // we can not print from within the kernel. Instead:
       // blink the error LED
       leds_error_blink();
       // reset the board
       board_reset();
    }
    // fill that task container with this task
    taskContainer->cb              = cb;
    taskContainer->prio            = prio;

    // find position in queue
    taskListWalker                 = &scheduler_vars.task_list;
    while (*taskListWalker!=NULL &&
          (*taskListWalker)->prio <= taskContainer->prio) {
       taskListWalker              = (taskList_item_t**)&((*taskListWalker)->next);
    }
    // insert at that position
    taskContainer->next            = *taskListWalker;
    *taskListWalker                = taskContainer;
    // maintain debug stats
#if SCHEDULER_DEBUG_ENABLE
    scheduler_dbg.numTasksCur++;
    if (scheduler_dbg.numTasksCur>scheduler_dbg.numTasksMax) {
        scheduler_dbg.numTasksMax   = scheduler_dbg.numTasksCur;
    }
#endif

    ENABLE_INTERRUPTS();
   thread_t *thread = (thread_t*) thread_get(openwsn_get_pid());
   thread_flags_set(thread, OPENWSN_SCHEDULER_FLAG);
}


#if SCHEDULER_DEBUG_ENABLE
uint8_t scheduler_debug_get_TasksCur(void)
{
   return scheduler_dbg.numTasksCur;
}

uint8_t scheduler_debug_get_TasksMax(void)
{
   return scheduler_dbg.numTasksMax;
}
#endif
//=========================== private =========================================
