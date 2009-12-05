/* Maciej Andrejczuk */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "mesg.h"
#include "err.h"

void print_usage_info(char* prog_name) {
	fatal("Usage:\n"
          "%s + V W G  adds an edge from V to W with cost G\n"
          "%s - V W  removes an edge from V to W\n"
          "%s H W_1 ... W_M finds the cheapest Hamiltonian cycle of "
          "subgraph W_i\n"
          "Note that 0<=V,W,W_i<50 and 1<=G<100\n",
			prog_name, prog_name, prog_name);

void add_edge(char* V, char* W, char* G) {
	int id
}

void remove_edge(char *V, char* W) {

}

void find_hamiltonian(int m, char* nodes[]) {

}

int main(int argc, char* argv[]) {
	int V, W, G;
	int nodes[MAX_N]

	if ((id = msgget(MKEY, 0)) == -1)
		syserr("msgget");

	if (argc < 2)
        print_usage_info(argv[0]);

	char* command = argv[1];
	if (strcmp(command, "+") == 0) {
		if (argc != 5)
            print_usage_info(argv[0]);
		if ((V = atoi(argv[2])) == 0)
            print_usage_info(argv[0]);
		if ((W = atoi(argv[3])) == 0)
            print_usage_info(argv[0]);
		if ((G = atoi(argv[4])) == 0)
            print_usage_info(argv[0]);
		add_edge(V, W, G);
    }

	else if (strcmp(command, "-") == 0) {
		if (argc != 4)
            print_usage_info(argv[0]);
		if ((V = atoi(argv[2])) == 0)
            print_usage_info(argv[0]);
		if ((W = atoi(argv[3])) == 0)
            print_usage_info(argv[0]);
		remove_edge(V, W);
	}
	else if (strcmp(command, "H") == 0) {
		if (argc < 3) print_usage_info(argv[0]);
		find_hamiltonian(argc - 2, argv + 2);
	}
	else print_usage_info(argv[0]);
}

