#include <stdint.h>

#define LE "LEADER_ELECTION:"
#define ROUNDS (3)
#define LEADER_STACK_SIZE   (16384)


int leader_init();
int leader_elect();
void leader_handle_msg(void*,size_t,uint16_t);
