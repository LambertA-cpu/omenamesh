/*! LICENCED UNDER CJLF 2025(c)
 *!
 *! This file has hashmap for ip(real) value ip(virtual) key, the reason for the
 *! hashmap is because the ip are generated in random fashion & yeah the map
 *! helps to keep everything unique so we dont have ip conflict later in the
 *! network, OK that is not actually the reason, the thing is different networks
 *! have almost similar IP addresses, the idea is we map actual real IP
 *! behind a virtual IP, when you share something across the network, the
 *! virtual then maps to the real IP. IDK if that makes sense but that is
 *! the whole idea behind our virtual IP addressing.
 *!
 *! OK this will make sense, y'know how we have virtual memory which is
 *! translated to physical memory, blah blah TLB MMU PDE all that?. Yeah thats
 *! it. You can do anything with the virtual memory (dereference, map files
 *! ...etc except its not what the computer will see, Just an illusion for your
 *! user process). If u got this far in the comment you're beautiful & deserve
 *! the world :)
 */

#include "common.h"

#define STRING (ADDRESS_LEN / 2)

struct __table *tables;

void initialize_table(size_t cap, size_t block) {
	tables = xmalloc(sizeof(*tables));
	tables->entries = xcalloc(cap, block);
	return;
}

void seed_virtual_ips(size_t seed_count) {
	initialize_table(seed_count, STRING);
	/*! rxn for hashmap*/
	for (int elements = 0; elements < seed_count; elements++) {
		tables->ip = xmalloc(STRING);
		snprintf(tables->ip, ADDRESS_LEN, "%d:%d:%d", rand() % 255,
			 rand() % 255, rand() % 255);
		if (hash_insert_virtual_ips(tables->ip))
			tables->entries[elements] = tables->ip;
		else
			elements--; /*one more time to make up for that*/
	}

	return;
}

typedef struct __entries_t {
	char *ip;
	struct __entries_t *next;
} pde_t;

pde_t *hashmap[HASH_SIZE];

unsigned int hash_virtual_ip(const char *ip) {
	unsigned hash = 5381; /*prime*/
	while (*ip)
		hash = ((hash << 5) + hash) + *ip++;

	return hash % HASH_SIZE;
}

bool hash_insert_virtual_ips(char *ip) {
	bool status = false;
	unsigned index = hash_virtual_ip(ip);

	pde_t *cur = hashmap[index];
	for (pde_t *k = cur; cur; k = k->next)
		if (strcmp(k->ip, ip) == 0)
			return status;

	status = true;
	pde_t *new = xmalloc(sizeof *new);
	new->ip = ip;
	new->next = hashmap[index];
	hashmap[index] = new;

	return status;
}
