#include <stdlib.h>
#include "graph.h"

#define INF 1000000

void prepare_graph(Graph* g, int n) {
    int i, j;
    (*g) = (int**) malloc(n * sizeof(int*));
    for (i = 0; i < n; i++) {
        (*g)[i] = (int*) malloc(n * sizeof(int));
        for (j = 0; j < n; j++)
            (*g)[i][j] = 0;
    }
}

void copy_subgraph(const Graph* source, Graph* dest, int n, long long mask) {
    int i, j;
    for (i = 0; i < n; i++) {
        if ((mask & (1LL << i)) == 0) continue;
        for (j = 0; j < n; j++) {
            if ((mask & (1LL << j)) == 0) continue;
            (*dest)[i][j] = (*source)[i][j];
        }
    }
}

int modify_graph(const Graph g, int x, int y, int w) {
    int ret = !((w > 0) ^ (g[x][y] > 0));
    g[x][y] = w;
    return ret;
}

int find(const Graph g, const int n, long long mask, int cur, const int first) {
    int i, ret = INF, pom;
    if (mask == 0) {
        if (cur == first) return 0;
        else return INF;
    }
    for (i = 0; i < n; i++) {
        if ((mask & (1LL << i)) == 0) continue;
        if (g[cur][i] == 0) continue;
        pom = g[cur][i] + find(g, n, mask ^ (1LL << i), i, first);
        if (ret > pom) ret = pom;
    }
    return ret;
}

int find_hamiltonian(const Graph g, const int n, long long mask) {
    int pocz = 0, ret;
    if (mask == 0) return -1;
    while ((mask & (1LL << pocz)) == 0) pocz++;
        ret = find(g, n, mask, pocz, pocz);
    if (ret == INF) return 0;
    return ret;
}

