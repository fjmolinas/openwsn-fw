#ifndef EVENT_TASK_H
#define EVENT_TASK_H

#include "event.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Callback Event structure definition
 */
typedef struct {
    event_t super;              /**< event_t structure that gets extended   */
    int prio;                   /**< event priority                         */
} event_task_t;

/**
 * @brief   event callback initialization function
 *
 * @param[out]  event_callback  object to initialize
 * @param[in]   callback        callback to set up
 * @param[in]   arg             callback argument to set up
 */
void event_task_init(event_task_t *event_task, event_handler_t handler, int prio);

void event_task_post_sorted(event_queue_t *queue, event_task_t *event);

/**
 * @brief   Callback Event static initializer
 *
 * @param[in]   _cb     callback function to set
 * @param[in]   _arg    arguments to set
 */
#define EVENT_TASK_INIT(_handler, _prio) \
    { \
        .super.handler = _handler, \
        .prio = _prio, \
    }

#ifdef __cplusplus
}
#endif
#endif /* EVENT_TASK_H */
/** @} */
