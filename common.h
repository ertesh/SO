#ifndef _COMMON_
#define _COMMON_


#define MAX_BUF_SIZE 65536
// 64 Kb > 4Kb = minimal pipe capacity
#define GRAMMAR_LENGTH 20
#define PRODUCTION_LENGTH 20
#define MAX_LINE_LEN 20
#define FINISHING '!'
#define CLOSING '$'

struct grammar {
    int len;
    char prod[GRAMMAR_LENGTH][PRODUCTION_LENGTH];
};

//Checks whether a character is nonterminal (uppercase)
int is_upper(char c);

//Reads grammar from `filename` to `gram`
void read_grammar(struct grammar* gram, char* filename);

//Process the whole work that should be done
void work(struct grammar* g, char starting_char);

#endif
