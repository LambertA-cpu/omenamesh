#include "net.h"

/*dynamically discover IP, instead of distributing IPs and maintaning
 *a list(currently what we are doing)*/

void get_local_ip(char *buffer, size_t len) {
	struct ifaddrs *ifaddr, *ifa;
	int family;

	if (getifaddrs(&ifaddr) == -1)
		return;

	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr == NULL)
			continue;

		family = ifa->ifa_addr->sa_family;
		if (family == AF_INET || family == AF_INET6) {
			getnameinfo(ifa->ifa_addr,
				    (family == AF_INET)
					? sizeof(struct sockaddr_in)
					: sizeof(struct sockaddr_in6),
				    buffer, len, NULL, 0, NI_NUMERICHOST);
			OMENA_MESH_LOG(1, "Interface: %s, IP: %s\n",
				       ifa->ifa_name, buffer);
		}
	}

	freeifaddrs(ifaddr);
}