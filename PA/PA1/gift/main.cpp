#include<stdio.h>
#include<math.h>

//枚举一半数组产生所有的情况
void subset(long long *sub, int arr[], int len)
{
    int index_arr = 0;
    int index_subl = 1;    //左哨兵
    int index_subr = 2;    //右哨兵
    sub[0] = 0;
    while(index_arr < len)
    {
        int bias = index_subr - index_subl;
        for(int i = index_subl; i < index_subr; i ++)
        {
            sub [i] = sub[i-bias] + arr[index_arr];
        }
        index_arr ++;
        index_subl = index_subr;
        index_subr *= 2;
    }
}

//对右侧数组进行快速排序
void quick_sort(long long arr[], int left, int right)
{
    if (left >= right)
        return;

    int i = left;
    int j = right;
    long long pivot = arr[(left + right) / 2];

    while (i <= j)
    {
        while (arr[i] < pivot)
            i++;
        while (arr[j] > pivot)
            j--;
        // 找到了两个放错位置的数，交换
        if (i <= j)
        {
            long long temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;

            i++;
            j--;
        }
    }

    // 排左半边
    quick_sort(arr, left, j);

    // 排右半边
    quick_sort(arr, i, right);
}

int binary_search(long long arr[], int lo, int hi, int maximum)
{
    if(hi - lo < 2) 
    {
        return lo + 1;
    }

    int mid = (hi + lo) / 2;
    if(arr [mid] > maximum)
    {
        return binary_search(arr, lo, mid, maximum);
    }
    else 
    {
        return binary_search(arr, mid, hi, maximum);
    }

    return 0;
}

int main() {
    int n, P;
    scanf("%d %d", &n, &P);
    long long min_need = 0;    //处理最小情况
    int delta[n];        //记录两个礼物价格之间的差值
    for(int i = 0; i < n; i ++) 
    {
        int c1, c2;
        scanf("%d %d", &c1, &c2);
        min_need += (c1 > c2)? c2: c1;
        delta[i] = abs(c1 - c2);
    }

    //如果最小情况都大于n，那么一定为0
    if(min_need > P)
    {
        printf("%d", 0);
        return 0;
    }
    //如果最大情况也小于P，那么一定为2^n
    long long sum = min_need;
    for(int i = 0; i < n; i ++) sum += delta[i];
    if(sum <= P)
    {
        long long ans = 1LL << n;
        printf("%lld", ans);
        return 0;
    }

    //采用折半处理+二分查找的方法
    int P2 = P - min_need;      //确定delta中子集和的新上限
    int mid = n / 2;
    //将原来的数组分为左右两个
    int Larr[mid];
    int Rarr[(n % 2)? mid + 1: mid];
    int len = sizeof(Rarr) / sizeof(Rarr[0]);
    for(int i = 0; i < mid; i ++) Larr[i] = delta[i];
    for(int i = mid; i < n; i ++) Rarr[i - mid] = delta[i];
    //左右数组分别枚举子集
    long long* L = new long long[1 << mid];
    long long* R = new long long[1 << len];
    subset(L, Larr, mid);
    subset(R, Rarr, len);

    //将右边的数组进行排序，L + R <= P2，即R <= P2 - L，对L中每一个元素进行判断，对R排序后的元素进行二分查找
    //对R的排序
    quick_sort(R, 0, (1 << len)-1);
    //对R中的元素进行二分查找
    long long cnt = 0;
    for(int i = 0; i < (1 << mid); i ++)
    {
        if(R[0] > P2 - L[i]) continue;
        cnt += binary_search(R, 0, 1 << len, P2 - L[i]);
    }
    printf("%lld", cnt);
    delete[] L;
    delete[] R;
    return 0;
}
