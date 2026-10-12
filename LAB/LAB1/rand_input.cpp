#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

int main(int argc, char* argv[]) {
    // Usage: rand_input [n m q [random|full|point|row|column] [seed]]
    // With no arguments, keep the original small random test behavior.
    unsigned seed = static_cast<unsigned>(time(nullptr));
    if (argc == 6) seed = static_cast<unsigned>(strtoul(argv[5], nullptr, 10));
    srand(seed);

    int n, m, q;
    const char* shape = "random";
    if (argc == 1) {
        n = rand() % 91 + 10;
        m = rand() % 91 + 10;
        q = rand() % 11 + 10;
    } else if (argc == 5 || argc == 6) {
        n = atoi(argv[1]);
        m = atoi(argv[2]);
        q = atoi(argv[3]);
        shape = argv[4];
        if (n < 1 || n > 2000 || m < 1 || m > 2000 || q < 1 || q > 20000 ||
            (strcmp(shape, "random") && strcmp(shape, "full") &&
             strcmp(shape, "point") && strcmp(shape, "row") &&
             strcmp(shape, "column"))) {
            fprintf(stderr, "Invalid size or query shape\n");
            return 1;
        }
    } else {
        fprintf(stderr, "Usage: %s [n m q [random|full|point|row|column] [seed]]\n", argv[0]);
        return 1;
    }

    printf("%d %d\n", n, m);
    for (int i = 1; i <= n; ++i) {
        for (int j = 1; j <= m; ++j) {
            printf("%d%c", rand() % 1001, j == m ? '\n' : ' ');
        }
    }
    printf("%d\n", q);
    for (int i = 0; i < q; ++i) {
        int x = rand() % n + 1, y = rand() % m + 1;
        int a = rand() % (n - x + 1) + 1;
        int b = rand() % (m - y + 1) + 1;
        if (!strcmp(shape, "full")) x = y = 1, a = n, b = m;
        if (!strcmp(shape, "point")) a = b = 1;
        if (!strcmp(shape, "row")) y = 1, a = 1, b = m;
        if (!strcmp(shape, "column")) x = 1, a = n, b = 1;
        printf("%d %d %d %d\n", x, y, a, b);
    }
    return 0;
}
