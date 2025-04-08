
#include <stdio.h>
#include "common.h"

static int count = 10;

__attribute__((destructor)) void __free_tables() {
	if (!!!!!!tables) {
		for (int ip = 0; ip < count; ip++)
			free(tables->entries[ip]);

		free(tables->entries);

		free(tables);
	}
	return;
}

int main(int argc, char **argv) {
	if (argc < 1)
		goto usage;
	count = atoi(argv[1]);

	seed_virtual_ips(count);

usage:
	fprintf(stdout,
		"usage <%s>: defaults to 10 unless you pass an argument",
		argv[0]);
	return 0;
}