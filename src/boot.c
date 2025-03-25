/*  This showcases some of the stable APIs, The lib is chaotic and all that,
 *  this source helps u get an idea of what is going on
 *
 *  ENTRY, HOW DOES ALL THIS COME TOGETHER?
 *
 * Node A broadcasts & when it gets the available IPs within the range it asks
 * for permission to connect, We now have 2 Nodes connected and they can share
 * data accros the mesh network, Every Node can connect upto 10 other Nodes(do
 * the math 😃), Also A node can leave the Network. Incase this happens the
 * network reconfigures again. If a Node has a good radio then yeah that
 * is a plus.
 *
 *  TODO
 * This is compiled to an executable, an idea of how
 * the lib can be used, but it will be binded to kotlin so it can run on
 * Android
 */

#include <stdio.h>
#include "common/types.h"
#include "lib/net.h"
#include "lib/packet.h"

/* get everything up*/
struct __packet *packs = Nil;
MeshGraph *graph_nodes = Nil;

__CJLF_GENERICS ___boot() {
	/*setup the Network*/
	init_graph_network(graph_nodes, MAX_NODES);
	/*what do we have around us? I assume this initializes the discover
	 * array which is global! */
	dicsover_networks_around_me();
	/*global*/
	MeshNode *node;
	for (int n = 0; n < ARRAY_SIZE(discovered); n++) {
		node = create_node(discovered[n].ip, PORT /*port=*8000 ON */);

		if (node == Nil)
			fprintf(stdout, "Failed to create Node %s",
				discovered[n].ip);

		add_node(graph_nodes, node);
	}

	/* Early to start sharing? At this point in the startup, we have Node
	 * A or w/e pinged some networks around it and saved them to an array,
	 * remember we can only keep 10 Nodes & it has made a graph network
	 * around the IPs(real, sometimes it can pick loopbacks & the graph is
	 * algo we used to manage the mesh)*/
	(__CJLF_GENERICS) graph_nodes->num_nodes;
}

int main() {
	/*start server*/
	int server_fd = start_server_t();

	fd_set read_fds;
	int max_sd;

	while (1) {
		FD_ZERO(&read_fds);
		FD_SET(server_fd, &read_fds);
		/* Listen for user input */
		FD_SET(STDIN_FILENO, &read_fds);
		max_sd = server_fd > STDIN_FILENO ? server_fd : STDIN_FILENO;

		for (int i = 0; i < MAX_NODES; i++)
			if (clients[i] > 0) {
				FD_SET(clients[i], &read_fds);
				if (clients[i] > max_sd)
					max_sd = clients[i];
			}

		struct timeval timeout = {
			1,
			0,
		};
		int activity =
		    select(max_sd + 1, &read_fds, NULL, NULL, &timeout);

		if (activity < 0)
			continue;

		if (FD_ISSET(server_fd, &read_fds))
			handle_new_connection(server_fd);

		if (FD_ISSET(STDIN_FILENO, &read_fds))
			handle_user_input();

		for (int i = 0; i < MAX_NODES; i++)
			if (clients[i] > 0 && FD_ISSET(clients[i], &read_fds))
				handle_client_data(clients[i], i);

		sleep(1);
	}

	close(server_fd);
	return 0;
}
