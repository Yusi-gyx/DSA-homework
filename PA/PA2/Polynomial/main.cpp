#include <stack>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <vector>
using namespace std;

const int M = 1000000007;
const int MAX_DEGREE = 64;
const char* notation = "+-*^()$";
const char pri[7][7] = {
    /*           |------- 当 前 运 算 符 --------|  */
    /*           +    -    *    ^     (    )    $ */
    /*--  + */  '>', '>', '<', '<',  '<', '>', '>',
    /* 栈 - */  '>', '>', '<', '<',  '<', '>', '>',
    /* 顶 * */  '>', '>', '>', '<',  '<', '>', '>', 
    /* 运 ^ */  '>', '>', '>', '>',  '<', '>', '>',
    /* 算 ( */  '<', '<', '<', '<',  '<', '~', 'x',
    /* 符 ) */  ' ', ' ', ' ', ' ',  ' ', ' ', ' ',
    /*--  $ */  '<', '<', '<', '<',  '<', 'x', '~'
};
int normalize(long long c);

struct term {       //代表一个单项式
    long long coef;   //每一项的系数
    int exponent;     //每一项的指数

    term(): coef(0), exponent(0) {}
    term(long long c, int e): coef(c), exponent(e) {}
    void operator*=(term y) {
        //左结合乘法
        coef = 1LL * normalize(coef) * normalize(y.coef) % M;
        exponent += y.exponent;
    }
    void operator+=(term y) {
        coef = normalize(1LL * normalize(coef) + normalize(y.coef));
    }
    void operator-=(term y) {
        coef = normalize(1LL * normalize(coef) - normalize(y.coef));
    }
    bool operator>(term y) {
        if(exponent > y.exponent) return true;
        return false;
    }
};
struct Polynomial {     //代表一个多项式
    vector<term> terms;

    term& operator[](int i) {
        return terms[i];
    }
    const term& operator[](int i) const {
        return terms[i];
    }
};

int normalize(long long c) {    //定义取模函数
    return (c % M + M) % M;
}

int get_index(char s) {
    for(int i = 0; i < 7; i ++)
        if(notation[i] == s) return i;
    return 6;
}
char Pri(char s, char c) {     //c表示当前运算符，s表示栈顶运算符
    return pri[get_index(s)][get_index(c)];
}

void completeMultiplication(const char* input, char* output) {
    int w = 0;
    for(int i = 0; input[i] != '\0'; i++) {
        if(i > 0) {
            char left = input[i - 1], right = input[i];
            //左边表示一个操作数，右边满足题目允许省略乘号的条件
            bool canEnd = isdigit(left) || left == 'x' || left == ')';
            bool allowed = right == '(' || (right == 'x' && left != 'x');
            if(canEnd && allowed) output[w++] = '*';
        }
        output[w++] = input[i];
    }
    output[w] = '\0';
}

void readPoly(int& r, char* str, stack<Polynomial> &opnd) {
    Polynomial y;
    //处理只有一个x的情况
    if(str[r] == 'x'){
        y.terms.push_back(term(1, 1));
        opnd.push(y);
        ++r;
        return;
    }
    //处理纯数字情况
    long long value = 0;
    while(isdigit(str[r])) {
        value = value * 10 + (str[r] - '0');
        ++r;
    }
    y.terms.push_back(term(value, 0));
    opnd.push(y);
}
void combine(vector<term> &p)   //合并同类项
{
    size_t w = 0;
    for(size_t i = 0; i < p.size();) {
        term t = p[i++];
        t.coef = normalize(t.coef);
        while(i < p.size() && p[i].exponent == t.exponent)
            t += p[i++];
        if(t.coef != 0)
            p[w++] = t;
    }
    p.resize(w);
    if(p.empty())
        p.push_back(term(0, 0));
}
Polynomial addSub(const Polynomial& a, const Polynomial& b, int sign) {   //利用类似归并的方法对多项式进行加减合并
    Polynomial result;
    result.terms.reserve(a.terms.size() + b.terms.size());

    size_t i = 0, j = 0;
    while(i < a.terms.size() || j < b.terms.size()) {
        term t;

        if(j == b.terms.size() ||
           (i < a.terms.size() &&
            a.terms[i].exponent > b.terms[j].exponent)) {
            t = a.terms[i++];
        }
        else if(i == a.terms.size() ||
                a.terms[i].exponent < b.terms[j].exponent) {
            t = b.terms[j++];
            t.coef = 1LL * sign * normalize(t.coef);
        }
        else {
            t = a.terms[i++];
            t.coef = normalize(t.coef) + 1LL * sign * normalize(b.terms[j++].coef);
        }

        t.coef = normalize(t.coef);
        if(t.coef != 0)
            result.terms.push_back(t);
    }

    // 零多项式保留一项
    if(result.terms.empty())
        result.terms.push_back(term(0, 0));

    return result;
}
Polynomial Multiple(const Polynomial &a, const Polynomial &b) {
    Polynomial result;
    int degree = a[0].exponent + b[0].exponent;
    result.terms.reserve(degree + 1);
    long long coef[MAX_DEGREE + 1] = {};      //建立以指数为索引的系数数组
    long long bCoef[MAX_DEGREE + 1];
    for(size_t j = 0; j < b.terms.size(); j++)     //取模
        bCoef[j] = normalize(b[j].coef);

    //题目保证指数不超过64，可以直接直接累加
    for(int i = 0; i < a.terms.size(); i++) {
        long long aCoef = normalize(a[i].coef);     
        if(aCoef == 0) continue;
        for(int j = 0; j < b.terms.size(); j++) {
            int e = a[i].exponent + b[j].exponent;        //先取模再相乘
            coef[e] = (coef[e] + aCoef * bCoef[j]) % M;
        }
    }
    for(int e = degree; e >= 0; e--)
        if(coef[e] != 0)
            result.terms.push_back(term(coef[e], e));
    if(result.terms.empty())
        result.terms.push_back(term(0, 0));
    return result;
}
void calc(stack<Polynomial> &opnd, char notation)
{
    Polynomial y = opnd.top();
    switch(notation) {
        case '+': {
            opnd.pop();
            opnd.top() = addSub(opnd.top(), y, 1);  
            break;
        }
        case '-': {
            opnd.pop();
            opnd.top() = addSub(opnd.top(), y, -1);
            break;
        }
        case '*': {
            opnd.pop();
            opnd.top() = Multiple(opnd.top(), y);
            break;
        }
        case '^': {
            //题目保证次幂号的右边一定是正整数
            opnd.pop();
            Polynomial z = opnd.top();
            for(long long i = 1; i < y[0].coef; i++)
            {
                opnd.top() = Multiple(opnd.top(), z);
            }
            break;
        }
    }
}
int main() {
    char input[1000001];
    char str[2000001];     //补齐乘号后，字符串长度最多接近原来的两倍
    if(scanf("%1000000s", input) != 1) return 0;
    completeMultiplication(input, str);
    stack<Polynomial> opnd;
    stack<char> optr;
    optr.push('$');
    int r = 0;   //r表示当前处理的字符串的位置
    while(!optr.empty() )
    {
        if(isdigit(str[r]) || str[r] == 'x') {   //遇到操作数就读入
            readPoly(r, str, opnd);
        }
        else {   //若遇到运算符c
            switch(Pri(optr.top(), str[r])) {
                case '<': {
                    optr.push(str[r++]);
                    break;
                }
                case '>': {    //执行运算
                    calc(opnd, optr.top());
                    optr.pop();
                    break;
                }
                case '~': {
                    optr.pop();
                    r ++;
                    break;
                }
            }
        }
    }
    //最后在opnd中Polynomial一定就是需要的多项式
    Polynomial polynomial;
    polynomial = opnd.top();
    combine(polynomial.terms);     //也处理只有一个数字、没有经过运算的情况
    //对结果逐项输出
    if(polynomial.terms.size() == 1 && polynomial[0].coef == 0) 
    {
        printf("0");
        return 0;
    }
    int degree = polynomial[0].exponent;
    size_t i = 0;
    for(int e = degree; e >= 0; e--) {
        if(e != degree) putchar(' ');     //只在两个系数之间输出空格
        if(i < polynomial.terms.size() && polynomial[i].exponent == e)
            printf("%d", normalize(polynomial[i++].coef));
        else
            putchar('0');
    }
    return 0;
}