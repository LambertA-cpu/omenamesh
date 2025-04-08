#ifndef ROUTE_H
#define ROUTE_H

#include "packet.h"

#define MAX_ROUTES 10

struct __routes {
	char ip[ADDRESS];
	int socket_fd;
} Routes[MAX_ROUTES];

extern struct __routes routes;

extern i32__CJLF get_local_ip(char *buffer, i64__CJLF len);
extern void add_route(const char *ip, int socket_fd);
#endif /*! ROUTE_H*/
