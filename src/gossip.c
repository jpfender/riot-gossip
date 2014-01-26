#include "gossip.h"
#include <thread.h>
#include "list.h"
#include "random.h"
#include <string.h>
#include "protocol.h"
#include "hwtimer.h"
#include "vtimer.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

#define SND_BUFFER_SIZE     (100)
#define RCV_BUFFER_SIZE     (64)
#define RADIO_STACK_SIZE    (KERNEL_CONF_STACKSIZE_DEFAULT)

#define WAIT_TIME           (2)
#define SECOND              (1000 * 1000)
#define SENDING_DELAY       (10 * 1000)

#define CLEANUP_THRESHOLD   (10)

char gossip_radio_stack_buffer[RADIO_STACK_SIZE];
msg_t msg_q[RCV_BUFFER_SIZE];

int gossip_handle_msg(radio_packet_t* p);
int gossip_handle_announce(radio_packet_t* p);

static void (*gossip_application_msg_handler) (void*,size_t, uint16_t) = 0;
static int (*gossip_on_remove_neighbour_handler) (gossip_node_t*) = 0;

list_t *neighbours = 0;

int transceiver_thread = 0;

// this function contains the message revciever loop
void gossip_radio(void) {
    msg_t m;
    radio_packet_t *p;

    msg_init_queue(msg_q, RCV_BUFFER_SIZE);

    while (1) {
        msg_receive(&m);
        DEBUG("D: gossip_radio received something\n");
        if (m.type == PKT_PENDING) {
            p = (radio_packet_t*) m.content.ptr;

            if(gossip_handle_msg(p)) {
                DEBUG("E: Handle gossip msg failed\n");
            }

            p->processing--;
        }
        else if (m.type == ENOBUFFER) {
            DEBUG("W: Transceiver buffer full\n");
        }
        else {
            DEBUG("W: Unknown packet received\n");
        }
    }
}

int gossip_init(uint16_t id, transceiver_type_t transceiver_type) {
    int gossip_radio_pid;
    transceiver_command_t tcmd;
    msg_t mesg;

    if(id == 0) {
        return 1;
    }

    neighbours = list_new();

    transceiver_init(transceiver_type);
    transceiver_start();
    gossip_radio_pid = thread_create(gossip_radio_stack_buffer,
                                     RADIO_STACK_SIZE,
                                     PRIORITY_MAIN-2,
                                     CREATE_STACKTEST,
                                     gossip_radio,
                                     "gossip_radio");
    transceiver_register(transceiver_type, gossip_radio_pid);

    tcmd.transceivers = transceiver_type;
    tcmd.data = &id;
    mesg.content.ptr = (char *) &tcmd;
    mesg.type = SET_ADDRESS;

    DEBUG("D: trying to set address %i\n", id);
    msg_send_receive(&mesg, &mesg, transceiver_pid);

    hwtimer_wait(HWTIMER_TICKS(WAIT_TIME * SECOND));

    gossip_id = id;

    return 0;
}

int gossip_announce(void) {
    char msg_buffer[strlen(PREAMBLE) + strlen(ANNOUNCE) + 1];
    sprintf(msg_buffer, "%s%s", PREAMBLE, ANNOUNCE);
    return gossip_send(NULL, msg_buffer, strlen(msg_buffer));
}

gossip_node_list_t* gossip_get_all_neighbours(void) {
    size_t len = neighbours->len;
    gossip_node_list_t *node_list = malloc(sizeof(gossip_node_list_t));
    gossip_node_t **nodes = malloc(sizeof(gossip_node_t*)* len);

    item_t *cur = list_get_head(neighbours);
    size_t i = 0;
    for(i=0;i<len;i++) {
        nodes[i] = (gossip_node_t*)cur->val;
        cur = list_get_next(cur);
    }

    node_list->nodes=nodes;
    node_list->length = len;
    return node_list;
}

void gossip_free_node_list(gossip_node_list_t* nl) {
    free(nl->nodes);
    free(nl);
}

gossip_node_t *gossip_find_node_by_id(uint16_t id) {
    item_t *cur = list_get_head(neighbours);
    gossip_node_t *node = NULL;
        while(cur) {
            node = (gossip_node_t*) cur->val;
            if(node->id == id) {
                return node;
            }
            cur = list_get_next(cur);
        }
    return NULL;
}

gossip_node_t* gossip_get_neighbour_random() {
    size_t len = list_get_length(neighbours);
    item_t *cur = list_get_head(neighbours);
    uint32_t i = 0;

    if(len==0){
        return NULL;
    }

    uint32_t rand = genrand_uint32();
    rand = rand % len;

    for(i=0;i<rand;i++) {
        cur = list_get_next(cur);
    }
    return (gossip_node_t*) cur->val;
}

gossip_node_t* gossip_get_neighbour_oldest_first() {
    gossip_node_t *node = 0;
    gossip_node_t *lastest = 0;
    item_t *cur = list_get_head(neighbours);
    node = (gossip_node_t*) list_get_value(cur);
    lastest = node;
    cur = list_get_next(cur);
    while(cur) {
        node = (gossip_node_t*) list_get_value(cur);
        if (node->last_send < lastest->last_send) {
            lastest = node;
        }
        cur = list_get_next(cur);
    }

    return lastest;
}

gossip_node_t* gossip_get_neighbour(gossip_strategy_t strategy) {
    gossip_node_t *result;
    switch(strategy) {
        case RANDOM:
            result = gossip_get_neighbour_random();
            break;
        case OLDEST_FIRST:
            result = gossip_get_neighbour_oldest_first();
            break;
        default:
            return 0;
    }
    return result;
}

int gossip_send(gossip_node_t* node, void *gossip_message, int len) {

    msg_t mesg;
    transceiver_command_t tcmd;
    radio_packet_t p;

    mesg.type = SND_PKT;
    mesg.content.ptr = (char*) &tcmd;

    tcmd.transceivers = TRANSCEIVER_NATIVE;
    tcmd.data = &p;

    p.length = len;
    if (!node) {
        p.dst = 0;
    } else {
        p.dst = node->id;
    }
    p.data = gossip_message;

    int r = msg_send(&mesg, transceiver_pid, 1);
    hwtimer_wait(HWTIMER_TICKS(SENDING_DELAY));
    return r;
}

void gossip_update_neighbour(radio_packet_t* p) {
    gossip_node_t *node = gossip_find_node_by_id(p->src);
    if(!node) {
        DEBUG("D: adding new node: %i\n", p->src);
        node = malloc(sizeof(gossip_node_t));
        node->id = p->src;
        list_add_item(neighbours, node);
    }
    timex_t now;
    vtimer_now(&now);
    node->last_recv = now.microseconds/SECOND;
}

int gossip_handle_msg(radio_packet_t* p) {
    char *msg_text = (char*) p->data;
    size_t cur_len = p->length;
    msg_text[cur_len] = '\0';

    // check if it is a gossip packet
    if (strncmp(msg_text, PREAMBLE, strlen(PREAMBLE)))
        return -1;

    // strip premable and update msg length
    msg_text += strlen(PREAMBLE);
    cur_len -= strlen(PREAMBLE);

    // update neighbour table
    gossip_update_neighbour(p);

    // if packet is an ANNOUNCE Packet
    if (strncmp(msg_text, ANNOUNCE, strlen(ANNOUNCE)) == 0)
        return gossip_handle_announce(p);

    // if packet is an MSG Packet
    if (strncmp(msg_text, MSG, strlen(MSG)) == 0) {
        // strip msg header and update length
        msg_text += strlen(MSG);
        cur_len -= strlen(MSG);
        if(gossip_application_msg_handler) {
            gossip_application_msg_handler(msg_text,cur_len, p->src);
        }
        else {
            DEBUG("W: got msg from %i but handler was not set\n", p->src);
        }
        return 0;
    }

    return 0;
}

int gossip_handle_announce(radio_packet_t* p) {
    char msg_buffer[strlen(PREAMBLE) + strlen(ANNOUNCE) + 1];

    gossip_node_t* node = gossip_find_node_by_id(p->src);

    if( ! p->dst ){
        DEBUG("D: respnding to an announce from: %i\n", p->src);
        sprintf(msg_buffer, "%s%s", PREAMBLE, ANNOUNCE);
        return gossip_send(node, msg_buffer, strlen(msg_buffer));
    }
    return 0;
}

void gossip_cleanup(void) {
    gossip_node_t *node = 0;
    item_t *cur = list_get_head(neighbours);
    timex_t now;
    vtimer_now(&now);
    while (cur) {
        node = (gossip_node_t*) list_get_value(cur);
        if (now.microseconds/SECOND - node->last_recv > CLEANUP_THRESHOLD) {
            if (gossip_on_remove_neighbour_handler) {
                if (gossip_on_remove_neighbour_handler(node)) {
                    DEBUG("D: forgetting about node %d by handlers choice\n", node->id);
                    list_remove_item(neighbours, cur);
                } else {
                    DEBUG("D: keeping node %d by handlers choice\n", node->id);
                }
            } else {
                DEBUG("D: forgetting about node %d\n", node->id);
                list_remove_item(neighbours, cur);
            }
        } else {
            DEBUG("will forget %i in %i seconds\n", node->id,
                (CLEANUP_THRESHOLD - now.microseconds/SECOND + node->last_recv));
        }
        cur = list_get_next(cur);
    }
}

void gossip_register_msg_handler(void (*handle) (void*,size_t,uint16_t)) {
    gossip_application_msg_handler = handle;
}

void gossip_register_on_remove_neighbour_handler(int (*handle) (gossip_node_t*)) {
    gossip_on_remove_neighbour_handler = handle;
}
