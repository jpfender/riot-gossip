#include <stdint.h>

#define LE "LEADER_ELECTION:"
#define ROUNDS (3)
#define LEADER_STACK_SIZE   MINIMUM_STACK_SIZE


char leader_get_active();
void leader_set_active(char value);
void leader_set_leader(uint16_t new_leader);
uint16_t leader_get_leader();
void leader_set_initialized(char v);
char leader_get_initialized();
int leader_init();
void leader_elect();
void leader_handle_msg(void*,size_t,uint16_t);
int leader_handle_remove_neighbour(gossip_node_t*);
