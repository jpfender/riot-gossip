/*! \file gossip.h
    \brief Pulblic Functions for Gossip Layer 

    TODO: Details
*/

#include <stdint.h>
#include <stdlib.h>
#include "thread.h"
#include "transceiver.h"
#include "protocol.h"

#ifdef MODULE_MSBA2
#define TRANSCEIVER_TYPE TRANSCEIVER_CC1100
#endif
#ifdef MODULE_NATIVENET
#define TRANSCEIVER_TYPE TRANSCEIVER_NATIVE
#define MAX_UID 1023
#define UID_LEN 4
#define UID_LEN_STR "4"
#define PACKET_LENGTH 62
#endif
#ifdef MODULE_CC110X_NG
#include "cc110x_ng.h"
#define TRANSCEIVER_TYPE TRANSCEIVER_CC1100
#define UID_LEN 3
#define UID_LEN_STR "3"
#endif

#define RADIO_STACK_SIZE    (KERNEL_CONF_STACKSIZE_DEFAULT*4)
#define OK      1
#define KEEP    0

/*! Global ID of the Node. */
uint32_t gossip_id;

/*! List of possible neighbor selection strategies for
 *  gossip_get_neighbour() function
 */
typedef enum gossip_strategy {
    RANDOM,
    OLDEST_FIRST
} gossip_strategy_t;

/*! This struct describes an entry in the neigbour list
 */
typedef struct gossip_node {
    /*! ID of the node. */
    uint16_t id;
    /*! timestamp since last message send to node. */
    uint32_t last_send;
    /*! timestamp since last message recieved from node. */
    uint32_t last_recv;
} gossip_node_t;

/*! This struct describes a list of nodes as returned by
 *  gossip_get_all_neighbours()
 */
typedef struct gossip_node_list {
    /*! Number of nodes in this list. */
    size_t length;
    /*! List of nodes */
    gossip_node_t** nodes;
} gossip_node_list_t;

char gossip_radio_stack_buffer[RADIO_STACK_SIZE];


/*! \brief send hello message to broadcast adress
 *         (sould be called on a regular basis)
 *  \return 0 on success, error code in case of failure
 */ 
int gossip_announce(void);

/*! \brief register the message handler and init gossip system
 *  \param id Global ID to set
 *  \param transceiver_type transceiver type
 *  \return 0 on success, error code in case of failure
 */
int gossip_init(uint16_t id, transceiver_type_t transceiver_type);

/*! \brief get a list of all known neighbours
 *  \return node list on success, NULL in case of failure
 */
gossip_node_list_t* gossip_get_all_neighbours(void);

/*! \brief free a node list returned by gossip_get_all_neighbours()
 *  \param nl node list to be freed
 */
void gossip_free_node_list(gossip_node_list_t* nl);

/*! \brief look through the list for a node with id
 *  \param id ID of node in question
 *  \return pointer to that node on success, NULL in case of failure
 */
gossip_node_t* gossip_find_node_by_id(uint16_t id);

/*! \brief choose specific node by some strategy
 *  \param strategy strategy by witch node should be chosen
 *  \return node on success, NULL in case of failure
 */
gossip_node_t* gossip_get_neighbour(gossip_strategy_t strategy);

/*! \brief choose a random node from neighbour list
 *  \return node on success, NULL in case of failure
 */
gossip_node_t* gossip_get_neighbour_random(void);

/*! \brief choose oldest node from neighbour list
 *  \return node on success, NULL in case of failure
 */
gossip_node_t* gossip_get_neighbour_oldest_first(void);


/*! \brief send a message to a given node
 *  \param node node to send message to
 *  \param gossip_message data to send
 *  \param len length of data to send
 *  \returns 0 on success, error code in case of failure
 */
int gossip_send(gossip_node_t* node, void *gossip_message, int len);

/*! \brief registers callback for received application messages
 *  \param handle function pointer for callback function that handels
 *         incomming messages
 * 
 *  handler signature: void handle(void *data, size_t length, uint16_t source)
 */
void gossip_register_msg_handler(void (*handle) (void*,size_t,uint16_t));

/*! \brief removes neighbours from global list after CLEANUP_THRESHOLD
 *         time
 */
void gossip_cleanup(void);

// Hooks
// =====

/*! \brief register a handler that is run upon removing a node from the
 *         neighbour list
 *  \param handle function pointer for callback function that decides if
 *         a node should be removed from neighbor list or not
 *
 *  handler signature: int handle(gossip_node_t node_to_remove) \n
 *  if handle returns 0 the node is removed else its kept in the
 *  list
 */
void gossip_register_on_remove_neighbour_handler(int (*handle) (gossip_node_t*));
