#include "gossip.h"

int gossip_announce(void) {

    return 0;
}

int gossip_init(void) {
    
    return 0;
}

gossip_node_list_t* gossip_get_all_neighbours() {

    return 0;
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
