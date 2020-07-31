/**
\brief Task scheduler.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, October 2014.
*/

#include "opendefs.h"
#include "scheduler.h"
#include "debugpins.h"
#include "leds.h"

#include "event_task.h"

#include "openwsn_log.h"

//=========================== variables =======================================

event_queue_t _queue;
scheduler_vars_t scheduler_vars;
scheduler_dbg_t scheduler_dbg;

//=========================== public ==========================================

event_task_t* _scheduler_get_free_event(void)
{
    for(uint8_t i = 0; i < TASK_LIST_DEPTH; i++) {
        if(scheduler_vars.task_buff[i].super.handler == NULL) {
            return &scheduler_vars.task_buff[i];
        }
    }

    /* task list has overflown. This should never happpen! */
    LOG_RIOT_ERROR("[openos/scheduler]: critical, task list overflow\n");
    leds_error_blink();
    board_reset();
}

void scheduler_init(void) {
    memset(&scheduler_vars,0,sizeof(scheduler_vars_t));
    memset(&scheduler_dbg,0,sizeof(scheduler_dbg_t));

    for(uint8_t i = 0; i < TASK_LIST_DEPTH; i++) {
        scheduler_vars.task_buff[i].super.handler = NULL;
    }

    event_queue_init(&_queue);
}

void scheduler_start(unsigned state) {
    irq_restore(state);
    event_t *event;

    /* wait for events */
    while ((event = event_wait(&_queue))) {
        debugpins_task_clr();
        event->handler(event);
        /* remove from task list */
        event->handler = NULL;
        debugpins_task_set();
    }
}

void scheduler_push_task(task_cbt cb, task_prio_t prio)
{
    INTERRUPT_DECLARATION();
    DISABLE_INTERRUPTS();
    /* get a free event from the queue */
    event_task_t* event = _scheduler_get_free_event();
    event_task_init(event, cb, prio);
    /* posted and sort the list */
    event_task_post_sorted(&_queue, event);
    ENABLE_INTERRUPTS();
}

//=========================== private =========================================
