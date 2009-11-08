#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <err.h>

#define MAX_BUF_SIZE 65536
// 64 Kb > 4Kb = minimal pipe capacity
#define GRAMMAR_LENGTH 20
#define PRODUCTION_LENGTH 20
#define MAX_LINE_LEN 100
#define FINISHING '!'
#define CLOSING '$'

struct grammar {
    int len;
    char prod[GRAMMAR_LENGTH][PRODUCTION_LENGTH];
};

void read_message(char* text) {
    int length;
	read(0, &length, sizeof(int));
    read(0, text, length);
}

void write_message(char* text) {
    int length = strlen(text);
    write(1, &length, sizeof(int));
	write(1, text, length);
}

void debug(char *text) {
    int length = strlen(text);
    write(2, text, length);
}

void debug_int(int n) {
    if (n == 0) debug("0");
    char c[10];
    int k = 0;
    while (n > 0) {
        c[k] = '0' + (n % 10);
        n /= 10;
        k++;
    }
    c[k] = '\0';
    debug(c);
}

void read_grammar(struct grammar* g, char* filename) {
    char line[MAX_LINE_LEN + 3];
    int len, counter = 0;
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        fatal("Failed to open a file with grammar: %s\n", filename);
    }
    while (1) {
        fgets(line, MAX_LINE_LEN, file);
        len = strlen(line);
        if (len == 0) break;
        strncpy(line, g->prod[counter], MAX_LINE_LEN);
        counter++;
    }
    g->len = counter;
    fclose(file);
}

int is_upper(char c) {
    return (c >= 'A' && c <= 'Z');
}

int is_finishing(char* text) {
    int len, i;
    if (text[0] == FINISHING) return 1;
    len = strlen(text);
    for (i = 0; i < len; i++) {
        if (is_upper(text[i])) return 0;
    }
    return 1;
}

int is_closing(char* text) {
    return (text[0] == CLOSING);
}

void generate_word(struct grammar *g, char* text) {
    int i = 0, j = 0, k = 0;
    int len = strlen(text);
    int prod_len;
    // Searching first nonterminal
    while (k < len) {
        if (is_upper(text[k])) break;
        k++;
    }
    if (k == len) {
        fatal("Nonterminal not found as expected.\n");
    }
    // Searching first production for this nonterminal
    while (j < g->len) {
        if (g->prod[j][0] == text[k]) break;
        j++;
    }
    if (j == g->len) {
        fatal("No production for nonterminal %c found\n", text[k]);
    }
    // Inserting production into text
    prod_len = strlen(g->prod[j]) - 1;
    if (prod_len > 1) {
        for (i = len; i > k; i--) {
            text[i + prod_len - 1] = text[i];
        }
    }
    for (i = 1; i <= prod_len; i++) {
        text[k + i - 1] = g->prod[j][i];
    }
}

int work(struct grammar* g, int is_manager) {
    char text[MAX_BUF_SIZE];
    int length;
    while (1) {
        debug("Process ");
        debug_int(getpid());
        debug(": ");
        read_message(text);

        if (is_finishing(text)) {
            if (is_manager == 0) {
                write_message(text);
                debug("Work finished, waiting for close command.\n");
            } else {
                text[0] = CLOSING;
                write_message(text);
                debug("Work finished, sending close command.\n");
            }
            continue;
        }
        if (is_closing(text)) {
            if (is_manager == 0) {
                write_message(text);
            }
            debug("Close command recieved, exiting.\n");
            close(0);
            close(1);
            break;
        }
        generate_word(g, text);
        debug("Sending forward: ");
        debug(text);
        debug("\n");
        write_message(text);
    }
}

int main(int argc, char* argv[]) {
	struct grammar g;
    if (argc != 2) {
        fatal("Usage: %s <grammar file>\n", argv[0]);
    }
    read_grammar(&g, argv[1]);
    work(&g, 0);
   	return 0;
}

