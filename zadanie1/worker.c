/* Maciej Andrejczuk */

#include "err.h"
#include "common.h"

int main(int argc, char* argv[]) {
	struct grammar g;
    if (argc != 2) {
        fatal("Usage: %s <grammar file>\n", argv[0]);
    }
    read_grammar(&g, argv[1]);
    work(&g, 0);
   	return 0;
}

