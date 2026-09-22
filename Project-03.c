#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE 256
#define MAX_TOKENS 8
#define MAX_PATH_DEPTH 64

typedef struct Node {
    char data;
    struct Node *left;
    struct Node *right;
} Node;

typedef struct {
    Node *root;
    int capacity;
    int count;
} BTree;

BTree *create_btree(int size) {
    BTree *t = (BTree *)malloc(sizeof(BTree));
    t->root = NULL;
    t->capacity = size;
    t->count = 0;
    return t;
}

static Node *make_node(char value) {
    Node *n = (Node *)malloc(sizeof(Node));
    n->data = value;
    n->left = NULL;
    n->right = NULL;
    return n;
}

static void free_subtree(Node *n) {
    if (n == NULL) return;
    free_subtree(n->left);
    free_subtree(n->right);
    free(n);
}

void destroy_btree(BTree *t) {
    if (t == NULL) return;
    free_subtree(t->root);
    t->root = NULL;
    t->count = 0;
    free(t);
}

static Node *find_by_path(BTree *t, char *path, int path_len) {
    if (t->root == NULL) return NULL;
    if (path_len == 0) return NULL;
    if (t->root->data != path[0]) return NULL;

    Node *cur = t->root;
    for (int i = 1; i < path_len; i++) {
        Node *next = NULL;
        if (cur->left && cur->left->data == path[i]) next = cur->left;
        else if (cur->right && cur->right->data == path[i]) next = cur->right;

        if (next == NULL) return NULL;
        cur = next;
    }
    return cur;
}

static Node *find_parent_of(BTree *t, char *path, int path_len, int *child_is_left) {
    if (path_len < 2) return NULL;
    Node *parent = find_by_path(t, path, path_len - 1);
    if (parent == NULL) return NULL;

    if (parent->left && parent->left->data == path[path_len - 1]) {
        if (child_is_left) *child_is_left = 1;
    } else if (parent->right && parent->right->data == path[path_len - 1]) {
        if (child_is_left) *child_is_left = 0;
    } else {
        return NULL;
    }
    return parent;
}

int insert_root(BTree *t, char value) {
    if (t->root != NULL) return 0;
    t->root = make_node(value);
    t->count++;
    return 1;
}

int insert_child(BTree *t, Node *parent, char child_dir, char value) {
    if (parent == NULL) return 0;
    if (parent->left != NULL && parent->right != NULL) return 0;
    if (child_dir != 'L' && child_dir != 'R') return 0;

    if (child_dir == 'L') {
        if (parent->left != NULL) return 0;
        if (parent->right != NULL && parent->right->data == value) return 0;
        parent->left = make_node(value);
    } else {
        if (parent->right != NULL) return 0;
        if (parent->left != NULL && parent->left->data == value) return 0;
        parent->right = make_node(value);
    }
    t->count++;
    return 1;
}

int delete_node(BTree *t, Node *parent, int child_is_left, Node *leaf, int is_root) {
    if (leaf->left != NULL || leaf->right != NULL) return 0;

    if (is_root) {
        free(t->root);
        t->root = NULL;
    } else {
        if (child_is_left) parent->left = NULL;
        else parent->right = NULL;
        free(leaf);
    }
    t->count--;
    return 1;
}

int update_value(Node *parent, int has_parent, Node *target, char value) {
    if (has_parent) {
        Node *sibling = NULL;
        if (parent->left == target) sibling = parent->right;
        else sibling = parent->left;
        if (sibling != NULL && sibling->data == value) return 0;
    }
    target->data = value;
    return 1;
}

void read_child(Node *parent, char *out, int *has_children) {
    *has_children = 0;
    out[0] = '\0';
    if (parent->left == NULL && parent->right == NULL) return;

    *has_children = 1;
    if (parent->left && parent->right) {
        sprintf(out, "%c(L), %c(R)", parent->left->data, parent->right->data);
    } else if (parent->left) {
        sprintf(out, "%c(L)", parent->left->data);
    } else {
        sprintf(out, "%c(R)", parent->right->data);
    }
}

static void print_recursive(Node *n, int depth) {
    if (n == NULL) return;
    if (depth == 0) {
        printf("%c\n", n->data);
    } else {
        for (int i = 0; i < depth - 1; i++) printf("    ");
        printf("+---%c\n", n->data);
    }
    print_recursive(n->left, depth + 1);
    print_recursive(n->right, depth + 1);
}

void print_btree(BTree *t) {
    if (t->root == NULL) {
        printf("트리가 비어 있습니다.\n");
        return;
    }
    print_recursive(t->root, 0);
}

static int parse_path(const char *s, char *path) {
    if (s[0] != '/') return -1;
    int len = 0;
    int i = 1;
    while (s[i] != '\0') {
        if (s[i] == '/') {
            i++;
            continue;
        }
        if (!isupper((unsigned char)s[i])) return -1;

        if (s[i + 1] != '\0' && s[i + 1] != '/') return -1;
        if (len >= MAX_PATH_DEPTH) return -1;
        path[len++] = s[i];
        i++;
    }
    if (len == 0) return -1;
    return len;
}

static int is_cmd(const char *tok, const char full[], char short_c) {
    if (tok[1] == '\0' && toupper((unsigned char)tok[0]) == short_c) return 1;

    size_t n = strlen(tok);
    if (n != strlen(full)) return 0;
    for (size_t i = 0; i < n; i++) {
        if (toupper((unsigned char)tok[i]) != toupper((unsigned char)full[i])) return 0;
    }
    return 1;
}

static void do_insert(BTree *t, int argc, char **argv) {
    if (argc == 2) {

        if (strcmp(argv[0], "/") != 0 || strlen(argv[1]) != 1 || !isupper((unsigned char)argv[1][0])) {
            printf("오류: 잘못된 Insert 형식입니다.\n");
            return;
        }
        if (t->root != NULL) {
            printf("오류: 이미 루트 노드가 존재합니다.\n");
            return;
        }
        insert_root(t, argv[1][0]);
        printf("루트 노드 %c가 생성되었습니다.\n", argv[1][0]);
        return;
    }

    if (argc != 3) {
        printf("오류: Insert 명령의 인자 개수가 올바르지 않습니다.\n");
        return;
    }

    char path[MAX_PATH_DEPTH];
    int len = parse_path(argv[0], path);
    if (len < 0) {
        printf("오류: 경로 형식이 올바르지 않습니다.\n");
        return;
    }

    char dir = toupper((unsigned char)argv[1][0]);
    if ((dir != 'L' && dir != 'R') || argv[1][1] != '\0') {
        printf("오류: child는 L 또는 R이어야 합니다.\n");
        return;
    }

    if (strlen(argv[2]) != 1 || !isupper((unsigned char)argv[2][0])) {
        printf("오류: new-data는 영문 대문자 한 글자여야 합니다.\n");
        return;
    }

    Node *parent = find_by_path(t, path, len);
    if (parent == NULL) {
        printf("오류: parent-node가 존재하지 않습니다.\n");
        return;
    }
    if (parent->left != NULL && parent->right != NULL) {
        printf("오류: parent-node가 단말 노드가 아닙니다.\n");
        return;
    }

    if (t->capacity > 0 && t->count >= t->capacity) {
        printf("오류: 트리 용량을 초과했습니다.\n");
        return;
    }

    if (!insert_child(t, parent, dir, argv[2][0])) {
        printf("오류: 동일한 부모의 다른 자식이 이미 같은 데이터를 가지고 있습니다.\n");
        return;
    }
    printf("노드 %c가 추가되었습니다.\n", argv[2][0]);
}

static void do_delete(BTree *t, int argc, char **argv) {
    if (argc != 1) {
        printf("오류: Delete 명령의 인자 개수가 올바르지 않습니다.\n");
        return;
    }
    char path[MAX_PATH_DEPTH];
    int len = parse_path(argv[0], path);
    if (len < 0) {
        printf("오류: 경로 형식이 올바르지 않습니다.\n");
        return;
    }

    Node *leaf = find_by_path(t, path, len);
    if (leaf == NULL) {
        printf("오류: 해당 노드가 존재하지 않습니다.\n");
        return;
    }
    if (leaf->left != NULL || leaf->right != NULL) {
        printf("오류: 단말 노드가 아니므로 삭제할 수 없습니다.\n");
        return;
    }

    if (len == 1) {
        delete_node(t, NULL, 0, leaf, 1);
    } else {
        int child_is_left;
        Node *parent = find_parent_of(t, path, len, &child_is_left);
        delete_node(t, parent, child_is_left, leaf, 0);
    }
    printf("노드가 삭제되었습니다.\n");
}

static void do_update(BTree *t, int argc, char **argv) {
    if (argc != 2) {
        printf("오류: Update 명령의 인자 개수가 올바르지 않습니다.\n");
        return;
    }
    char path[MAX_PATH_DEPTH];
    int len = parse_path(argv[0], path);
    if (len < 0) {
        printf("오류: 경로 형식이 올바르지 않습니다.\n");
        return;
    }
    if (strlen(argv[1]) != 1 || !isupper((unsigned char)argv[1][0])) {
        printf("오류: new-data는 영문 대문자 한 글자여야 합니다.\n");
        return;
    }

    Node *target = find_by_path(t, path, len);
    if (target == NULL) {
        printf("오류: 해당 노드가 존재하지 않습니다.\n");
        return;
    }

    if (len == 1) {
        target->data = argv[1][0];
    } else {
        int child_is_left;
        Node *parent = find_parent_of(t, path, len, &child_is_left);
        if (!update_value(parent, 1, target, argv[1][0])) {
            printf("오류: 변경 결과 형제 노드와 데이터가 중복됩니다.\n");
            return;
        }
    }
    printf("노드 데이터가 %c로 변경되었습니다.\n", argv[1][0]);
}

static void do_read(BTree *t, int argc, char **argv) {
    if (argc != 1) {
        printf("오류: Read 명령의 인자 개수가 올바르지 않습니다.\n");
        return;
    }
    char path[MAX_PATH_DEPTH];
    int len = parse_path(argv[0], path);
    if (len < 0) {
        printf("오류: 경로 형식이 올바르지 않습니다.\n");
        return;
    }
    Node *parent = find_by_path(t, path, len);
    if (parent == NULL) {
        printf("오류: 해당 노드가 존재하지 않습니다.\n");
        return;
    }

    char buf[64];
    int has_children;
    read_child(parent, buf, &has_children);
    if (!has_children) {
        printf("단말 노드입니다. 자식이 없습니다.\n");
    } else {
        printf("%s\n", buf);
    }
}

static void do_print(BTree *t, int argc) {
    if (argc != 0) {
        printf("오류: Print 명령은 인자를 받지 않습니다.\n");
        return;
    }
    print_btree(t);
}

static int tokenize(char *line, char **argv) {
    int argc = 0;
    char *tok = strtok(line, " \t\r\n");
    while (tok != NULL && argc < MAX_TOKENS) {
        argv[argc++] = tok;
        tok = strtok(NULL, " \t\r\n");
    }
    return argc;
}

int main(void) {
    BTree *tree = create_btree(1000);
    char line[MAX_LINE];

    printf("트리 조작 프로그램을 시작합니다. (종료: Ctrl+D 또는 Quit)\n");

    while (fgets(line, sizeof(line), stdin) != NULL) {
        char *argv[MAX_TOKENS];
        int argc = tokenize(line, argv);
        if (argc == 0) continue;

        char *cmd = argv[0];
        int rest_argc = argc - 1;
        char **rest_argv = argv + 1;

        if (is_cmd(cmd, "Insert", 'I')) {
            do_insert(tree, rest_argc, rest_argv);
        } else if (is_cmd(cmd, "Delete", 'D')) {
            do_delete(tree, rest_argc, rest_argv);
        } else if (is_cmd(cmd, "Update", 'U')) {
            do_update(tree, rest_argc, rest_argv);
        } else if (is_cmd(cmd, "Read", 'R')) {
            do_read(tree, rest_argc, rest_argv);
        } else if (is_cmd(cmd, "Print", 'P')) {
            do_print(tree, rest_argc);
        } else if (is_cmd(cmd, "Quit", 'Q')) {
            break;
        } else {
            printf("오류: 알 수 없는 명령어입니다.\n");
        }
    }

    destroy_btree(tree);
    return 0;
}
