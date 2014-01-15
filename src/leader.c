#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <protocol.h>
#include <gossip.h>
#include "leader.h"
#include "vtimer.h"

#define ENABLE_DEBUG (1)
#include "debug.h"

uint16_t leader=0;
uint16_t election_round=0;

uint16_t leader_elect(uint16_t source){
    int i=0;
    char msg_buffer[strlen(PREAMBLE) + strlen(MSG) + strlen(LE) + 100];
    gossip_node_t* node;

    leader=source;
    for(i=0;i<ROUNDS;i++){
        sprintf(msg_buffer, "%s%s%s%03i%i", PREAMBLE, MSG, LE, election_round, leader);
        node = gossip_get_neighbour(RANDOM);
        if(!node){
            DEBUG("Warning: no neighbours, election failed.\n");
            return leader;
        }
        gossip_send(node, msg_buffer, strlen(msg_buffer));
        vtimer_usleep(1000*1000*10);
    }
    return leader;
}

void leader_handle_msg(void* msg_text, size_t size, uint16_t src){
    uint16_t received_leader;
    uint16_t round;
    gossip_node_t* node;
    char msg_buffer[strlen(PREAMBLE) + strlen(MSG) + strlen(LE) + 100];

    round = atol((char*)msg_text+strlen(LE));
    if(round > election_round || election_round - round > 128)
        // A new leader election round has been started; discard old
        // leader and assume I am the leader
        leader = gossip_id;

    received_leader = atol((char*)msg_text+strlen(LE)+3);
    printf("received candidate: %i\n",received_leader);

    // if new message contains worse leader candidate, inform node directly
    // TODO: add custom metrics functions here instead of a<b
#if 0
    if(received_leader < leader ){
        DEBUG("discarding candidate and informing sender\n");
        sprintf(msg_buffer, "%s%s%s%03i%i", PREAMBLE, MSG, LE, round, leader);
        node = gossip_find_node_by_id(src);
        gossip_send(node, msg_buffer, strlen(msg_buffer));
    }
#endif
    // update leader if we receive a better candidate
    if(received_leader > leader ){
        DEBUG("adding a new, better leader\n");
        leader = received_leader;
    }

    if(round > election_round || election_round - round > 128) {
        election_round = round;
        leader_elect(leader);
    }

    printf("%i\n",leader);
}

int leader_handle_remove_neighbour(gossip_node_t* neighbour) {
    if (neighbour->id == leader) {
        // Leader was removed from neighbour table; start a new election
        // round
        election_round++;
        // This obviously won't work as intended as of now;
        // leader_elect() needs to be a thread. But this is the general
        // idea.
        leader_elect(gossip_id);
    }

    return OK;
}
