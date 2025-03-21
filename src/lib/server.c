
#include "net.h"

__CJLF_GENERICS *server_thread(__CJLF_GENERICS *arg, i32__CJLF port) {
	i32__CJLF server_fd, new_socket;
	struct sockaddr_in server_addr, client_addr;

	socklen_t addr_len = sizeof(client_addr);
	i8__CJLF buffer[1024] = { 0 };

	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
		return 0;

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);

	if (bind(server_fd, (struct sockaddr *)&server_addr,
		 sizeof(server_addr)) < 0) {
		perror("Bind failed");
		close(server_fd);
		return NULL;
	}

	if (listen(server_fd, 3) < 0) {
		perror("Listen failed");
		close(server_fd);
		return NULL;
	}

	/*TODO*/
	while ((new_socket = accept(server_fd, (struct sockaddr *)&client_addr,
				    &addr_len)) >= 0)
		;

	close(server_fd);
	return NULL;
}