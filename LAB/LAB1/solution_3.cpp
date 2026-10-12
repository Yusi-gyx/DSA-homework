#include <cstdio>

long long matrix[2002][2002];

int main() {
    int n, m, q;
    scanf("%d%d", &n, &m);
    for (int i = 1; i <= n; ++i) {
        for (int j = 1; j <= m; ++j) {
            scanf("%lld", &matrix[i][j]);
        }
    }
    //对数组进行预处理，使得m[i][j]表示从位置(i, j)到末尾的所有数之和
    for(int i = n - 1; i > 0; i --)
    {
        matrix[i][m] += matrix[i + 1][m];
    }
    for(int j = m - 1; j > 0; j --)
    {
        matrix[n][j] += matrix[n][j + 1];
    }
    for(int i = n - 1; i > 0; i --)
    {
        for(int j = m - 1; j > 0; j--)
        {
            matrix[i][j] += matrix[i + 1][j] + matrix[i][j + 1] - matrix[i + 1][j + 1];
        }
    }

    scanf("%d", &q);       //q个询问次数
    for(int i = 0; i < q; i ++)
    {
        int x, y, a, b;
        scanf("%d %d %d %d", &x, &y, &a, &b);
        printf("%lld\n", matrix[x][y] - matrix[x + a][y] - matrix[x][y + b] + matrix[x + a][y + b]);
    }
    
    return 0;
}
