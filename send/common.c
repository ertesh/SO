/* Maciej Andrejczuk */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "err.h"
#include "common.h"

void read_message(char* text) {
    int length;
	read(0, &length, sizeof(int));
    read(0, text, length);
}

void write_message(const char* text) {
    int length = strlen(text);
    write(1, &length, sizeof(int));
	write(1, text, length);
}

void debug(const char *text) {
    int length = strlen(text);
    write(2, text, length);
}

void debug_int(int n) {
    char c[12];
    c[10] = '\0';
    int k = 9;
    if (n == 0) debug("0");
    while (n > 0 || k == 9) {
        c[k] = '0' + (n % 10);
        n /= 10;
        k--;
    }
    debug(c + k + 1);
}

void debug_process_info() {
    debug("Process ");
    debug_int(getpid());
    debug(": ");
}

void send_forward(char* text) {
    write_message(text);
    debug_process_info();
    debug("Sending forward: ");
    debug(text);
    debug("\n");
}

int is_upper(char c) {
    return (c >= 'A' && c <= 'Z');
}

int is_finishing(const char* text) {
    int len, i;
    if (text[0] == FINISHING) return 1;
    if (text[0] == CLOSING) return 0;
    len = strlen(text);
    for (i = 0; i < len; i++) {
        if (is_upper(text[i])) return 0;
    }
    return 1;
}

int is_closing(const char* text) {
    return (text[0] == CLOSING);
}


/* ------- Essential functions -------- */

void read_grammar(struct grammar* g, const char* filename) {
    char line[MAX_LINE_LEN + 3];
    int counter = 0;
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        fatal("Failed to open a file with grammar: %s\n", filename);
    }
    while (1) {
        fgets(line, MAX_LINE_LEN, file);
        if (strlen(line) < 2) break;
        strncpy(g->prod[counter], line, MAX_LINE_LEN);
        counter++;
    }
    g->len = counter;
    fclose(file);
}

void generate_word(const struct grammar *g, char* text) {
    int i = 0, j = 0, k = 0;
    int len = strlen(text);
    int prod_len;
    /* Searching for the first nonterminal */
    while (k < len) {
        if (is_upper(text[k])) break;
        k++;
    }
    if (k == len) {
        fatal("Nonterminal not found as expected.\n");
    }
    /* Searching for the first production of this nonterminal */
    while (j < g->len) {
        if (g->prod[j][0] == text[k]) break;
        j++;
    }
    if (j == g->len) {
        fatal("No production for nonterminal %c found\n", text[k]);
    }
    /* Inserting production into text */
    prod_len = strlen(g->prod[j]) - 2;  /* without first and last char */
    if (prod_len > 1) {
        for (i = len; i > k; i--) {
            text[i + prod_len - 1] = text[i];
        }
    }
    for (i = 1; i <= prod_len; i++) {
        text[k + i - 1] = g->prod[j][i];
    }
}

void work(const struct grammar* g, char starting_char) {
    char text[MAX_BUF_SIZE];
    if (starting_char != 0) {  /* manager's process */
        text[0] = starting_char;
        text[1] = '\0';
        send_forward(text);
    }
    while (1) {
        read_message(text);
        if (is_closing(text)) {
            if (starting_char == 0)
                write_message(text);
            debug_process_info();
            debug("Close command recieved, exiting.\n");
            if (close(0) == -1)
                syserr("Error in close(0)\n");
            if (close(1) == -1)
                syserr("Error in close(1)\n");
            break;
        }
        if (is_finishing(text)) {
            if (starting_char == 0) {
                write_message(text);
                debug_process_info();
                debug("Work finished, waiting for close command.\n");
            } else {
                text[0] = CLOSING;
                write_message(text);
                debug_process_info();
                debug("Work finished, sending close command.\n");
            }
            continue;
        }
        generate_word(g, text);
        send_forward(text);
    }
}

