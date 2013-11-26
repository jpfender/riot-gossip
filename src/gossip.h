#include <stdint.h>
#include <stdlib.h>
#include "transceiver.h"
#include "protocol.h"

typedef enum gossip_strategy {
    RANDOM,
    OLDEST_FIRST
} gossip_strategy_t;

typedef struct gossip_node {
    uint32_t id;         //id of the node
    uint32_t last_send;  //timestamp till last msg
    uint32_t last_recv;  //timestamp till last msg
} gossip_node_t;

typedef struct gossip_node_list {
    size_t length;
    gossip_node_t** nodes;
} gossip_node_list_t;


// send hello message to broadcast adress (sould be called on a regular basis)
// returns 0 on success
// return error code in case of failure
int gossip_announce(void);

// register the message handler and init gossip system
// returns 0 on success
// return error code in case of failure
int gossip_init(int16_t id, transceiver_type_t transceiver_type);

// get a list of all known neighbours
// returns node list on success
// returns NULL in case of failure
gossip_node_list_t* gossip_get_all_neighbours();

// choose specific node by some strategy
// returns node on success
// returns NULL in case of failure
gossip_node_t* gossip_get_neighbour(gossip_strategy_t strategy);

// send a message to some node
// returns 0 on success
// return error code in case of failure
int gossip_send(gossip_node_t* node, void *gossip_message, int len);

// registers callback for received application messages
// signature: handle(void *data, size_t length)
void gossip_register_msg_handler(void (*handle) (void*,size_t));
  
