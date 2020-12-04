#include "config.h"

#if OPENWSN_URTOS_C

#include "opendefs.h"
#include "urtos.h"
#include "sock.h"
#include "async.h"
#include "openserial.h"
#include "packetfunctions.h"
#include "scheduler.h"
#include "IEEE802154E.h"
#include "schedule.h"
#include "icmpv6rpl.h"
#include "idmanager.h"
#include "openrandom.h"
#include "scheduler.h"
#include "sctimer.h"

#include "msf.h"

//=========================== defines =========================================

#define CPU_CYCLES_S                    (32000000UL / 3)
#define UDP_PERIOD_MS                   (10 * 1000)
#define FAST_PERIOD_MS                  (100)
#define SLOW_PERIOD_MS                  (6 * 1000)
#define FAST_EXEC_TIME_MS               (CPU_CYCLES_S / 100)
#define SLOW_EXEC_TIME_MS               (CPU_CYCLES_S / 10)

#define UDP_TRAFFIC_RATE 2

//=========================== variables =======================================


typedef struct {
   uint32_t start;
   uint32_t max_delay;
   uint32_t cum_delay;
   uint32_t counter;
} task_stats_t;

static task_stats_t _slow_stats;
static task_stats_t _fast_stats;

static bool _busy;
static sock_udp_t _sock_udp;

static const uint8_t urtos_dst_addr[] = { 0xbb, 0xbb, 0x00, 0x00, 0x00,
                                          0x00, 0x00, 0x00, 0x00, 0x00,
                                          0x00, 0x00, 0x00, 0x00, 0x00,
                                          0x01 };

#define URTOS_ID_STR       "urtos"

//=========================== prototypes ======================================

void _sock_udp_handler(sock_udp_t *sock, sock_async_flags_t type, void *arg);

void _slow_sensor_callback(void);
void _fast_sensor_callback(void);
void _slow_sensor_handler(opentimers_id_t id);
void _fast_sensor_handler(opentimers_id_t id);
void _udp_cb(opentimers_id_t id);
int _send(task_stats_t* slow_stats, task_stats_t* fast_stats);
//=========================== public ==========================================

void urtos_init(void) {

    openserial_printf("[udp]: urtos init\n");
    memset(&_sock_udp, 0, sizeof(sock_udp_t));
    sock_udp_ep_t local;

    local.family = AF_INET6;
    local.port = WKP_UDP_ECHO;

    if (sock_udp_create(&_sock_udp, &local, NULL, 0) < 0) {
        openserial_printf("[udp]: ERROR, could not create socket\n");
        return;
    }

    sock_udp_set_cb(&_sock_udp, _sock_udp_handler, NULL);

    /* ready to start injecting */
    _busy = false;

    /* start periodic timer */
    uint8_t id = opentimers_create(TIMER_ISR, TASKPRIO_NONE);
    opentimers_scheduleAbsolute(id, SLOW_PERIOD_MS, sctimer_readCounter(),
            TIME_MS, _slow_sensor_handler);
    id = opentimers_create(TIMER_ISR, TASKPRIO_NONE);
    opentimers_scheduleAbsolute(id, FAST_PERIOD_MS, sctimer_readCounter(),
            TIME_MS, _fast_sensor_handler);
    /* start periodic timer */
    id = opentimers_create(TIMER_GENERAL_PURPOSE, TASKPRIO_UDP);
    opentimers_scheduleIn(id, UDP_PERIOD_MS, TIME_MS, TIMER_PERIODIC, _udp_cb);
}

//=========================== private =========================================


void _fast_sensor_callback(void) {
    uint32_t start_ticks = sctimer_readCounter();
    uint32_t delay = start_ticks - _fast_stats.start;
    if (delay > _fast_stats.max_delay) {
        _fast_stats.max_delay = delay;
    }
    _fast_stats.cum_delay += delay;
    _fast_stats.counter++;
    /* busy loop */
    for (uint32_t i = 0; i < FAST_EXEC_TIME_MS; i++) {
        /* Make sure for loop is not optimized out */
        __asm__ ("");
    }
}

void _slow_sensor_callback(void) {
    uint32_t start_ticks = sctimer_readCounter();
    uint32_t delay = start_ticks - _slow_stats.start;
    if (delay > _slow_stats.max_delay) {
        _slow_stats.max_delay = delay;
    }
    _slow_stats.cum_delay += delay;
    _slow_stats.counter++;
    /* busy loop */
    for (uint32_t i = 0; i < SLOW_EXEC_TIME_MS; i++) {
        /* Make sure for loop is not optimized out */
        __asm__ ("");
    }
}

void _udp_cb(opentimers_id_t id) {
    if (openrandom_get16b() < (0xffff / UDP_TRAFFIC_RATE)) {
        _send(&_slow_stats, &_fast_stats);
    }
    /* reset the cumulative delay */
    _slow_stats.cum_delay = 0;
    _fast_stats.cum_delay = 0;
    _slow_stats.max_delay = 0;
    _fast_stats.max_delay = 0;
    _slow_stats.counter = 0;
    _fast_stats.counter = 0;
}

void _slow_sensor_handler(opentimers_id_t id)
{
    _slow_stats.start = sctimer_readCounter();
    opentimers_scheduleAbsolute(id, SLOW_PERIOD_MS, _slow_stats.start,
            TIME_MS, _slow_sensor_handler);
    scheduler_push_task(_slow_sensor_callback, TASKPRIO_SNIFFER);
}

void _fast_sensor_handler(opentimers_id_t id)
{
    _fast_stats.start = sctimer_readCounter();
    opentimers_scheduleAbsolute(id, FAST_PERIOD_MS, _fast_stats.start,
            TIME_MS, _fast_sensor_handler);
    scheduler_push_task(_fast_sensor_callback, TASKPRIO_BUTTON);
}

void _sock_udp_handler(sock_udp_t *sock, sock_async_flags_t type, void *arg)
{
    char buf[50];

    if (type & SOCK_ASYNC_MSG_RECV) {
        sock_udp_ep_t remote;
        int16_t res;

        if ((res = sock_udp_recv(sock, buf, sizeof(buf), 0, &remote)) >= 0) {
            openserial_printf("Received %d bytes from remote endpoint:\n", res);
            openserial_printf(" - port: %d", remote.port);
            openserial_printf(" - addr: ");
            for(int i=0; i < 16; i ++)
                openserial_printf("%x ", remote.addr.ipv6[i]);
            openserial_printf("\n\n");
            openserial_printf("Msg received: %s\n\n", buf);
        }
    }

    if (type & SOCK_ASYNC_MSG_SENT) {
        if (*((uint8_t *)arg) == E_FAIL) {
            openserial_printf("[udp]: send FAILED\n");
        }
        else {
            openserial_printf("[udp]: send SUCCESS\n");
        }
        _busy = false;
    }
}

bool clear_to_send(void)
{
    open_addr_t neighbor;

    /* dont inject packets if node is not synchronized */
    if (ieee154e_isSynch() == FALSE) {
        openserial_printf("[udp]: ERROR, no sync\n");
        return false;
    }

    /* dont inject packets if node is root node */
    if (idmanager_getIsDAGroot()) {
        openserial_printf("[udp]: ERROR, is DagRoot \n");
        return false;
    }

    /* check if node has a preffered parent */
    if (!icmpv6rpl_getPreferredParentEui64(&neighbor)) {
        openserial_printf("[udp]: ERROR, no preferred parent\n");
        return false;
    }

    /* we must have a negotiated cell to that neighbor */
    if (!schedule_hasNegotiatedCellToNeighbor(&neighbor, CELLTYPE_TX)) {
        openserial_printf("[udp]: ERROR, no negotiated cell\n");
        return false;
    }

    /* abort is previous package has not been sent out yet */
   if (_busy) {
        openserial_printf("[udp]: ERROR, busy\n");
        return false;
    }

    return true;
}


int _send(task_stats_t* slow_stats, task_stats_t* fast_stats)
{
    if (!clear_to_send()) {
        return -1;
    }
    sock_udp_ep_t remote;
    remote.port = WKP_UDP_INJECT;
    remote.family = AF_INET6;
    memcpy(remote.addr.ipv6, urtos_dst_addr, sizeof(urtos_dst_addr));

    uint8_t buffer[50];
    size_t len = 0;
    uint8_t asn_array[5];

    /* add 'uinject' string */
    memcpy(&buffer[len], URTOS_ID_STR, strlen(URTOS_ID_STR));
    len += strlen(URTOS_ID_STR);
    /* add 16b addr */
    buffer[len++] = (uint8_t)(idmanager_getMyID(ADDR_16B)->addr_16b[1]);
    buffer[len++] = (uint8_t)(idmanager_getMyID(ADDR_16B)->addr_16b[0]);
    /* add asn */
    ieee154e_getAsn(asn_array);
    memcpy(&buffer[len], asn_array, sizeof(asn_array));
    len += sizeof(asn_array);
    /* add time data */
    memcpy(&buffer[len], &slow_stats->cum_delay, sizeof(slow_stats->cum_delay));
    len += sizeof(slow_stats->cum_delay);
    memcpy(&buffer[len], &slow_stats->max_delay, sizeof(slow_stats->max_delay));
    len += sizeof(slow_stats->max_delay);
    memcpy(&buffer[len], &slow_stats->counter, sizeof(slow_stats->counter));
    len += sizeof(slow_stats->counter);
    memcpy(&buffer[len], &fast_stats->max_delay, sizeof(fast_stats->max_delay));
    len += sizeof(fast_stats->max_delay);
    memcpy(&buffer[len], &fast_stats->cum_delay, sizeof(fast_stats->cum_delay));
    len += sizeof(fast_stats->cum_delay);
    memcpy(&buffer[len], &fast_stats->counter, sizeof(fast_stats->counter));
    len += sizeof(fast_stats->counter);

    if (sock_udp_send(&_sock_udp, buffer, len, &remote)) {
        _busy = true;
    }
    else {
        openserial_printf("[udp]: failed to send\n");
        return -1;
    }
    return 0;
}

#endif /* OPENWSN_URTOS_C */
