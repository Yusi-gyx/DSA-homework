#include <stdio.h>

void swapln(long long int*& ln1, long long int*& ln2) {
    long long int* temp = ln1;
    ln1 = ln2;
    ln2 = temp;
}

int main()
{
    //读取输入，将文件名存为char数组
    long long int len_a, len_b, T;
    scanf("%lld %lld %lld", &len_a, &len_b, &T);
    long long int limit = (len_a + len_b - T)/2;   //设置LCS下限，如果后面全部匹配仍无法达到则直接剪枝

    char a[len_a + 1];
    char b[len_b + 1];
    getchar();  //清理上一行残留的换行符
    fgets(a, sizeof(a), stdin);   //只读a长度的内容
    getchar();
    fgets(b, sizeof(b), stdin);

    //动态规划计算LCS长度
    long long int legh = sizeof(b);
    //准备使用滚动数组法
    //1.初始化
    long long int* ln1 = new long long int[legh]();
    long long int* ln2 = new long long int[legh]();
    //2.由底向上迭代
    for(int i = 0; i < sizeof(a); i ++)
    {
        for(int j = 1; j < legh; j ++)
        {
            if(a[i] == b[j - 1])
            {
                ln2[j] = ln1[j - 1] + 1;
            }
            else ln2[j] = (ln2[j - 1] > ln1[j])? ln2[j - 1]: ln1[j];
        }
        if (i != (sizeof(a) -1) && (ln2[legh - 1] + sizeof(a) - i - 1) < limit) 
        { // 剪枝操作
            printf("%d", -1);
            delete[] ln1;
            delete[] ln2;
            return 0;
        }
        //将ln2序列转换为ln1并重新初始化ln2
        swapln(ln1, ln2);
    }
    //3.根据len(LCS)计算需要时间
    long long int len_LCS = ln1[legh - 1];
    long long int T_need = len_a + len_b - 2*len_LCS;
    //4.比较
    if(T_need > T) printf("%d", -1);
    else printf("%lld", T_need);

    delete[] ln1;
    delete[] ln2;

    return 0;
}