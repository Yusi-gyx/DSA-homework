#include <stdio.h>
#include <vector>
using namespace std;

void merge(int lo, int mi, int hi, vector<int> &v) {
    vector<int> temp;                        //左半段 v[lo, mi) 的备份
    for(int j = lo; j < mi; ++j) {
        temp.push_back(v[j]);
    }
    int i = lo, j = 0, k = mi;               //i 回填下标，j 左半段游标，k 右半段游标
    while(j < (int)temp.size() && k < hi) {
        if(temp[j] <= v[k]) {
            v[i++] = temp[j++];
        }
        else {
            v[i++] = v[k++];
        }
    }
    while(j < (int)temp.size()) {            //左半段没填充完，右半段剩余元素本就在位
        v[i++] = temp[j++];
    }
}
void mergeSort(int lo, int hi, vector<int> &v) {
    if(hi - lo < 2) return;
    int mi = (lo + hi) / 2;
    mergeSort(lo, mi, v);
    mergeSort(mi, hi, v);
    merge(lo, mi, hi, v);
}

int search(vector<int> &v, long long int m, int lo, int hi) {    //二分查找：返回小于 m 的最大索引
    if(lo >= hi) return lo - 1;
    int mi = (lo + hi) / 2;
    if(v[mi] < m)  return search(v, m, mi + 1, hi);
    else return search(v, m, lo, mi);
}

int main() {
    //处理基础的输入
    long long int n;
    scanf("%lld", &n);
    vector<int> diagnosed(n + 1, 0);
    for(int i = 1; i <= n; ++i) {
        int x;
        scanf("%d", &x);
        diagnosed[i] = x;
    }

    //根据每天往前回溯的天数建立“Max Queue”
    vector<long long int> traces(n + 1, 0);
    for(int i = 1; i <= n; ++i) {
        scanf("%lld", &traces[i]);
    }
    vector<int> maxque_idx;     //记录着下标的最大队列
    vector<int> result;         //记录每天的观测病例最大值
    int head = 0;               //指向当前窗口最大值的序号的游标
    for(int i = 1; i <= n; ++i) {
        while(maxque_idx.size() > head && diagnosed[maxque_idx.back()] <= diagnosed[i - 1]) {
            maxque_idx.pop_back();
        }
        maxque_idx.push_back(i - 1);
        //判断当前最前面的下标是否已经超过了观测天数的下界
        long long int L = (i - traces[i] > 0)? i - traces[i]: 0;
        while(head < maxque_idx.size() && maxque_idx[head] < L) {     //更新界限内的最大值对应的下标
            head++;
        }
        result.push_back(diagnosed[maxque_idx[head]]);
    }
    //将result进行排序（归并）
    mergeSort(0, n, result);

    //利用Max Queue对T组阈值进行判断并计数
    int T;
    scanf("%d", &T);
    while(T--)
    {
        long long int p, q;
        scanf("%lld %lld", &p, &q);
        int lo = search(result, p, 0, n);
        int mi = search(result, q, 0, n) - lo;
        printf("%d %d", lo + 1, mi);
        if(T) printf("\n");
    }

    return 0;
}