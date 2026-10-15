#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define V          100
#define SPARSE_E   100
#define DENSE_E    4000
#define INS_OPS    10
#define DEL_OPS    10
#define CHK_OPS    10

typedef struct { int u, v; } Edge;

typedef struct { int m[V][V]; } AdjMatrix;

typedef struct Node { int vertex; struct Node *next; } Node;
typedef struct { Node *head[V]; size_t node_count; } AdjList;

typedef struct {
    Edge ins[INS_OPS];
    Edge del[DEL_OPS];
    Edge chk[CHK_OPS];
    int  adj_v;
} Workload;

typedef struct {
    size_t mem;
    long ins, del, chk, adj;
    int chk_result[CHK_OPS];
    int nbr[V], nbr_cnt;
} Result;

static int ref[V][V];

static void gen_graph(int E, Edge *edges)
{
    int k = 0;
    memset(ref, 0, sizeof(ref));
    while (k < E) {
        int u = rand() % V, v = rand() % V;
        if (u == v || ref[u][v]) continue;
        ref[u][v] = ref[v][u] = 1;
        edges[k].u = u; edges[k].v = v; k++;
    }
}

static void gen_workload(int E, const Edge *edges, Workload *w)
{
    static int used[V][V];
    int used_edge[DENSE_E] = {0};
    memset(used, 0, sizeof(used));

    for (int i = 0; i < INS_OPS; ) {
        int u = rand() % V, v = rand() % V;
        if (u == v || ref[u][v] || used[u][v]) continue;
        used[u][v] = used[v][u] = 1;
        w->ins[i].u = u; w->ins[i].v = v; i++;
    }
    for (int i = 0; i < DEL_OPS; ) {
        int k = rand() % E;
        if (used_edge[k]) continue;
        used_edge[k] = 1;
        w->del[i] = edges[k]; i++;
    }
    for (int i = 0; i < CHK_OPS; ) {
        int u = rand() % V, v = rand() % V;
        if (u == v) continue;
        w->chk[i].u = u; w->chk[i].v = v; i++;
    }
    w->adj_v = rand() % V;
}

static void m_build(AdjMatrix *g, const Edge *e, int E)
{
    memset(g, 0, sizeof(*g));
    for (int i = 0; i < E; i++) g->m[e[i].u][e[i].v] = g->m[e[i].v][e[i].u] = 1;
}

static long m_insert(AdjMatrix *g, int u, int v)
{
    long c = 1;
    if (g->m[u][v]) return c;
    g->m[u][v] = g->m[v][u] = 1;
    return c;
}

static long m_delete(AdjMatrix *g, int u, int v)
{
    long c = 1;
    if (!g->m[u][v]) return c;
    g->m[u][v] = g->m[v][u] = 0;
    return c;
}

static long m_connected(const AdjMatrix *g, int u, int v, int *res)
{
    *res = g->m[u][v];
    return 1;
}

static long m_neighbors(const AdjMatrix *g, int u, int *out, int *cnt)
{
    long c = 0;
    *cnt = 0;
    for (int j = 0; j < V; j++) {
        c++;
        if (g->m[u][j]) out[(*cnt)++] = j;
    }
    return c;
}

static void l_push(AdjList *g, int u, int v)
{
    Node *n = malloc(sizeof(Node));
    if (!n) { perror("malloc"); exit(1); }
    n->vertex = v; n->next = g->head[u];
    g->head[u] = n; g->node_count++;
}

static void l_build(AdjList *g, const Edge *e, int E)
{
    memset(g, 0, sizeof(*g));
    for (int i = 0; i < E; i++) { l_push(g, e[i].u, e[i].v); l_push(g, e[i].v, e[i].u); }
}

static void l_free(AdjList *g)
{
    for (int i = 0; i < V; i++) {
        Node *p = g->head[i];
        while (p) { Node *t = p->next; free(p); p = t; }
        g->head[i] = NULL;
    }
    g->node_count = 0;
}

static long l_insert(AdjList *g, int u, int v)
{
    long c = 0;
    for (Node *p = g->head[u]; p; p = p->next) {
        c++;
        if (p->vertex == v) return c;
    }
    l_push(g, u, v);
    l_push(g, v, u);
    return c;
}

static long l_remove_one(AdjList *g, int u, int v)
{
    long c = 0;
    Node *prev = NULL;
    for (Node *p = g->head[u]; p; prev = p, p = p->next) {
        c++;
        if (p->vertex == v) {
            if (prev) prev->next = p->next; else g->head[u] = p->next;
            free(p); g->node_count--;
            return c;
        }
    }
    return c;
}

static long l_delete(AdjList *g, int u, int v)
{
    return l_remove_one(g, u, v) + l_remove_one(g, v, u);
}

static long l_connected(const AdjList *g, int u, int v, int *res)
{
    long c = 0;
    *res = 0;
    for (Node *p = g->head[u]; p; p = p->next) {
        c++;
        if (p->vertex == v) { *res = 1; break; }
    }
    return c;
}

static long l_neighbors(const AdjList *g, int u, int *out, int *cnt)
{
    long c = 0;
    *cnt = 0;
    for (Node *p = g->head[u]; p; p = p->next) { c++; out[(*cnt)++] = p->vertex; }
    return c;
}

static int cmp_int(const void *a, const void *b) { return *(const int *)a - *(const int *)b; }

static void run_matrix(const Edge *e, int E, const Workload *w, Result *r)
{
    static AdjMatrix g;
    memset(r, 0, sizeof(*r));
    m_build(&g, e, E);
    r->mem = sizeof(AdjMatrix);
    for (int i = 0; i < INS_OPS; i++) r->ins += m_insert(&g, w->ins[i].u, w->ins[i].v);
    for (int i = 0; i < DEL_OPS; i++) r->del += m_delete(&g, w->del[i].u, w->del[i].v);
    for (int i = 0; i < CHK_OPS; i++)
        r->chk += m_connected(&g, w->chk[i].u, w->chk[i].v, &r->chk_result[i]);
    r->adj = m_neighbors(&g, w->adj_v, r->nbr, &r->nbr_cnt);
    qsort(r->nbr, r->nbr_cnt, sizeof(int), cmp_int);
}

static void run_list(const Edge *e, int E, const Workload *w, Result *r)
{
    AdjList g;
    memset(r, 0, sizeof(*r));
    l_build(&g, e, E);
    r->mem = sizeof(AdjList) + g.node_count * sizeof(Node);
    for (int i = 0; i < INS_OPS; i++) r->ins += l_insert(&g, w->ins[i].u, w->ins[i].v);
    for (int i = 0; i < DEL_OPS; i++) r->del += l_delete(&g, w->del[i].u, w->del[i].v);
    for (int i = 0; i < CHK_OPS; i++)
        r->chk += l_connected(&g, w->chk[i].u, w->chk[i].v, &r->chk_result[i]);
    r->adj = l_neighbors(&g, w->adj_v, r->nbr, &r->nbr_cnt);
    qsort(r->nbr, r->nbr_cnt, sizeof(int), cmp_int);
    l_free(&g);
}

static void print_case(int no, const char *name, const Result *r, int adj_v)
{
    printf("Case %d : %s\n", no, name);
    printf("Memory                         : %zu Bytes\n", r->mem);
    printf("Edge insertion comparisons     : %ld  (%d회 삽입 합계)\n", r->ins, INS_OPS);
    printf("Edge deletion comparisons      : %ld  (%d회 삭제 합계)\n", r->del, DEL_OPS);
    printf("Connectivity check comparisons : %ld  (%d쌍 확인 합계)\n", r->chk, CHK_OPS);
    printf("Adjacent vertices comparisons  : %ld  (정점 %d, 인접 정점 %d개)\n",
           r->adj, adj_v, r->nbr_cnt);
    printf("Adjacent vertices of %d         :", adj_v);
    for (int i = 0; i < r->nbr_cnt; i++) printf(" %d", r->nbr[i]);
    printf("\n\n");
}

static int same_result(const Result *a, const Result *b)
{
    if (a->nbr_cnt != b->nbr_cnt) return 0;
    if (memcmp(a->nbr, b->nbr, a->nbr_cnt * sizeof(int)) != 0) return 0;
    return memcmp(a->chk_result, b->chk_result, sizeof(a->chk_result)) == 0;
}

static void experiment(const char *label, int E, int first_case)
{
    static Edge edges[DENSE_E];
    Workload w;
    Result rm, rl;
    char name[64];

    gen_graph(E, edges);
    gen_workload(E, edges, &w);
    run_matrix(edges, E, &w, &rm);
    run_list(edges, E, &w, &rl);

    snprintf(name, sizeof(name), "%s Graph - Adjacency Matrix", label);
    print_case(first_case, name, &rm, w.adj_v);
    snprintf(name, sizeof(name), "%s Graph - Adjacency List", label);
    print_case(first_case + 1, name, &rl, w.adj_v);

    printf("[검증] %s: 두 표현의 연결 확인/인접 정점 결과 %s\n\n", label,
           same_result(&rm, &rl) ? "일치" : "불일치(오류!)");
}

int main(int argc, char *argv[])
{
    unsigned seed = (argc > 1) ? (unsigned)strtoul(argv[1], NULL, 10) : (unsigned)time(NULL);
    srand(seed);
    printf("seed = %u, sizeof(int)=%zu, sizeof(Node)=%zu, sizeof(void*)=%zu\n\n",
           seed, sizeof(int), sizeof(Node), sizeof(void *));

    experiment("Sparse", SPARSE_E, 1);
    experiment("Dense",  DENSE_E,  3);
    return 0;
}
