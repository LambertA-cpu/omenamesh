
#include "common/types.h"
#include "net.h"

i32__CJLF clients[MAX_NODES] = { 0 };

/*1, will definately change, its mostly here for testcases*/
__CJLF_GENERICS handle_user_input() {
	char message[MAX_DATA_SIZE];

	if (fgets(message, sizeof(message), stdin) != Nil) {
		message[strcspn(message, "\n")] = '\0';

		printf("[SENDING] %s\n", message);

		for (int i = 0; i < MAX_NODES; i++) {
			if (clients[i] > 0) {
				send(clients[i], message, strlen(message), 0);
			}
		}
	}
}

/*2*/
__CJLF_GENERICS handle_client_data(int client_fd, int index) {
	char buffer[MAX_DATA_SIZE];
	int bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);

	if (bytes_read > 0) {
		buffer[bytes_read] = '\0';
		printf("[RECEIVED] %s\n", buffer);

		for (int i = 0; i < MAX_NODES; i++) {
			if (clients[i] > 0 && clients[i] != client_fd) {
				send(clients[i], buffer, bytes_read, 0);
			}
		}
	} else if (bytes_read == 0) {
		printf("[CLIENT] Disconnected\n");
		close(client_fd);
		clients[index] = 0;
	}
}

/*TCP*/
i32__CJLF connect_to_node(const char *ip) {
	i32__CJLF sock;
	struct sockaddr_in serv_addr;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return FAILURE;

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(8000);
	;
	if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0)
		goto close_client_socket;

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		goto close_client_socket;

	return sock;

close_client_socket:
	close(sock);
	return FAILURE;
}

i32__CJLF connect_to_peer(const char *peer_ip) {
	i32__CJLF client_fd;
	struct sockaddr_in peer_addr;

	client_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (client_fd < 0)
		return -1;

	peer_addr.sin_family = AF_INET;
	peer_addr.sin_port = htons(PORT);
	inet_pton(AF_INET, peer_ip, &peer_addr.sin_addr);

	if (connect(client_fd, (struct sockaddr *)&peer_addr,
		    sizeof(peer_addr)) < 0) {
		close(client_fd);
		exit(Nil);
	}

	set_nonblocking(client_fd);
	printf("[CLIENT] Connected to %s\n", peer_ip);
	return client_fd;
}

#include "route.h"

__CJLF_GENERICS handle_new_connection(int server_fd) {
	struct sockaddr_in client_addr;
	socklen_t addr_len = sizeof(client_addr);
	int new_client =
	    accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);

	if (new_client > 0) {
		set_nonblocking(new_client);
		char ip[ADDRESS];
		inet_ntop(AF_INET, &client_addr.sin_addr, ip, ADDRESS);
		add_route(ip, new_client);
		printf("[SERVER] New route added for %s\n", ip);
	}
}
