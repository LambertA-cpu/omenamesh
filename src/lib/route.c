
#include "route.h"
#include "common/types.h"
#include "lib/net.h"

__CJLF_GENERICS handle_new_connection(int server_fd) {
	struct sockaddr_in client_addr;
	socklen_t addr_len = sizeof(client_addr);
	int new_client =
	    accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);

	if (new_client > 0) {
		set_nonblocking(new_client);
		printf("[SERVER] New connection from %s\n",
		       inet_ntoa(client_addr.sin_addr));

		for (int i = 0; i < MAX_NODES; i++) {
			if (clients[i] == 0) {
				clients[i] = new_client;
				break;
			}
		}
	}
}

__CJLF_GENERICS add_route(const char *ip, int socket_fd) {
	for (int i = 0; i < MAX_ROUTES; i++) {
		if (Routes[i].socket_fd == 0) {
			strcpy(Routes[i].ip, ip);
			Routes[i].socket_fd = socket_fd;
			break;
		}
	}
}
