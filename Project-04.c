#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_INPUT 1024
#define STACK_MAX 256

typedef struct Node {
    char data;
    struct Node *left;
    struct Node *right;
} Node;

Node *make_node(char c) {
    Node *n = (Node *)malloc(sizeof(Node));
    n->data = c;
    n->left = NULL;
    n->right = NULL;
    return n;
}

int g_parse_error = 0;

/*
 * 괄호 표기법: DATA(child)(child)
 * - 자식이 없으면 괄호를 아예 쓰지 않음 (빈 괄호 없음)
 * - 괄호가 하나만 있으면 왼쪽 자식으로 간주
 * - 괄호가 두 개 있으면 순서대로 왼쪽, 오른쪽 자식
 */
Node *parse_node(const char **p) {
    while (**p == ' ') (*p)++;

    if (!isupper((unsigned char)**p)) {
        g_parse_error = 1;
        return NULL;
    }
    char data = **p;
    (*p)++;
    Node *node = make_node(data);

    Node *children[2];
    int child_count = 0;

    while (**p == '(') {
        (*p)++;
        Node *child = parse_node(p);
        if (g_parse_error) return node;

        while (**p == ' ') (*p)++;
        if (**p != ')') {
            g_parse_error = 1;
            return node;
        }
        (*p)++;

        if (child_count >= 2) {
            g_parse_error = 1;
            return node;
        }
        children[child_count++] = child;

        while (**p == ' ') (*p)++;
    }

    if (child_count == 1) {
        node->left = children[0];
    } else if (child_count == 2) {
        node->left = children[0];
        node->right = children[1];
    }
    return node;
}

Node *parse_tree(const char *s) {
    g_parse_error = 0;
    const char *p = s;
    while (*p == ' ') p++;
    if (*p == '\0') {
        g_parse_error = 1;
        return NULL;
    }
    Node *root = parse_node(&p);
    while (*p == ' ') p++;
    if (*p != '\0') {
        g_parse_error = 1;
    }
    if (g_parse_error) return root;
    return root;
}

/* ---------------- 스택 (반복적 순회용) ---------------- */

typedef struct {
    Node *data[STACK_MAX];
    int top;
} Stack;

void stack_init(Stack *s) { s->top = -1; }
int stack_empty(Stack *s) { return s->top == -1; }
void stack_push(Stack *s, Node *n) { s->data[++(s->top)] = n; }
Node *stack_pop(Stack *s) { return s->data[(s->top)--]; }

/* ---------------- 반복적 순회 (재귀 사용 안 함) ---------------- */

void preorder(Node *root) {
    if (root == NULL) {
        printf("(빈 트리)\n");
        return;
    }
    Stack s;
    stack_init(&s);
    stack_push(&s, root);
    int first = 1;

    while (!stack_empty(&s)) {
        Node *cur = stack_pop(&s);
        if (!first) printf(" ");
        printf("%c", cur->data);
        first = 0;

        if (cur->right != NULL) stack_push(&s, cur->right);
        if (cur->left != NULL) stack_push(&s, cur->left);
    }
    printf("\n");
}

void inorder(Node *root) {
    if (root == NULL) {
        printf("(빈 트리)\n");
        return;
    }
    Stack s;
    stack_init(&s);
    Node *cur = root;
    int first = 1;

    while (cur != NULL || !stack_empty(&s)) {
        while (cur != NULL) {
            stack_push(&s, cur);
            cur = cur->left;
        }
        cur = stack_pop(&s);
        if (!first) printf(" ");
        printf("%c", cur->data);
        first = 0;
        cur = cur->right;
    }
    printf("\n");
}

void postorder(Node *root) {
    if (root == NULL) {
        printf("(빈 트리)\n");
        return;
    }
    Stack s1, s2;
    stack_init(&s1);
    stack_init(&s2);
    stack_push(&s1, root);

    while (!stack_empty(&s1)) {
        Node *cur = stack_pop(&s1);
        stack_push(&s2, cur);
        if (cur->left != NULL) stack_push(&s1, cur->left);
        if (cur->right != NULL) stack_push(&s1, cur->right);
    }

    int first = 1;
    while (!stack_empty(&s2)) {
        Node *cur = stack_pop(&s2);
        if (!first) printf(" ");
        printf("%c", cur->data);
        first = 0;
    }
    printf("\n");
}

/* ---------------- 트리 구조 출력 (참고용) ---------------- */

void print_structure(Node *n, int depth) {
    if (n == NULL) return;
    if (depth == 0) {
        printf("%c\n", n->data);
    } else {
        for (int i = 0; i < depth - 1; i++) printf("    ");
        printf("+---%c\n", n->data);
    }
    print_structure(n->left, depth + 1);
    print_structure(n->right, depth + 1);
}

/* ---------------- 메인 ---------------- */

int main(void) {
    char line[MAX_INPUT];

    printf("괄호 표기법으로 이진트리를 입력하세요 (예: A(B(D)(E))(C(F))):\n");
    if (fgets(line, sizeof(line), stdin) == NULL) {
        printf("입력이 없습니다.\n");
        return 0;
    }
    line[strcspn(line, "\r\n")] = '\0';

    Node *root = parse_tree(line);
    if (g_parse_error) {
        printf("오류: 괄호 표현이 올바르지 않습니다.\n");
        return 0;
    }

    printf("\n[입력된 이진트리 구조]\n");
    print_structure(root, 0);

    printf("\n[전위 순회 결과]\nPreorder  : ");
    preorder(root);

    printf("[중위 순회 결과]\nInorder   : ");
    inorder(root);

    printf("[후위 순회 결과]\nPostorder : ");
    postorder(root);

    return 0;
}
