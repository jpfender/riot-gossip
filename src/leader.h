#include <stdint.h>

#define LE "LEADER_ELECTION:"
#define ROUNDS 10


uint16_t leader_elect(uint16_t);
void leader_handle_msg(void*,size_t,uint16_t);
