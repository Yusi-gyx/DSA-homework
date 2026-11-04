//data structure: 考虑到算法中不含动态操作，在输入时整棵树的结构是完整确定的，
//故使用数组记录每个节点的信息，每个节点只记录父亲、孩子的数量、长子和下一个兄弟的位置；
//每个人的师承路径用列表表示，若无则为空指针，若有即为指向父亲的指针
//algorithm: 考虑自底向上求每个节点子树和，建立每个节点的师承链
#include <stdio.h>

struct Node {
    int a;
    int chinum;
    Node* parent = nullptr;
    Node* firstChild = nullptr;
    Node* nextSibling = nullptr;
    Node* sifu = nullptr;   //记录节点的师傅，默认为空指针，否则为parent
    long long ChildTreeSum;    //记录以当前节点为根的子树和
};
Node tree[1000001];
int main() {
    int N;
    scanf("%d", &N);
    for(int i = 1; i <= N; ++i) {
        int chinum = 0;
        scanf("%d", &chinum);
        tree[i].chinum = chinum;
        int index1 = 0;   //表示当前孩子的序号
        int index2 = 0;   //记录前一个孩子的序号
        for(int j = 1; j <= chinum; ++j) {
            scanf("%d", &index1);
            if(j == 1) {  //记录长子
                tree[i].firstChild = &tree[index1];
            }
            tree[index1].parent = &tree[i];
            if(index2) tree[index2].nextSibling = &tree[index1];
            if(j != chinum) {
                index2= index1;
            } 
        }
    }
    for(int i = 1; i <= N; ++i) {
        int a = 0;
        scanf("%d", &a);
        tree[i].a = a;
    }
}