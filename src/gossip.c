#include "gossip.h"
#include "list.h"

list_t *neighbours = 0;
int gossip_init(uint32_t id, transceiver_type_t transceiver_type) {

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

int gossip_register_msg_handler(void (*handle) (void*,size_t)) {

    return 0;
}
