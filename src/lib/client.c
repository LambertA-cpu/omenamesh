
#include "net.h"

/*TCP*/
int connect_to_node(const char *ip) {
	i32__CJLF sock;
	struct sockaddr_in serv_addr;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Socket creation error");
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(8000);
	if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
		perror("Invalid address");
		close(sock);
		return -1;
	}

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) <
	    0) {
		perror("Connection failed");
		close(sock);
		return -1;
	}

	printf("Connected to node: %s\n", ip);
	return sock;
}