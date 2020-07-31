/*
 * Copyright (C) 2017 Inria
 *               2017 Freie Universität Berlin
 *               2017 Kaspar Schleiser <kaspar@schleiser.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include "event_task.h"

int _event_task_cmp(clist_node_t *a, clist_node_t *b)
{
    event_t *event_a = container_of(a, event_t, list_node);
    event_t *event_b = container_of(b, event_t, list_node);
    event_task_t *event_task_a = container_of(event_a, event_task_t, super);
    event_task_t *event_task_b = container_of(event_b, event_task_t, super);
    return event_task_a->prio - event_task_b->prio;
}

void event_task_init(event_task_t *event_task, event_handler_t handler, int prio)
{
    event_task->super.handler = handler;
    event_task->prio = prio;
}

void event_task_post_sorted(event_queue_t *queue, event_task_t *event)
{
    event_post(queue, (event_t *) event);
    clist_sort(&queue->event_list, _event_task_cmp);
}
