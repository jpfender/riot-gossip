#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <string.h>

#include "vtimer.h"
#include "rtc.h"
#include "board_uart0.h"
#include "board.h"
#include "transceiver.h"
#include "posix_io.h"
#include "msg.h"
#include "random.h"
#include "thread.h"

#include "gossip.h"
#include "leader.h"
#include "timesync.h"
#include "blink.h"

#define ENABLE_DEBUG (1)
#include <debug.h>

#define ENABLE_WARN (1)
#include <warn.h>

char leader_stack[LEADER_STACK_SIZE];
char blink_stack[BLINK_STACK_SIZE];

void handle_msg(void* msg_text, size_t size, uint16_t src){
    size_t i;

    /* MSG is for Leader Election */
    if (strncmp(msg_text, LE, strlen(LE)) == 0) {
        DEBUG("D: received a leader election msg\n");
        if( ! leader_get_active() ){
            thread_create( leader_stack, LEADER_STACK_SIZE, PRIORITY_MAIN-1,
                            0, leader_elect, "Leader");
            leader_set_active(1);
        }
        leader_handle_msg(msg_text, size, src);
    }
    /* MSG is for Time Synchronization */
    else if (strncmp(msg_text, TS, strlen(TS)) == 0) {
        DEBUG("D: received a timesync msg\n");
        timesync_handle_msg((char*)msg_text + strlen(TS), size - strlen(TS), src);
    }
    /* MSG is unknown */
    else {
        WARN("W: Unknown message received.");
        // format string black hole ahead
        DEBUG("D: %s", msg_text);
    }
}

int handle_remove_neighbour(gossip_node_t* neighbour) {
    return leader_handle_remove_neighbour(neighbour);
}

int main(void)
{
    uint16_t id;
    timex_t time;
    transceiver_type_t transceiver = TRANSCEIVER_TYPE;
    gossip_node_list_t *neighbours;

    printf("\n\t\t\tWelcome to RIOT\n\n");

    puts("Initializing gossiping.");

    puts("Registering sample gossip message handler.");
    gossip_register_msg_handler(handle_msg);

    puts("Registering gossip on_remove_neighbour handler.");
    gossip_register_on_remove_neighbour_handler(handle_remove_neighbour);

    vtimer_init();
    vtimer_now(&time);
    genrand_init( time.microseconds );
    id = 1+((uint16_t)genrand_uint32())%254;
    if( 0 != gossip_init(id,transceiver) ){
        DEBUG("D: gossip_init(%d) failed\n", transceiver);
        return 1;
    }
    printf("I am %i\n",id);
    leader_set_leader(id);


    DEBUG("D: Announcing.\n");
    if( ! gossip_announce() ){
        DEBUG("D: gossip_announce() failed\n");
    }

#if 0
    thread_create( blink_stack, BLINK_STACK_SIZE , PRIORITY_MAIN-2,
                            0, blink, "Blink");
#endif

    // TODO: sleep for now, should receive IPC logger msg and printf here
    while (1) {
        unsigned long delay = 1000000 * ((genrand_uint32()%10) + 1);
        printf("sleeping %lums\n", delay/1000);
        vtimer_usleep(delay);
        printf("Re-Announcing.\n");
        gossip_announce();
        neighbours = gossip_get_all_neighbours();
        printf("There are %d neighbours\n", neighbours->length);
        if( neighbours->length && ! leader_get_initialized() ){
            printf("Starting initial leader election.\n");
            leader_set_leader(gossip_id);
            leader_init();
            leader_set_initialized(1);
        }
        printf("Current leader: %d\n", leader_get_leader());
        // Send a message to a random neighbour
        //sprintf(msg_buffer, "%s%sThis is message %i from node %i",
        //        PREAMBLE, MSG, count++, id);
        //gossip_node_t* node = gossip_get_neighbour_random();
        //gossip_send(node, msg_buffer, strlen(msg_buffer));

        //Time synchronization IF I am the leader OR if I received my
        //timestamp from the leader
        if (leader_get_leader() == gossip_id || timesync_get_trusted())
            timesync_init();

        gossip_cleanup();
        gossip_free_node_list(neighbours);
    }
    return 0;
}
