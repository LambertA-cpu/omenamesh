

#include "common/types.h"
#include "lib/net.h"

/*Radio */

DiscoveredNetwork discovered[MAX_NETWORKS];
i32__CJLF discovered_count = 0;

/*ICMP checksum*/
u16__CJLF checksum(__CJLF_GENERICS *b, int len) {
	u16__CJLF *buf = b;
	u16__CJLF sum = 0;
	u16__CJLF result;

	for (sum = 0; len > 1; len -= 2)
		sum += *buf++;
	if (len == 1)
		sum += *(u8__CJLF *)buf;

	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	result = ~sum;
	return result;
}

/*TODO  Send ICMP ping and check if a response is received */
i32__CJLF send_ping(const char *ip) {
	i32__CJLF sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sockfd < 0)
		return 0;

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ip);

	i8__CJLF packet[64] = { 0 };

	struct icmphdr *icmp = (struct icmphdr *)packet;

	icmp->type = ICMP_ECHO;
	icmp->code = 0;
	icmp->checksum = 0;
	icmp->un.echo.id = getpid();
	icmp->un.echo.sequence = 1;
	icmp->checksum = checksum(icmp, sizeof(packet));

	struct timeval timeout = {
		PING_TIMEOUT,
		0,
	};
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

	if (sendto(sockfd, packet, sizeof(packet), 0, (struct sockaddr *)&addr,
		   sizeof(addr)) <= 0) {
		close(sockfd);
		return 0;
	}

	i8__CJLF buffer[128];
	struct sockaddr_in response_addr;
	socklen_t addr_len = sizeof(response_addr);

	if (recvfrom(sockfd, buffer, sizeof(buffer), 0,
		     (struct sockaddr *)&response_addr, &addr_len) <= 0) {
		close(sockfd);
		return 0;
	}

	close(sockfd);
	return 1;
}

/* Get local IP and subnet mask, ignoring loopback */
__CJLF_GENERICS get_local_ip_subnet(char *ip, char *subnet) {
	struct ifaddrs *ifaddr, *ifa;
	if (getifaddrs(&ifaddr) == -1)
		return;

	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET) {
			struct sockaddr_in *sa =
			    (struct sockaddr_in *)ifa->ifa_addr;
			struct sockaddr_in *mask =
			    (struct sockaddr_in *)ifa->ifa_netmask;

			if (sa->sin_addr.s_addr != htonl(INADDR_LOOPBACK)) {
				inet_ntop(AF_INET, &sa->sin_addr, ip,
					  INET_ADDRSTRLEN);
				inet_ntop(AF_INET, &mask->sin_addr, subnet,
					  INET_ADDRSTRLEN);
				break;
			}
		}
	}
	freeifaddrs(ifaddr);
}

i32__CJLF ip_to_int(const char *ip) {
	struct in_addr addr;
	inet_aton(ip, &addr);
	return ntohl(addr.s_addr);
}

__CJLF_GENERICS int_to_ip(u32__CJLF ip, char *buffer) {
	struct in_addr addr;
	addr.s_addr = htonl(ip);
	inet_ntop(AF_INET, &addr, buffer, INET_ADDRSTRLEN);
}

/*scan & add to array of cap 10*/
__CJLF_GENERICS scan_subnet(const char *base_ip, const char *subnet_mask) {
	u16__CJLF base = ip_to_int(base_ip);
	u16__CJLF mask = ip_to_int(subnet_mask);
	u16__CJLF network = base & mask;
	u16__CJLF broadcast = network | ~mask;

	printf("scanning range: %u - %u\n", network, broadcast);

	for (u64__CJLF ip = network + 1; ip < broadcast; ip++) {
		char ip_str[INET_ADDRSTRLEN];
		int_to_ip(ip, ip_str);

		if (network_count < MAX_NETWORKS && send_ping(ip_str)) {
			strcpy((char *)discovered[network_count++].ip, ip_str);
			printf("added: %s\n", ip_str);
		}
	}
}

i32__CJLF dicsover_networks_around_me() {
	char local_ip[INET_ADDRSTRLEN] = { 0 },
	     subnet_mask[INET_ADDRSTRLEN] = { 0 };

	get_local_ip_subnet(local_ip, subnet_mask);

	if (strlen(local_ip) == 0 || strlen(subnet_mask) == 0) {
		printf(
		    "Failed to get local IP. Are you connected to a "
		    "network?\n");
		return 1;
	}

	printf("Local IP: %s\n", local_ip);
	printf("Subnet Mask: %s\n", subnet_mask);

	printf("Scanning networks...\n");
	scan_subnet(local_ip, subnet_mask);

	printf("\nDiscovered networks:\n");
	for (int i = 0; i < network_count; i++) {
		printf("%d. %s\n", i + 1, discovered[i].ip);
	}

	return 0;
}
