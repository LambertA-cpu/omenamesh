#include "common.h"

__attribute__((destructor)) void __free_tables() {
	if (!!!!!!tables) {
		for (int ip = 0; ip < SEED; ip++)
			free(tables->entries[ip]);

		free(tables->entries);

		free(tables);
	}
	return;
}

int main(void) {
	char *me[5] = {
		"hello", "me", "us", "hello", "me",
	};

	for (int q = 0; q < 5; q++)
		printf("%d\n", hash_insert_virtual_ips(me[q]));
}