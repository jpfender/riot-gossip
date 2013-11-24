#include "gossip.h"
#include "list.h"


static void (*gossip_application_msg_handler) (void*,size_t) = 0;

list_t *neighbours = 0;

uint32_t gossip_id;

int gossip_init(uint32_t id, transceiver_type_t transceiver_type) {
    int gossip_radio_pid;
    
    if(id == 0) {
        return 1;
    }

    transceiver_init(transceiver_type);
    transceiver_start();
    gossip_radio_pid = thread_create(gossip_radio_stack_buffer,
                                     RADIO_STACK_SIZE,
                                     PRIORITY_MAIN-2,
                                     CREATE_STACKTEST,
                                     gossip_radio,
                                     "gossip_radio");
    transceiver_register(transceiver_type, gossip_radio_pid);

    neighbours = list_new();

    gossip_id = id;

    return 0;
}

int gossip_announce(void) {

    return 0;
}

gossip_node_list_t* gossip_get_all_neighbours(void) {
    size_t len = neighbours->len;
    gossip_node_list_t *node_list = malloc(sizeof(gossip_node_list_t));
    gossip_node_t **nodes = malloc(sizeof(gossip_node_t*)* len);

    item_t *cur = list_get_head(neighbours);
    int i = 0;
    for(i=0;i<len;i++) {
        nodes[i] = (gossip_node_t*)cur->val;
        cur = cur->next;
    }

    node_list->nodes=nodes;
    node_list->length = len;
    return node_list;
}

gossip_node_t* gossip_get_neighbour(gossip_strategy_t strategy) {

    return 0;
}

int gossip_send(gossip_node_t node, void *gossip_message) {

    return 0;
}

int gossip_handle_msg(void *msg, size_t length) {

    return 0;
}

void gossip_register_msg_handler(void (*handle) (void*,size_t)) {
    gossip_application_msg_handler = handle;
}
