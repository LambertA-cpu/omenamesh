#ifndef PACKET_H
#define PACKET_H

#define MAX_DATA_SIZE 1024 /*enough? */
#define ADDRESS 16

// Normal data packet
#define PACKET_TYPE_DATA 0x01
// Initial handshake packet
#define PACKET_TYPE_HANDSHAKE 0x02
// Acknowledgment packet
#define PACKET_TYPE_ACK 0x03
// Node discovery packet
#define PACKET_TYPE_HELLO 0x04
// Authentication request packet
#define PACKET_TYPE_AUTH 0x05
// Error notification packet
#define PACKET_TYPE_ERROR 0x06

#include "../common/types.h"

typedef struct __packet {
	/* Packet type */
	u8__CJLF type;
	/* Source IP*/
	i8__CJLF src_ip[ADDRESS];
	/* Destination IP*/
	i8__CJLF dest_ip[ADDRESS];
	/* Sequence number (for ordering)*/
	u16__CJLF seq_num;
	/* Time-to-Live (hops before discard)*/
	u8__CJLF ttl;
	/*Actual data length */
	u16__CJLF payload_size;

	/*so, hmm the max buf is 1024, figuring out how to setvbuf(sometimes we
	 * need urgency)*/
	char data[MAX_DATA_SIZE];
	/*queue the next packet*/
	struct __packet *next;
} Packet;

/*queue packet*/

typedef struct {
	Packet *front;
	Packet *rear;
	int size;
} PacketQueue;

/*packet queue*/
OMENAMESH_API __CJLF_GENERICS enqueue_packet(PacketQueue *queue, Packet *pkt);
OMENAMESH_API Packet *dequeue_packet(PacketQueue *queue);
OMENAMESH_API __CJLF_GENERICS forward_packet(PacketQueue *queue, Packet *pkt);
OMENAMESH_API __CJLF_GENERICS handle_packet(PacketQueue *queue,
					    Packet *pkt,
					    char *my_ip);

typedef struct RoutingTable {
	char dest_ip[16];
	char next_hop[16];
	int hops;
} RoutingTable;

OMENAMESH_API __CJLF_GENERICS add_route(char *dest_ip,
					char *next_hop,
					int hops);

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "../common/debug.h"
#include "../common/lock.h"

#endif