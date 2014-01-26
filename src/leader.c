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

#define ENABLE_WARN (1)
#include "warn.h"

uint16_t leader=0;
char active=0;
uint16_t election_round=0;

char leader_get_active(){
    return active;
}

void leader_set_active(char value){
    active=value;
}

uint16_t leader_get_leader(){
    return leader;
}

int leader_init(){
    char msg_buffer[strlen(PREAMBLE) + strlen(MSG) + strlen(LE) + 100];
    gossip_node_t* node;

    node = gossip_get_neighbour(RANDOM);
    while (node->id == leader) {
        node = gossip_get_neighbour(RANDOM);
    }

    leader=gossip_id;
    DEBUG("D: initial leader: %d\n",leader);
    sprintf(msg_buffer, "%s%s%s%03i%i", PREAMBLE, MSG, LE, election_round, leader);

    if(!node){
        WARN("W: no neighbours, election failed.\n");
        return 1;
    } else {
        return gossip_send(node, msg_buffer, strlen(msg_buffer));
    }
}

int leader_elect(){
    char msg_buffer[strlen(PREAMBLE) + strlen(MSG) + strlen(LE) + 100];
    gossip_node_t* node;

    for(int i=0;i<ROUNDS;i++){
        sprintf(msg_buffer, "%s%s%s%03i%i", PREAMBLE, MSG, LE, election_round, leader);
        node = gossip_get_neighbour(RANDOM);
        if(!node){
            WARN("W: no neighbours, election failed.\n");
            return 1;
        }
        gossip_send(node, msg_buffer, strlen(msg_buffer));
        vtimer_usleep(1000*1000*2);
    }
    leader_set_active(0);
    return 0;
}

void leader_handle_msg(void* msg_text, size_t size, uint16_t src){
    uint16_t received_leader;
    uint16_t round;
    gossip_node_t* node;
    char round_buffer[3];
    char msg_buffer[strlen(PREAMBLE) + strlen(MSG) + strlen(LE) + 100];

    strncpy( round_buffer, (char*)msg_text+strlen(LE) , sizeof(round_buffer) );
    round = atol(round_buffer);

    DEBUG("D: got round %i (current round %i)\n",round,election_round);

    // a new election round, invalidate leader and elect the next one
    if(round > election_round) { // TODO: fix possible overflow
        DEBUG("D: got new election round %i (was %i)\n",round,election_round);
        leader = gossip_id;
        election_round = round;
        leader_init();
        return;
    }

    // got leader from an old round, inform sending node
    if(round < election_round) { // TODO: fix possible overflow
        DEBUG("D: got round %i (current is %i) informing sender\n",round,election_round);
        sprintf(msg_buffer, "%s%s%s%03i%i", PREAMBLE, MSG, LE, election_round, leader);
        node = gossip_find_node_by_id(src);
        gossip_send(node, msg_buffer, strlen(msg_buffer));
        return;
    }

    received_leader = atol((char*)msg_text+strlen(LE)+3);
    printf("received candidate: %i\n",received_leader);

    // if new message contains worse leader candidate, inform node directly
    // TODO: add custom metrics functions here instead of a<b
#if 1
    if(received_leader < leader ){
        DEBUG("D: discarding candidate and informing sender\n");
        sprintf(msg_buffer, "%s%s%s%03i%i", PREAMBLE, MSG, LE, round, leader);
        node = gossip_find_node_by_id(src);
        gossip_send(node, msg_buffer, strlen(msg_buffer));
    }
#endif

    // update leader if we receive a better candidate
    if(received_leader > leader ){
        DEBUG("D: adding a new, better leader\n");
        leader = received_leader;
    }
}

int leader_handle_remove_neighbour(gossip_node_t* neighbour) {
    if (neighbour->id == leader) {
        // Leader was removed from neighbour table; start a new election
        // round
        election_round++;
        // This obviously won't work as intended as of now;
        // leader_elect() needs to be a thread. But this is the general
        // idea.
        leader_init();
    }

    return OK;
}
