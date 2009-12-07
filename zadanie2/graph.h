#ifndef _GRAPH_
#define _GRAPH_

    typedef int** Graph;

    void prepare_graph(Graph* g, int n);
    void copy_subgraph(const Graph* source, Graph* dest, int n, int mask);
    int modify_graph(const Graph g, int x, int y, int w);
    int find_hamiltonian(const Graph g, int n, int mask);

#endif
