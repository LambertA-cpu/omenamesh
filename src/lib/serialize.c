
#include "common/types.h"
#include "lib/packet.h"

__CJLF_GENERICS serialize_packet(const Packet *pkt, u8__CJLF *buffer) {
	if (!pkt || !buffer)
		return;

	u8__CJLF *ptr = buffer;

	*ptr++ = pkt->type;

	for (int i = 0; i < ADDRESS; i++)
		*ptr++ = pkt->src_ip[i];
	for (int i = 0; i < ADDRESS; i++)
		*ptr++ = pkt->dest_ip[i];

	u16__CJLF seq = htons(pkt->seq_num);
	*(u16__CJLF *)ptr = seq;
	ptr += sizeof(seq);

	*ptr++ = pkt->ttl;

	u16__CJLF size = htons(pkt->payload_size);
	*(u16__CJLF *)ptr = size;
	ptr += sizeof(size);

	for (int i = 0; i < pkt->payload_size; i++)
		*ptr++ = pkt->data[i];

	return;
}

__CJLF_GENERICS deserialize_packet(const u8__CJLF *buffer, Packet *pkt) {
	if (!pkt || !buffer)
		return;

	const u8__CJLF *ptr = buffer;

	pkt->type = *ptr++;

	for (int i = 0; i < ADDRESS; i++)
		pkt->src_ip[i] = *ptr++;
	for (int i = 0; i < ADDRESS; i++)
		pkt->dest_ip[i] = *ptr++;

	pkt->seq_num = ntohs(*(u16__CJLF *)ptr);
	ptr += sizeof(u16__CJLF);

	pkt->ttl = *ptr++;

	pkt->payload_size = ntohs(*(u16__CJLF *)ptr);
	ptr += sizeof(u16__CJLF);

	for (int i = 0; i < pkt->payload_size; i++)
		pkt->data[i] = *ptr++;

	return;
}
