
//! we will work with virtual IPs

// #include "net.h"
// #include "route.h"
// #include "types.h"

// /* Dynamically discover IP instead of maintaining a list */
// i32__CJLF get_local_ip(char *buffer, i64__CJLF len) {
// 	struct ifaddrs *ifaddr, *ifa;
// 	i32__CJLF family, found = 0;

// 	if (getifaddrs(&ifaddr) == -1) {
// 		perror("getifaddrs() failed");
// 		return -1;
// 	}

// 	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
// 		if (ifa->ifa_addr == NULL)
// 			continue;

// 		family = ifa->ifa_addr->sa_family;
// 		if (family == AF_INET || family == AF_INET6) {
// 			if (getnameinfo(ifa->ifa_addr,
// 					(family == AF_INET)
// 					    ? sizeof(struct sockaddr_in)
// 					    : sizeof(struct sockaddr_in6),
// 					buffer, len, NULL, 0,
// 					NI_NUMERICHOST) == 0) {
// 				OMENA_MESH_LOG(1, "Interface: %s, IP: %s\n",
// 					       ifa->ifa_name, buffer);
// 				found = 1;
// 				break;
// 			}
// 		}
// 	}

// 	freeifaddrs(ifaddr);
// 	return found ? 0 : -1;
// }

// __CJLF_GENERICS broadcast_presence() {
// 	i32__CJLF udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
// 	if (udp_sock < 0)
// 		return;

// 	i32__CJLF broadcast = 1;
// 	if (setsockopt(udp_sock, SOL_SOCKET, SO_BROADCAST, &broadcast,
// 		       sizeof(broadcast)) < 0)
// 		goto close_socket;

// 	struct sockaddr_in addr = {
// 		.sin_family = AF_INET,
// 		.sin_port = htons(PORT),
// 		.sin_addr.s_addr = inet_addr("255.255.255.255"),
// 	};

// 	char *stmt = "DISCOVER";
// 	if (sendto(udp_sock, stmt, strlen(stmt), 0, (struct sockaddr *)&addr,
// 		   sizeof(addr)) < 0)
// 		return;

// 	goto close_socket;

// close_socket:
// 	close(udp_sock);
// 	return;
// }

// __CJLF_GENERICS listen_for_discovery() {
// 	i32__CJLF udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
// 	if (udp_sock < 0)
// 		return;

// 	struct sockaddr_in addr = {
// 		.sin_family = AF_INET,
// 		.sin_port = htons(PORT),
// 		.sin_addr.s_addr = INADDR_ANY,
// 	};

// 	if (bind(udp_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
// 		return;

// 	i8__CJLF buffer[MAX_DATA_SIZE];
// 	while (1) {
// 		struct sockaddr_in sender_addr;
// 		socklen_t addr_len = sizeof(sender_addr);
// 		i32__CJLF bytes_received =
// 		    recvfrom(udp_sock, buffer, sizeof(buffer) - 1, 0,
// 			     (struct sockaddr *)&sender_addr, &addr_len);

// 		if (bytes_received < 0) {
// 			perror("recvfrom() failed");
// 			continue;
// 		}

// 		buffer[bytes_received] = Nil;
// 		char sender_ip[INET_ADDRSTRLEN];
// 		inet_ntop(AF_INET, &sender_addr.sin_addr, sender_ip,
// 			  INET_ADDRSTRLEN);
// 		printf("[DISCOVERY] Found peer: %s\n", sender_ip);

// 		i32__CJLF new_peer = connect_to_peer(sender_ip);
// 		if (new_peer > 0) {
// 			add_route(sender_ip, udp_sock);
// 		}
// 	}
// }
