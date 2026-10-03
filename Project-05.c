#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define DATA_COUNT 100
#define KEY_COUNT  50
#define MAX_VALUE  1000

typedef struct Node {
    int data;
    struct Node *left;
    struct Node *right;
} Node;

Node *insert(Node *root, int value, int *cmp) {
    Node *newNode = (Node *)malloc(sizeof(Node));
    newNode->data = value;
    newNode->left = newNode->right = NULL;

    if (root == NULL) return newNode;

    Node *cur = root;
    while (1) {
        (*cmp)++;
        if (value < cur->data) {
            if (cur->left == NULL) { cur->left = newNode; break; }
            cur = cur->left;
        } else {
            if (cur->right == NULL) { cur->right = newNode; break; }
            cur = cur->right;
        }
    }
    return root;
}

int sequential_search(const int *arr, int n, int key, int *cmp) {
    *cmp = 0;
    for (int i = 0; i < n; i++) {
        (*cmp)++;
        if (arr[i] == key) return 1;
    }
    return 0;
}

int bst_search(Node *root, int key, int *cmp) {
    *cmp = 0;
    Node *cur = root;
    while (cur != NULL) {
        (*cmp)++;
        if (key == cur->data) return 1;
        else if (key < cur->data) cur = cur->left;
        else cur = cur->right;
    }
    return 0;
}

int height(Node *root) {
    if (root == NULL) return 0;
    int l = height(root->left), r = height(root->right);
    return (l > r ? l : r) + 1;
}

void free_tree(Node *root) {
    if (root == NULL) return;
    free_tree(root->left);
    free_tree(root->right);
    free(root);
}

int main(void) {
    int arr[DATA_COUNT];
    int used[MAX_VALUE + 1] = {0};
    Node *root = NULL;
    int buildCmp = 0;

    srand((unsigned)time(NULL));

    for (int i = 0; i < DATA_COUNT; ) {
        int v = rand() % (MAX_VALUE + 1);
        if (used[v]) continue;
        used[v] = 1;
        arr[i++] = v;
        root = insert(root, v, &buildCmp);
    }

    printf("=== 생성된 %d개의 정수 ===\n", DATA_COUNT);
    for (int i = 0; i < DATA_COUNT; i++) {
        printf("%4d%s", arr[i], (i % 10 == 9) ? "\n" : " ");
    }
    printf("\nBST 생성 총 비교 횟수 : %d\n", buildCmp);
    printf("BST 높이              : %d\n\n", height(root));

    int keys[KEY_COUNT];
    for (int i = 0; i < KEY_COUNT; i++) keys[i] = rand() % (MAX_VALUE + 1);

    printf("=== 탐색 결과 ===\n");
    printf("%-4s %-8s %-8s %-12s %-12s\n", "No", "Key", "Result", "Sequential", "BST");

    int seqTotal = 0, bstTotal = 0, foundCount = 0;
    for (int i = 0; i < KEY_COUNT; i++) {
        int sc, bc;
        int sf = sequential_search(arr, DATA_COUNT, keys[i], &sc);
        int bf = bst_search(root, keys[i], &bc);
        (void)bf;
        seqTotal += sc;
        bstTotal += bc;
        if (sf) foundCount++;
        printf("%-4d %-8d %-8s %-12d %-12d\n",
               i + 1, keys[i], sf ? "Found" : "Not Found", sc, bc);
    }

    printf("\n=== 요약 ===\n");
    printf("Number of searches  : %d (성공 %d, 실패 %d)\n",
           KEY_COUNT, foundCount, KEY_COUNT - foundCount);

    printf("\nSequential Search\n");
    printf("Total comparisons   : %d\n", seqTotal);
    printf("Average comparisons : %.2f\n", (double)seqTotal / KEY_COUNT);

    printf("\nBST Search\n");
    printf("Total comparisons   : %d\n", bstTotal);
    printf("Average comparisons : %.2f\n", (double)bstTotal / KEY_COUNT);

    printf("\n=== 생성 비용 포함 비교 ===\n");
    printf("순차 탐색 총합            : %d\n", seqTotal);
    printf("BST 생성 + 탐색 총합      : %d (생성 %d + 탐색 %d)\n",
           buildCmp + bstTotal, buildCmp, bstTotal);

    free_tree(root);
    return 0;
}
