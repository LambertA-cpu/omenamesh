
#include <stdio.h>
#include "test.h"

int main() {
	int p;
	server_socket(&p);
	printf("socket fd %d", p);
}