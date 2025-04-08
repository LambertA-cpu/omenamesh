
#include "route.h"

__CJLF_GENERICS add_route(const char *ip, int socket_fd) {
	for (int i = 0; i < MAX_ROUTES; i++) {
		if (Routes[i].socket_fd == 0) {
			strcpy(Routes[i].ip, ip);
			Routes[i].socket_fd = socket_fd;
			break;
		}
	}
}
