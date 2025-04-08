
#include "net.h"
#include "packet.h"

#define BROADCAST_PORT PORT
#define VIRTUAL_IP "169.0.0.0"
#define BACKLOGS MAX_LOCKS

__CJLF_GENERICS send_udp_broadcast() {
	i32__CJLF sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		perror("socket() failed");
		return;
	}

	i32__CJLF broadcastEnable = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcastEnable,
		       sizeof(broadcastEnable)) < 0) {
		perror("setsockopt() failed");
		close(sockfd);
		return;
	}

	struct sockaddr_in broadcast_addr = {
		.sin_family = AF_INET,
		.sin_port = htons(BROADCAST_PORT),
		.sin_addr.s_addr = INADDR_BROADCAST,
	};

	char real_ip[ADDRESS] = { 0 };
	(__CJLF_GENERICS) get_local_ip(real_ip, sizeof(real_ip));

	char message[128];
	snprintf(message, sizeof(message), "VIRTUAL_IP=%s REAL_IP=%s PORT=%d",
		 VIRTUAL_IP, real_ip, PORT);

	if (sendto(sockfd, message, strlen(message), 0,
		   (struct sockaddr *)&broadcast_addr,
		   sizeof(broadcast_addr)) < 0) {
		close(sockfd);
		return;
	}

	close(sockfd);
}

/* Setup SERVER */
i8__CJLF start_server_t() {
	i32__CJLF server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0) {
		perror("socket() failed");
		exit(EXIT_FAILURE);
	}

	i32__CJLF opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) <
	    0) {
		perror("setsockopt() failed");
		close(server_fd);
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in server_addr = {
		.sin_family = AF_INET,
		.sin_addr.s_addr = INADDR_ANY,
		.sin_port = htons(PORT),
	};

	if (bind(server_fd, (struct sockaddr *)&server_addr,
		 sizeof(server_addr)) < 0) {
		perror("bind() failed");
		close(server_fd);
		exit(EXIT_FAILURE);
	}

	if (listen(server_fd, BACKLOGS) < 0) {
		perror("listen() failed");
		close(server_fd);
		exit(EXIT_FAILURE);
	}

	set_nonblocking(server_fd);

	printf("[SERVER] Listening on port %d...\n", PORT);
	return server_fd;
}
