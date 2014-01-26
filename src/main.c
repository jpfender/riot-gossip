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
#include "nativenet.h"
#include "msg.h"
#include "random.h"
#include "thread.h"

#include "gossip.h"
#include "leader.h"
#include "timesync.h"

#define ENABLE_DEBUG (0)
#include <debug.h>

#define ENABLE_WARN (1)
#include <warn.h>

char leader_stack[LEADER_STACK_SIZE];

void handle_msg(void* msg_text, size_t size, uint16_t src){
    size_t i;

    if (strncmp(msg_text, LE, strlen(LE)) == 0) {
        if( ! leader_get_active() ){
            thread_create( leader_stack, LEADER_STACK_SIZE, PRIORITY_MAIN - 1,
                            CREATE_STACKTEST, leader_elect, "Leader");
            leader_set_active(1);
        }
        leader_handle_msg(msg_text, size, src);
    } 
    else if (strncmp(msg_text, TS, strlen(TS)) == 0) {
        timesync_handle_msg(msg_text, size, src);
    }
    else {
        // fallback
        // assume we were sending a simple string for now
        puts("Unknown message: ");
        for( i=0 ; i<size; i++){
            putchar( ((char*)msg_text)[i] );
        }
        putchar('\n');
    }
}

int handle_remove_neighbour(gossip_node_t* neighbour) {
    return leader_handle_remove_neighbour(neighbour);
}

int main(void)
{
    uint16_t id;
    timex_t time;
    transceiver_type_t transceiver = TRANSCEIVER_NATIVE;
    char is_leader=1;
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
    id = (uint16_t)genrand_uint32();
    if( 0 != gossip_init(id,transceiver) ){
        DEBUG("D: gossip_init(%d) failed\n", transceiver);
        return 1;
    }
    printf("I am %i\n",id);


    DEBUG("D: Announcing.\n");
    int r = gossip_announce();
    if( 1 != r ){
        DEBUG("D: gossip_announce() failed with %i\n", r);
    }
    // initiate leader election
    printf("Starting initial leader election.\n");
    leader_init();

    // TODO: sleep for now, should receive IPC logger msg and printf here
    int count = 0;
    char msg_buffer[strlen(PREAMBLE) + strlen(MSG) + 100];
    while (1) {
        unsigned long delay = 1000000 * ((genrand_uint32()%10) + 1);
        printf("sleeping %lums\n", delay/1000);
        vtimer_usleep(delay);
        printf("Re-Announcing.\n");
        gossip_announce();
        neighbours = gossip_get_all_neighbours();
        printf("There are %d neighbours\n", neighbours->length);
        printf("Current leader: %d\n", leader_get_leader());
        // Send a message to a random neighbour
        //sprintf(msg_buffer, "%s%sThis is message %i from node %i",
        //        PREAMBLE, MSG, count++, id);
        //gossip_node_t* node = gossip_get_neighbour_random();
        //gossip_send(node, msg_buffer, strlen(msg_buffer));

        gossip_cleanup();
        gossip_free_node_list(neighbours);
    }
    return 0;
}
