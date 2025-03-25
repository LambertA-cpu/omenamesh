#ifndef ROUTE_H
#define ROUTE_H

#include "lib/packet.h"

#define MAX_ROUTES 10

struct __routes {
	char ip[ADDRESS];
	int socket_fd;
} Routes[MAX_ROUTES];

void add_route(const char *ip, int socket_fd);
#endif /*! ROUTE_H*/
