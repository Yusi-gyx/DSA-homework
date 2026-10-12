#include <stdio.h>
#include <cmath>

void swap_ln(long long*& ln1, long long*& ln2)
{
    long long* tmp = ln1;
    ln1 = ln2;
    ln2 = tmp;
}

int main() {
    //处理输入
    long long len_a, len_b, T;
    scanf("%lld %lld %lld", &len_a, &len_b, &T);

    char a[len_a + 1];     //留出终止符的位置
    char b[len_b + 1];
    getchar();
    fgets(a, sizeof(a), stdin);
    getchar();
    fgets(b, sizeof(b), stdin);

    if(abs(len_a - len_b) > T)   //判断是否有提前剪枝的可能
    {
        printf("%d", -1);
        return 0;
    }

    //建立两个滚动的动态数组
    long long* ln1 = new long long[sizeof(a)]();
    long long* ln2 = new long long[sizeof(a)]();

    //DP
    //规定i,j表示两个字符串转换时的前缀长度
    //规定dp[i][j]表示将a[0..i-1]转换为b[0..j-1]所需要的操作数
    //dp[i][j] = min(dp[i-1][j] + 1, dp[i][j-1] + 1), a[i]!=b[j];
    //dp[i][j] = min(dp[i-1][j] + 1, dp[i][j-1] + 1, dp[i-1][j-1]), a[i]==b[j];
    //能够进行操作的两个字符串之间天然的存在限制：两个字符串的长度之差不能超过T；利用这一点进行剪枝
    for(int j = 0 ; j <= ((T > len_a)? len_a: T); j ++) ln1[j] = j;   //初始化ln1（第一行）
    // for(int j = 0; j <= ((T > len_a)? len_a: T); j ++) printf("%lld ", ln1[j]);
    // printf("\n");
    for(int i = 1; i < sizeof(b); i ++)
    {
        bool first = true;

        int left = (i > T) ? i - T : 0;
        int right = (T + i + 1 > sizeof(a))? sizeof(a): T + i + 1;
        for(int j = left; j < right; j ++)
        {
            //初始化第一项
            if(first)
            {
                if(j == 0)
                {
                    ln2[j] = i;
                    first = false;
                    continue;
                }
                if(b[i-1] == a[j-1]) ln2[j] = ln1[j-1];
                else ln2[j] = ln1[j] + 1;
                first = false;
                continue;
            }
            //处理每行最后一项
            if(j - i == T)
            {
                if(b[i-1] == a[j-1]) ln2[j] = (ln1[j -1] > ln2[j-1] + 1)? ln2[j-1] + 1: ln1[j-1];
                else ln2[j] = ln2[j-1] + 1;
                continue;
            }

            //递推
            if(b [i-1] != a [j-1]) 
            {
                ln2[j] = ((ln1[j] > ln2[j-1])? ln2[j-1] : ln1[j]) + 1;
            }
            else
            {
                long long min = ln1[j-1];
                if(min > ln2[j-1] + 1) min = ln2[j-1] + 1;
                if(min > ln1[j] + 1) min = ln1[j] + 1;
                ln2[j] = min;
            }
        }
        // for(int j = 0; j < sizeof(a); j ++)
        // {
        //     printf("%lld ", ln2[j]);
        // }
        // printf("\n");
        swap_ln(ln1, ln2);
    }

    if(ln1[sizeof(a)-1] > T) 
    {
        printf("%d", -1);
    }
    else 
    {
        printf("%lld", ln1[sizeof(a)-1]);
    }

    delete[] ln1;
    delete[] ln2;
    return 0;
}