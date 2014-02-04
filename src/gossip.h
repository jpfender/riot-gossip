#include <stdint.h>
#include <stdlib.h>
#include "transceiver.h"
#include "protocol.h"

#ifdef MODULE_MSBA2
#define TRANSCEIVER_TYPE TRANSCEIVER_CC1100
#endif
#ifdef MODULE_NATIVENET
#define TRANSCEIVER_TYPE TRANSCEIVER_NATIVE
#endif
#ifdef MODULE_CC110X_NG
#include "cc110x_ng.h"
#define TRANSCEIVER_TYPE TRANSCEIVER_CC1100
#endif

#define OK      1
#define KEEP    0

uint32_t gossip_id;

typedef enum gossip_strategy {
    RANDOM,
    OLDEST_FIRST
} gossip_strategy_t;

typedef struct gossip_node {
    uint16_t id;         //id of the node
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
int gossip_init(uint16_t id, transceiver_type_t transceiver_type);

// get a list of all known neighbours
// returns node list on success
// returns NULL in case of failure
gossip_node_list_t* gossip_get_all_neighbours();

// free a node list returned by gossip_get_all_neighbours()
// takes node list as paramter
// returns nothing
void gossip_free_node_list(gossip_node_list_t* nl);

// look through the list for a node with id
// returns pointer to that node on success
// returns NULL in case of failure
gossip_node_t* gossip_find_node_by_id(uint16_t id);

// choose specific node by some strategy
// returns node on success
// returns NULL in case of failure
gossip_node_t* gossip_get_neighbour(gossip_strategy_t strategy);

// choose a random node
// returns node on success
// return NULL in case of failure
gossip_node_t* gossip_get_neighbour_random(void);

// send a message to some node
// returns 0 on success
// return error code in case of failure
int gossip_send(gossip_node_t* node, void *gossip_message, int len);

// registers callback for received application messages
// signature: void handle(void *data, size_t length, uint16_t source)
void gossip_register_msg_handler(void (*handle) (void*,size_t,uint16_t));

// removes neighbours from global list after CLEANUP_THRESHOLD time
void gossip_cleanup(void);

// Hooks
// =====

// register a handler that is run upon removing a node from the
// neighbour list
// handler signature: int handle(gossip_node_t node_to_remove)
// if handle returns 0 the node is removed else its kept in the list
void gossip_register_on_remove_neighbour_handler(int (*handle) (gossip_node_t*));
