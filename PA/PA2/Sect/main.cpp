//data structure: 考虑到算法中不含动态操作，在输入时整棵树的结构是完整确定的，
//故使用数组记录每个节点的信息，每个节点只记录父亲、孩子的数量、长子和下一个兄弟的位置；
//师承链用两个指针表示：sifu 指向链上的父亲（有路径时），heir 指向链上向下的继承人
//algorithm: 考虑自底向上求每个节点子树和，建立每个节点的师承链
#include <stdio.h>

struct Node {
    int a;
    int chinum;
    int index;
    Node* parent = nullptr;
    Node* firstChild = nullptr;
    Node* nextSibling = nullptr;
    Node* sifu = nullptr;   //继承路径上的父亲：自己接在父亲的链上，否则为空
    Node* heir = nullptr;   //继承人的正统弟子
    long long ChildTreeSum = 0;    //以当前节点为根的子树内力值之和

    void calcSum() {    //后序：每个节点只计算一次，结果缓存在 ChildTreeSum
        ChildTreeSum = a;
        for(Node* child = firstChild; child; child = child->nextSibling) {
            child->calcSum();
            ChildTreeSum += child->ChildTreeSum;
        }
    }
};
Node tree[1000001];
int result[1000001];
int stack[1000001];
int main() {
    int N;
    scanf("%d", &N);
    for(int i = 1; i <= N; ++i) {
        tree[i].index = i;
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

    tree[1].calcSum();    //后序遍历，一次性求出所有节点的子树和

    for(int i = 1; i <= N; ++i) {
        if(!tree[i].chinum) continue;
        Node* heir = tree[i].firstChild;    //只有一个弟子时，直接就是继承人
        if(tree[i].chinum > 1) {
            for(Node* child = heir->nextSibling; child; child = child->nextSibling) {
                if(child->ChildTreeSum > heir->ChildTreeSum ||
                   (child->ChildTreeSum == heir->ChildTreeSum && child->index < heir->index)) {
                    heir = child;
                }
            }
        }
        tree[i].heir = heir;
        heir->sifu = &tree[i];
    }

    for(int i = 1; i <= N; ++i) {
        if(tree[i].sifu) continue;    //从没有师傅的节点开始
        int top = 0;      //代表栈顶位置
        for(Node* h = &tree[i]; h; h = h->heir) {
            while(top > 0 && tree[stack[top]].a < h->a) top--;   //弹出栈中内力值比当前节点小的
            result[h->index] = (top > 0)? stack[top]: -1;
            stack[++top] = h->index;    //将当前节点入栈
        }
    }

    for(int i = 1; i <= N; ++i) {
        (i != N)? printf("%d\n", result[i]): printf("%d", result[i]);
    }
    return 0;
}