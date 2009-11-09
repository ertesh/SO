/* Maciej Andrejczuk */

#ifndef _COMMON_
#define _COMMON_

    #define MAX_BUF_SIZE 65536
    /* 64 Kb > 4Kb = minimal pipe capacity */
    #define GRAMMAR_LENGTH 20
    #define PRODUCTION_LENGTH 20
    #define MAX_LINE_LEN 20
    #define FINISHING '!'
    #define CLOSING '$'

    struct grammar {
        int len;
        char prod[GRAMMAR_LENGTH][PRODUCTION_LENGTH];
    };

    int is_upper(char c);
    /* Checks whether a character is nonterminal (uppercase) */

    void read_grammar(struct grammar* gram, const char* filename);
    /* Reads grammar from `filename` to `gram`  *
     * MODIFIES: gram                           */

    void work(const struct grammar* g, char starting_char);
    /* Process the whole work that should be done */

#endif
