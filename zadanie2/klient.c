/* Maciej Andrejczuk */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mesg.h"
#include "err.h"

void print_usage_info(char* prog_name) {
	fatal("Correct usage:\n"
          "%s + V W G  \tadds an edge from V to W with cost G\n"
          "%s - V W  \tremoves an edge from V to W\n"
          "%s H W1 ... WM \tfinds the cheapest Hamiltonian cycle of "
          "subgraph W_i\n"
          "Moreover V, W, Wi >= 0  and G > 0.\n",
			prog_name, prog_name, prog_name);
}

int myatoi(char* text) {
    int ret = atoi(text);
    if (ret > 0) return ret;
    if (strcmp(text, "0") == 0) return 0;
    return -1;
}

int add_edge(int argc, char* argv[]) {
	int V, W, G;
    int id1, id2;
    MesgOrder message;
    MesgInfo reply;
    if (argc != 5)
        print_usage_info(argv[0]);
	if ((V = myatoi(argv[2])) < 0)
        print_usage_info(argv[0]);
	if ((W = myatoi(argv[3])) < 0)
        print_usage_info(argv[0]);
	if ((G = myatoi(argv[4])) <= 0)
        print_usage_info(argv[0]);

    message.type = getpid();
    message.command = ADD;
    message.len = 3;
    message.data[0] = V;
    message.data[1] = W;
    message.data[2] = G;
    id1 = init_queue(MKEY1, 0);
    send_order(id1, &message, 5 * sizeof(int));
    id2 = init_queue(MKEY2, 0);
    receive_info(id2, &reply, getpid());
    return reply.command;
}

int remove_edge(int argc, char* argv[]) {
	int V, W;
    int id1, id2;
    MesgOrder message;
    MesgInfo reply;
    if (argc != 4)
        print_usage_info(argv[0]);
	if ((V = myatoi(argv[2])) < 0)
        print_usage_info(argv[0]);
	if ((W = myatoi(argv[3])) < 0)
        print_usage_info(argv[0]);

    message.type = getpid();
    message.command = DEL;
    message.len = 2;
    message.data[0] = V;
    message.data[1] = W;
    id1 = init_queue(MKEY1, 0);
    send_order(id1, &message, 4 * sizeof(int));
    id2 = init_queue(MKEY2, 0);
    receive_info(id2, &reply, getpid());
    return reply.command;
}

int find_hamiltonian(int argc, char* argv[]) {
    int i, p;
    int id1, id2, ret;
    MesgOrder message;
    MesgInfo reply;
	if (argc < 1) print_usage_info(argv[0]);

    message.type = getpid();
    message.command = FIND;
    message.len = argc - 2;
    for (i = 0; i < message.len; i++) {
        if ((p = myatoi(argv[i + 2])) < 0)
            print_usage_info(argv[0]);
        message.data[i] = p;
    }

    id1 = init_queue(MKEY1, 0);
    send_order(id1, &message, argc * sizeof(int));
    id2 = init_queue(MKEY2, 0);
    receive_info(id2, &reply, getpid());
    printf("H %d\n", reply.command);
    if (reply.command == -1) ret = -1;
    if (reply.command == 0) ret = 1;
    if (reply.command > 0) ret = 0;
    return ret;
}

int main(int argc, char* argv[]) {

	char* command;

	if (argc < 2)
        print_usage_info(argv[0]);

    command = argv[1];
	if (strcmp(command, "+") == 0) {
		return add_edge(argc, argv);
    }
	if (strcmp(command, "-") == 0) {
		return remove_edge(argc, argv);
	}
	else if (strcmp(command, "H") == 0) {
		return find_hamiltonian(argc, argv);
	}
	print_usage_info(argv[0]);
    return -1;
}

