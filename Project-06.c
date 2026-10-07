#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define GEN_COUNT 100
#define SEARCH_COUNT 50
#define MAX_VALUE 1000

typedef struct Node {
    int key;
    struct Node *left, *right;
    int height;
} Node;

static Node *new_node(int key) {
    Node *n = (Node *)malloc(sizeof(Node));
    n->key = key;
    n->left = n->right = NULL;
    n->height = 1;
    return n;
}

static int arr[GEN_COUNT];
static int arr_len = 0;

static int array_insert(int v, long *cmp) {
    for (int i = 0; i < arr_len; i++) {
        (*cmp)++;
        if (arr[i] == v) return 0;
    }
    arr[arr_len++] = v;
    return 1;
}

static int array_search(int v, long *cmp) {
    for (int i = 0; i < arr_len; i++) {
        (*cmp)++;
        if (arr[i] == v) return 1;
    }
    return 0;
}

static Node *bst_root = NULL;

static int bst_insert(int v, long *cmp) {
    Node **p = &bst_root;
    while (*p) {
        (*cmp)++;
        if (v == (*p)->key) return 0;
        p = (v < (*p)->key) ? &(*p)->left : &(*p)->right;
    }
    *p = new_node(v);
    return 1;
}

static int tree_search(Node *root, int v, long *cmp) {
    Node *cur = root;
    while (cur) {
        (*cmp)++;
        if (v == cur->key) return 1;
        cur = (v < cur->key) ? cur->left : cur->right;
    }
    return 0;
}

static int tree_height(Node *n) {
    if (!n) return 0;
    int l = tree_height(n->left), r = tree_height(n->right);
    return 1 + (l > r ? l : r);
}

static Node *avl_root = NULL;

static int h(Node *n) { return n ? n->height : 0; }
static int max(int a, int b) { return a > b ? a : b; }
static void update(Node *n) { n->height = 1 + max(h(n->left), h(n->right)); }
static int balance(Node *n) { return n ? h(n->left) - h(n->right) : 0; }

static Node *rotate_right(Node *y) {
    Node *x = y->left;
    y->left = x->right;
    x->right = y;
    update(y);
    update(x);
    return x;
}

static Node *rotate_left(Node *x) {
    Node *y = x->right;
    x->right = y->left;
    y->left = x;
    update(x);
    update(y);
    return y;
}

static Node *avl_insert_rec(Node *node, int v, long *cmp, int *inserted) {
    if (!node) {
        *inserted = 1;
        return new_node(v);
    }
    (*cmp)++;
    if (v == node->key) {
        *inserted = 0;
        return node;
    }
    if (v < node->key)
        node->left = avl_insert_rec(node->left, v, cmp, inserted);
    else
        node->right = avl_insert_rec(node->right, v, cmp, inserted);

    if (!*inserted) return node;

    update(node);
    int bf = balance(node);

    if (bf > 1) {
        if (balance(node->left) < 0)
            node->left = rotate_left(node->left);
        return rotate_right(node);
    }
    if (bf < -1) {
        if (balance(node->right) > 0)
            node->right = rotate_right(node->right);
        return rotate_left(node);
    }
    return node;
}

static int avl_insert(int v, long *cmp) {
    int inserted = 0;
    avl_root = avl_insert_rec(avl_root, v, cmp, &inserted);
    return inserted;
}

static void free_tree(Node *n) {
    if (!n) return;
    free_tree(n->left);
    free_tree(n->right);
    free(n);
}

int main(int argc, char *argv[]) {

    srand(argc > 1 ? (unsigned)atoi(argv[1]) : (unsigned)time(NULL));

    int gen[GEN_COUNT], keys[SEARCH_COUNT];
    long arr_cmp = 0, bst_cmp = 0, avl_cmp = 0;
    int stored = 0, duplicates = 0;

    for (int i = 0; i < GEN_COUNT; i++) {
        gen[i] = rand() % (MAX_VALUE + 1);
        int a = array_insert(gen[i], &arr_cmp);
        int b = bst_insert(gen[i], &bst_cmp);
        int c = avl_insert(gen[i], &avl_cmp);
        (void)b; (void)c;
        if (a) stored++; else duplicates++;
    }

    printf("Generated %d integers:\n", GEN_COUNT);
    for (int i = 0; i < GEN_COUNT; i++)
        printf("%4d%s", gen[i], (i % 10 == 9) ? "\n" : " ");

    printf("\nStored values     : %d\n", stored);
    printf("Duplicates skipped: %d\n", duplicates);

    printf("\nConstruction\n");
    printf("Array comparisons : %ld\n", arr_cmp);
    printf("BST comparisons   : %ld\n", bst_cmp);
    printf("AVL comparisons   : %ld\n", avl_cmp);

    printf("\nStructure\n");
    printf("Array length : %d\n", arr_len);
    printf("BST height   : %d\n", tree_height(bst_root));
    printf("AVL height   : %d\n", tree_height(avl_root));

    for (int i = 0; i < SEARCH_COUNT; i++)
        keys[i] = rand() % (MAX_VALUE + 1);

    printf("\nSearch keys (%d):\n", SEARCH_COUNT);
    for (int i = 0; i < SEARCH_COUNT; i++)
        printf("%4d%s", keys[i], (i % 10 == 9) ? "\n" : " ");
    printf("\n");

    long seq_total = 0, bst_total = 0, avl_total = 0;

    for (int i = 0; i < SEARCH_COUNT; i++) {
        long sc = 0, bc = 0, ac = 0;
        int sr = array_search(keys[i], &sc);
        int br = tree_search(bst_root, keys[i], &bc);
        int ar = tree_search(avl_root, keys[i], &ac);
        seq_total += sc;
        bst_total += bc;
        avl_total += ac;

        printf("Search Key : %d\n\n", keys[i]);
        printf("Sequential Search\nResult      : %s\nComparisons : %ld\n\n",
               sr ? "Found" : "Not Found", sc);
        printf("BST Search\nResult      : %s\nComparisons : %ld\n\n",
               br ? "Found" : "Not Found", bc);
        printf("AVL Search\nResult      : %s\nComparisons : %ld\n\n",
               ar ? "Found" : "Not Found", ac);
        printf("----------------------------------------\n\n");
    }

    printf("Searches : %d\n\n", SEARCH_COUNT);
    printf("Sequential Search\nTotal comparisons   : %ld\nAverage comparisons : %.2f\n\n",
           seq_total, (double)seq_total / SEARCH_COUNT);
    printf("BST Search\nTotal comparisons   : %ld\nAverage comparisons : %.2f\n\n",
           bst_total, (double)bst_total / SEARCH_COUNT);
    printf("AVL Search\nTotal comparisons   : %ld\nAverage comparisons : %.2f\n",
           avl_total, (double)avl_total / SEARCH_COUNT);

    free_tree(bst_root);
    free_tree(avl_root);
    return 0;
}
