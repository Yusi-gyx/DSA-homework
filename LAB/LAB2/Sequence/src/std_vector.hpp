// ============================================================================
// StdVector - std::vector 的包装，用于性能对比和正确性验证
// ============================================================================
#include <vector>

template <class T>
class StdVector {
    std::vector<T> vec_;

public:
    void insert(int index, T val) {
        vec_.insert(vec_.begin() + index, val);
    }
    
    void remove(int index) {
        vec_.erase(vec_.begin() + index);
    }
    
    T query(int index) const {
        return vec_[index];
    }
    
    // 端点操作是普通成员函数；push_back/pop_back直接使用vector接口。
    void push_front(T x) { insert(0, x); }
    void push_back(T x) { vec_.push_back(x); }
    void pop_front() { remove(0); }
    void pop_back() { vec_.pop_back(); }
    T front() const { return vec_.front(); }
    T back() const { return vec_.back(); }
    
    std::size_t size() const { return vec_.size(); }
    
    auto begin() { return vec_.begin(); }
    auto end() { return vec_.end(); }
    auto begin() const { return vec_.begin(); }
    auto end() const { return vec_.end(); }
};
