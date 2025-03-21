
#include "packet.h"

/* Mutex for synchronization (locking) */
// static pthread_mutex_t packet_lock = PTHREAD_MUTEX_INITIALIZER;
static LockManager *packet_lock = 0;

/* Queue packet - Add a packet to the queue & keep the writing as atomic as
 * possible, Locking to prevent simultaneous packet handling */
__CJLF_GENERICS enqueue_packet(PacketQueue *queue, Packet *pkt) {
	lock_init(packet_lock);

	acquire_lock(packet_lock, 0, 0);

	if (queue->rear == NULL) {
		queue->front = queue->rear = pkt;
	} else {
		queue->rear->next = pkt;
		queue->rear = pkt;
	}
	release_lock(packet_lock, queue->size);
	queue->size++;
}

/* Remove and return the front packet of the queue */
Packet *dequeue_packet(PacketQueue *queue) {
	acquire_lock(packet_lock, 0, 0);  // Lock to ensure thread safety

	if (queue->front == NULL) {
		/* No packets in queue */
		release_lock(packet_lock, queue->size);
		return 0;
	}

	Packet *pkt = queue->front;
	queue->front = queue->front->next;
	if (queue->front == NULL) {
		queue->rear = NULL;
	}
	release_lock(packet_lock, queue->size);
	queue->size--;
	return pkt;
}

#include "../common/debug.h"

/* Forward packet - This function simulates sending/forwarding a packet */
__CJLF_GENERICS forward_packet(PacketQueue *queue, Packet *pkt) {
	OMENA_MESH_LOG(1, "Forwarding packet with seq_num: %d\n", pkt->seq_num);

	/*handle all packets*/
	switch (pkt->type) {
		case PACKET_TYPE_DATA:
			OMENA_MESH_TODO("TODO!");
			break;
		case PACKET_TYPE_HANDSHAKE:
			OMENA_MESH_TODO("TODO!");
			break;
		case PACKET_TYPE_ACK:
			OMENA_MESH_TODO("TODO!");
			break;
		case PACKET_TYPE_HELLO:
			OMENA_MESH_TODO("TODO!");
			break;
		case PACKET_TYPE_AUTH:
			OMENA_MESH_TODO("TODO!");
			break;
		case PACKET_TYPE_ERROR:
			OMENA_MESH_TODO("TODO!");
			break;
		default:
			OMENA_MESH_TODO("TODO!");
	}

	return;
}

/* Handle packet - Process received packets */
__CJLF_GENERICS handle_packet(PacketQueue *queue, Packet *pkt, char *my_ip) {
	switch (pkt->type) {
		case PACKET_TYPE_DATA:
			OMENA_MESH_LOG(1, "Handling data packet: %s\n",
				       pkt->data);
			break;
		case PACKET_TYPE_HANDSHAKE:
			OMENA_MESH_LOG(1, "Processing handshake packet\n");
			break;
		case PACKET_TYPE_ACK:
			OMENA_MESH_LOG(1, "Processing acknowledgment packet\n");
			break;
		case PACKET_TYPE_HELLO:
			OMENA_MESH_LOG(1, "Handling node discovery packet\n");
			break;
		case PACKET_TYPE_AUTH:
			OMENA_MESH_LOG(
			    1, "Processing authentication request packet\n");
			break;
		case PACKET_TYPE_ERROR:
			OMENA_MESH_LOG(1, "Error notification received: %s\n",
				       pkt->data);
			break;
		default:
			OMENA_MESH_LOG(2, "AHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH\n");
	}
}
