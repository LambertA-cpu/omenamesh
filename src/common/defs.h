/*LICENCE under CJLF*/

#ifndef __DEFS_H
#define __DEFS_H

#include <arpa/inet.h>

typedef struct {
	int socket; /*our socket node fd*/
	struct sockaddr_in address;
} Node;

#endif /*__DEFS_H*/