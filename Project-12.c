#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef enum { FIRST, RANDOM, MEDIAN3 } PivotType;

static long long cmp_count = 0;

static void swap(int *a, int *b) { int t = *a; *a = *b; *b = t; }

static int select_pivot(int a[], int lo, int hi, PivotType type) {
    if (type == FIRST) return lo;
    if (type == RANDOM) return lo + rand() % (hi - lo + 1);

    int mid = lo + (hi - lo) / 2;
    int x = a[lo], y = a[mid], z = a[hi];
    cmp_count++;
    if (x < y) {
        cmp_count++;
        if (y < z) return mid;
        cmp_count++;
        if (x < z) return hi;
        return lo;
    } else {
        cmp_count++;
        if (x < z) return lo;
        cmp_count++;
        if (y < z) return hi;
        return mid;
    }
}

static int partition(int a[], int lo, int hi, PivotType type) {
    int p = select_pivot(a, lo, hi, type);
    swap(&a[lo], &a[p]);
    int pivot = a[lo];
    int i = lo;
    for (int j = lo + 1; j <= hi; j++) {
        cmp_count++;
        if (a[j] < pivot) swap(&a[++i], &a[j]);
    }
    swap(&a[lo], &a[i]);
    return i;
}

static void quick_recursive(int a[], int lo, int hi, PivotType type) {
    if (lo >= hi) return;
    int p = partition(a, lo, hi, type);
    quick_recursive(a, lo, p - 1, type);
    quick_recursive(a, p + 1, hi, type);
}

static void quick_iterative(int a[], int n, PivotType type) {
    int *stack = malloc(sizeof(int) * 2 * (n + 1));
    int top = -1;
    stack[++top] = 0;
    stack[++top] = n - 1;
    while (top >= 0) {
        int hi = stack[top--];
        int lo = stack[top--];
        if (lo >= hi) continue;
        int p = partition(a, lo, hi, type);
        stack[++top] = p + 1; stack[++top] = hi;
        stack[++top] = lo;    stack[++top] = p - 1;
    }
    free(stack);
}

static int is_sorted(const int a[], int n) {
    for (int i = 1; i < n; i++) if (a[i - 1] > a[i]) return 0;
    return 1;
}

static double now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}

int main(void) {
    int sizes[] = {1000, 2000, 4000, 8000, 16000, 20000};
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    const char *names[] = {"First ", "Random", "Median-3"};
    srand(12345);

    for (int s = 0; s < num_sizes; s++) {
        int n = sizes[s];
        int *orig = malloc(sizeof(int) * n);
        int *work = malloc(sizeof(int) * n);
        for (int i = 0; i < n; i++) orig[i] = rand() % 1000000;

        printf("Data Size : %d\n\n", n);
        for (int t = 0; t < 3; t++) {
            memcpy(work, orig, sizeof(int) * n);
            cmp_count = 0;
            quick_recursive(work, 0, n - 1, (PivotType)t);
            if (!is_sorted(work, n)) { printf("ERROR: not sorted\n"); return 1; }
            printf("Quick Sort (%-8s) : %lld comparisons\n", names[t], cmp_count);
        }
        printf("\n");
        free(orig); free(work);
    }

    int n = 10000, reps = 1000;
    int *orig = malloc(sizeof(int) * n);
    int *work = malloc(sizeof(int) * n);
    for (int i = 0; i < n; i++) orig[i] = rand() % 1000000;

    double t_rec = 0, t_it = 0;
    for (int r = 0; r < reps; r++) {
        memcpy(work, orig, sizeof(int) * n);
        double st = now_ms();
        quick_recursive(work, 0, n - 1, MEDIAN3);
        t_rec += now_ms() - st;
        if (!is_sorted(work, n)) { printf("ERROR\n"); return 1; }

        memcpy(work, orig, sizeof(int) * n);
        st = now_ms();
        quick_iterative(work, n, MEDIAN3);
        t_it += now_ms() - st;
        if (!is_sorted(work, n)) { printf("ERROR\n"); return 1; }
    }
    printf("Data Size : %d\nPivot     : Median-of-Three\n\n", n);
    printf("Recursive Quick Sort : %.4f ms\n", t_rec / reps);
    printf("Iterative Quick Sort : %.4f ms\n", t_it / reps);
    free(orig); free(work);
    return 0;
}
