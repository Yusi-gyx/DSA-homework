#include <stack>
#include <stdio.h>
#include <ctype.h>
using namespace std;

const int M = 1000000007;
const char* notation = "+-*^()$";
const char pri[7][7] = {
    /*           |------- 当 前 运 算 符 --------|  */
    /*           +    -    *    ^     (    )    $ */
    /*--  + */  '>', '>', '<', '<',  '<', '>', '>',
    /* 栈 - */  '>', '>', '<', '<',  '<', '>', '>',
    /* 顶 * */  '>', '>', '>', '<',  '<', '>', '>', 
    /* 运 ^ */  '>', '>', '>', '>',  '<', '>', '>',
    /* 算 ! */  '>', '>', '>', '>',  'x', '>', '>',
    /* 符 ( */  '<', '<', '<', '<',  '<', '~', 'x',
    /*--  $ */  '<', '<', '<', '<',  '<', 'x', '~'
};
struct unit {
    long long coef;   //每一项的系数
    int exponent;     //每一项的指数

    unit(long long c, int e): coef(c), exponent(e) {}
    void operator*=(unit y) {
        //左结合乘法
        coef *= y.coef;
        exponent += y.exponent;
    }
};

int normalize(long long c) {    //定义取模函数
    return (c % M + M) % M;
}
int get_index(char s) {
    for(int i = 0; i < 7; i ++)
        if(notation[i] == s) return i;
    return -1;
}
char Pri(char s, char c) {     //c表示当前运算符，s表示栈顶运算符
    return pri[get_index(s)][get_index(c)];
}
void readUnit(int& r, char* str, stack<unit> &opnd) {
    //处理只有一个x的情况
    if(str[r] == 'x'){
        opnd.push(unit(1, 1));
        return;
    }
    //处理纯数字情况
    long long value = 0;
    while(isdigit(str[r])) {
        value = value * 10 + (str[r] - '0');
        ++r;
    }
    //如果后面跟了x，即第一种省略乘号的情况
    if(str[r] == 'x')
    {
        opnd.push(unit(value, 1));
        return;
    }
    opnd.push(unit(value, 0));
}
void calc(stack<unit> &opnd, char notation)
{
    switch(notation) {
        case '+':
            unit y1 = opnd.top();
            opnd.pop();
            if(opnd.top().exponent == y1.exponent) opnd.top().coef += y1.coef;
            else return;
            break;
        case '-':
            unit y2 = opnd.top();
            opnd.pop();
            if(opnd.top().exponent == y2.exponent) opnd.top().coef -= y2.coef;
            else return;
            break;
        case '*':
            unit y3 = opnd.top();
            opnd.pop();
            opnd.top() *= y3;
            break;
        case '^':
            
    }
}
int main() {
    char str[1000001];
    scanf("%s", str);
    stack<unit> opnd;
    stack<char> optr;
    optr.push('$');
    int r = 0;   //r表示当前处理的字符串的位置
    while(!optr.empty() )
    {
        if(isdigit(str[r]) || str[r] == 'x') {   //遇到操作数就读入
            readUnit(r, str, opnd);
        }
        else {   //若遇到运算符c
            switch(Pri(optr.top(), str[r])) {
                case '<':
                    optr.push(str[r++]);
                    break;
                case '>':    //执行运算
                    calc(opnd, optr.top());
                    optr.pop();
                    break;
                case '~':
                    optr.pop();
                    r ++;
                    break;
            }
        }
    }
}