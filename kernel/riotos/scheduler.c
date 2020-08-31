/**
\brief RIOT based task scheduler.

\author Francisco Molina <francois-xavier.molina@inria.fr>, August 2020.
*/

#include "opendefs.h"
#include "scheduler.h"
#include "debugpins.h"
#include "leds.h"

#include "event/callback.h"

#include "openwsn_log.h"

//=========================== variables =======================================

scheduler_vars_t scheduler_vars;
scheduler_dbg_t scheduler_dbg;

event_queue_t _queues[TASKPRIO_MAX] = { EVENT_QUEUE_INIT_DETACHED };

//=========================== public ==========================================

event_callback_t* _scheduler_get_free_event(void)
{
    for(uint8_t i = 0; i < TASK_LIST_DEPTH; i++) {
        if(scheduler_vars.task_buff[i].super.handler == NULL) {
            return &scheduler_vars.task_buff[i];
        }
    }

    /* task list has overflown. This should never happen! */
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

    for (uint8_t i = 0; i < TASKPRIO_MAX; i++) {
        event_queue_init_detached(&_queues[i]);
    }
}

void scheduler_start(unsigned state) {
    irq_restore(state);
    event_t *event;

    for (uint8_t i = 0; i < TASKPRIO_MAX; i++) {
        event_queue_claim(&_queues[i]);
    }

    /* wait for events */
    while ((event = event_wait_multi(&_queues, TASKPRIO_MAX))) {
        debugpins_task_clr();
        event->handler(event);
        /* remove from task list */
        event->handler = NULL;
        scheduler_dbg.numTasksCur--;
        debugpins_task_set();
    }
}

void scheduler_push_task(task_cbt cb, task_prio_t prio)
{
    INTERRUPT_DECLARATION();
    DISABLE_INTERRUPTS();
    /* get a free event from the queue */
    event_callback_t* event = _scheduler_get_free_event();
    event_callback_init(event, cb, NULL);
    /* post and sort the list */
    event_post(&_queues[prio], (event_t*) event);

    scheduler_dbg.numTasksCur++;
    if (scheduler_dbg.numTasksCur > scheduler_dbg.numTasksMax) {
        scheduler_dbg.numTasksMax = scheduler_dbg.numTasksCur;
    }
    ENABLE_INTERRUPTS();
}

//=========================== private =========================================
