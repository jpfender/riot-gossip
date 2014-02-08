#include <stdint.h>

#define LE "LEADER_ELECTION:"
#define ROUNDS (3)
#define LEADER_STACK_SIZE   KERNEL_CONF_STACKSIZE_PRINTF * 2


char leader_get_active(void);
void leader_set_active(char value);
void leader_set_leader(uint16_t new_leader);
uint16_t leader_get_leader(void);
void leader_set_initialized(char v);
char leader_get_initialized(void);
int leader_init(void);
void leader_elect(void);
void leader_handle_msg(void*,size_t,uint16_t);
int leader_handle_remove_neighbour(gossip_node_t*);
char leader_is_leader(void);
