/* Maciej Andrejczuk */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "mesg.h"
#include "err.h"

void print_usage_info(char* prog_name) {
	fatal("Usage:\n %s + V W G\n%s - V W\n%s H W1 ... WM\n", 
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
	int nodes

	if ((id = msgget(MKEY, 0)) == -1)
		syserr("msgget");

	if (argc < 2) print_usage_info(argv[0]);
	
	char* command = argv[1];
	if (strcmp(command, "+") == 0) {
		if (argc != 5) print_usage_info(argv[0]);
		int V = atoi(argv[2]);
		int W = atoi(argv[
		add_edge(argv[2], argv[3], argv[4]);	
    }
	else if (strcmp(command, "-") == 0) {
		if (argc != 4) print_usage_info(argv[0]);
		remove_edge(argv[2], argv[3]);
	}
	else if (strcmp(command, "H") == 0) {
		if (argc < 3) print_usage_info(argv[0]);
		find_hamiltonian(argc - 2, argv + 2);
	}
	else print_usage_info(argv[0]);
}

