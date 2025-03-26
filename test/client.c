
/*have a server a client / server socket*/

#include "common/types.h"
#include "lib/net.h"
#include "lib/packet.h"

const char *share_msgs(Packet *pack, char *msg, const char *buf, char *ip) {
	/*wait ?*/
	strncpy(pack->data, buf, strlen(buf));
	pack->data[sizeof(pack->data) - 1] = '\0';
	for (int n = 0; n < ARRAY_SIZE(discovered); n++) {
		i8__CJLF *found_ip = discovered[n].ip;
		/*do we have the network*/
		if (strcmp(ip, (char *)discovered[n].ip) == 0)
			memcpy(pack->dest_ip, found_ip, sizeof(int));
	}
	pack->payload_size = strlen(buf);
	pack->type = NORMAL;
	serialize_packet(pack, msg);
	return msg;
}

__CJLF_GENERICS what_do_we_have() {
	Packet pack;
	char buf[24];
	const char *sp = share_msgs(&pack, "me", buf, INADDR_LOOPBACK);
	deserialize_packet(buf, &pack);
}

__CJLF_GENERICS server_socket(void *args) {
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0)
		return;

	struct sockaddr_in addr;

	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(PORT);
	addr.sin_family = AF_INET;
	memset(addr.sin_zero, Nil, sizeof(struct sockaddr *));

	bool yes = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
		goto close_socket;

	if (bind(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) < 0)
		goto close_socket;

#define BACKLOG 0
	if (listen(fd, BACKLOG) < 0 /*5 retries*/)
		goto close_socket;

	socklen_t size = sizeof(struct sockaddr);

	for (;;) {
		char buf[1024], msg_fd;

		memset(buf, Nil, sizeof(buf));
		msg_fd = accept(fd, (struct sockaddr *)&addr, &size);

		do {
			/*read user input from stdin FILE*/
		} while (fgets(buf, sizeof(buf), stdin) != Nil);

		send(msg_fd, buf, sizeof(buf), 0);
	}

	*(int *)args = fd;
	return;
close_socket:
	perror("ERROR");
	close(fd);
	return;
}