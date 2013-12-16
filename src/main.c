/*
 * Copyright (C) 2013 Ludwig Ortmann <ludwig.ortmann@fu-berlin.de>
 */

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

#define ENABLE_DEBUG (0)
#include <debug.h>

void handle_msg(void* data, size_t len){
    size_t i;

    // assume we were sending a simple string for now
    puts("got a message: ");
    for( i=0 ; i<len ; i++){
        putchar( ((char*)data)[i] );
    }
    putchar('\n');
}

int main(void)
{
    uint16_t id;
    timex_t time;
    transceiver_type_t transceiver = TRANSCEIVER_NATIVE;
    uint16_t leader;
    char is_leader=1;
    gossip_node_list_t *neighbours;

    // TODO: figure out what this does:
    posix_open(uart0_handler_pid, 0);

    printf("\n\t\t\tWelcome to RIOT\n\n");

    printf("Initializing gossiping.\n");
    vtimer_init();
    vtimer_now(&time);
    genrand_init( time.microseconds );
    id = (uint16_t)genrand_uint32();
    if( 0 != gossip_init(id,transceiver) ){
        DEBUG("gossip_init(%d) failed\n", transceiver);
        return 1;
    }
    printf("I am %i\n",id);

    puts("Registering sample gossip message handler.");
    gossip_register_msg_handler(handle_msg);

    DEBUG("Announcing.\n");
    int r = gossip_announce();
    if( 1 != r ){
        DEBUG("gossip_announce() failed with %i\n", r);
    }


    // TODO: sleep for now, should receive IPC logger msg and printf here
    int count = 0;
    char msg_buffer[strlen(PREAMBLE) + strlen(MSG) + 100];
    while (1) {
        vtimer_usleep(1000000 * (genrand_uint32()%10));
        DEBUG("Re-Announcing.\n");
        gossip_announce();
        neighbours = gossip_get_all_neighbours();
        printf("There are %d neighbours\n", neighbours->length);
        // initiate leader election
        printf("Starting leader election.\n");
        leader = leader_elect(id);
        if(leader!=id){
            is_leader=0;
        }
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
